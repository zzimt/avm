#ifndef AVM_MEM_H_
#define AVM_MEM_H_

#include <cstddef>
#include <algorithm>
#include <vector>
#include <cstring>

#include <rapidhash/rapidhash.h>

#include <parallel_hashmap/phmap.h>

#include "value.h"
#include "stack.h"
#include "localstack.h"

namespace avm {

    #define AVM_ALIGN_UP(n, alignment) \
        (((n) + (alignment) - 1) & ~((alignment) - 1))

    struct StrHeader {
        std::size_t size;
        bool mark;
        std::size_t total_size;

        static inline std::size_t allocated_size_in_bytes(std::size_t size);

        static inline StrHeader* allocate(std::size_t size, bool mark);

        static inline void deallocate(StrHeader* header);

        inline StrHeader(std::size_t size, bool mark, std::size_t total_size) :
            size(size),
            mark(mark),
            total_size(total_size) { }

        inline char* data();

        inline const char* data() const;
    };

    static constexpr std::size_t STR_HEADER_ALIGN =
        std::max(alignof(StrHeader), alignof(char));

    static constexpr std::size_t STR_HEADER_SIZE =
        AVM_ALIGN_UP(sizeof(StrHeader), STR_HEADER_ALIGN);

    inline std::size_t StrHeader::allocated_size_in_bytes(
        std::size_t size
    ) {
        return STR_HEADER_SIZE + sizeof(char) * size;
    }

    inline StrHeader* StrHeader::allocate(std::size_t size, bool mark) {
        std::size_t total_size = allocated_size_in_bytes(size);

        void* block_ptr = ::operator new(
            total_size,
            std::align_val_t(STR_HEADER_ALIGN)
        );

        auto* header = new (block_ptr) StrHeader(size, mark, total_size);
        new (
            reinterpret_cast<std::byte*>(block_ptr) + STR_HEADER_SIZE
        ) char[size];
        return header;
    }

    inline void StrHeader::deallocate(StrHeader* header) {
        void* block_ptr = header;
        ::operator delete(block_ptr, std::align_val_t(STR_HEADER_ALIGN));
    }

    inline char* StrHeader::data() {
        return reinterpret_cast<char*>(
            reinterpret_cast<std::byte*>(this) + STR_HEADER_SIZE
        );
    }

    inline const char* StrHeader::data() const {
        return reinterpret_cast<const char*>(
            reinterpret_cast<const std::byte*>(this) + STR_HEADER_SIZE
        );
    }

    struct StrKey {
        const char* data;
        std::size_t size;
        std::size_t hash;

        inline StrKey() :
            data(nullptr),
            size(0),
            hash(0) { }

        inline StrKey(const char* data, std::size_t size) :
            data(data),
            size(size),
            hash(rapidhash(data, size)) { }

        inline bool operator==(const StrKey& rhs) const {
            if (this->size != rhs.size) return false;
            if (this->hash != rhs.hash) return false;
            return std::memcmp(this->data, rhs.data, this->size) == 0;
        }

        inline bool operator!=(const StrKey& rhs) const {
            return !((*this) == rhs);
        }

        /// Hash function for `phmap::flat_hash_map`
        friend std::size_t hash_value(const StrKey& k) {
            return k.hash;
        }
    };

    struct MemHeader {
        enum class UniformType {
            None,
            Primitive,
            Str,
            Ref,
        };

        std::size_t count;
        bool mark;
        UniformType uniform_type;
        std::align_val_t type_align;
        std::size_t type_size;
        std::size_t total_size;

        static inline std::size_t allocated_size_in_bytes(
            std::size_t elem_count
        );

        template<typename T>
        static inline std::size_t allocated_size_in_bytes_uniform(
            std::size_t elem_count
        );

        static inline MemHeader* allocate(std::size_t count);

        template<typename T>
        static inline MemHeader* allocate_uniform(std::size_t count);

        static inline void deallocate(MemHeader* header);

        inline MemHeader(
            std::size_t count, 
            bool mark, 
            UniformType uniform_type,
            std::align_val_t type_align,
            std::size_t type_size,
            std::size_t total_size
        ) : count(count),
            mark(mark),
            uniform_type(uniform_type),
            type_align(type_align),
            type_size(type_size),
            total_size(total_size) { }

        inline Value* values();

        inline const Value* values() const;

        template<typename T>
        inline T* values_uniform();

        template<typename T>
        inline const T* values_uniform() const;
    };

    template<typename T>
    struct MemHeaderAlign {
        static constexpr std::size_t value = 
            std::max(alignof(MemHeader), alignof(T));
    };

    template<typename T>
    struct MemHeaderSize {
        static constexpr std::size_t value = 
            AVM_ALIGN_UP(sizeof(MemHeader), MemHeaderAlign<T>::value);
    };

    template<typename T>
    struct MemHeaderUniformType { 
        static constexpr MemHeader::UniformType value = 
            MemHeader::UniformType::Primitive;
    };

    template<>
    struct MemHeaderUniformType<Str> {
        static constexpr MemHeader::UniformType value = 
            MemHeader::UniformType::Str;
    };

    template<>
    struct MemHeaderUniformType<Ref> {
        static constexpr MemHeader::UniformType value = 
            MemHeader::UniformType::Ref;
    };

    inline std::size_t MemHeader::allocated_size_in_bytes(
        std::size_t elem_count
    ) {
        return MemHeaderSize<Value>::value + sizeof(Value) * elem_count;
    }

    template<typename T>
    inline std::size_t MemHeader::allocated_size_in_bytes_uniform(
        std::size_t elem_count
    ) {
        return MemHeaderSize<T>::value + sizeof(T) * elem_count;
    }

    inline MemHeader* MemHeader::allocate(std::size_t count) {
        std::size_t total_size = allocated_size_in_bytes(count);

        constexpr auto type_align = 
            std::align_val_t(MemHeaderAlign<Value>::value);
        
        void* block_ptr = ::operator new(
            total_size, 
            type_align
        );

        auto* header = new (block_ptr) MemHeader(
            count, 
            false, 
            UniformType::None,
            type_align,
            sizeof(Value),
            total_size
        );
        new (
            reinterpret_cast<std::byte*>(block_ptr) +
            MemHeaderSize<Value>::value
        ) Value[count](); // calling the `()` constructor is important here as 
        // we need to ensure that all values are zero when allocated. It is
        // especially important for Str and Ref types so that the GC ignores
        // uninitialized (null) strings and references
        return header;
    }

    template<typename T>
    inline MemHeader* MemHeader::allocate_uniform(std::size_t count) {
        std::size_t total_size = allocated_size_in_bytes_uniform<T>(count);

        constexpr auto type_align = 
            std::align_val_t(MemHeaderAlign<T>::value);

        void* block_ptr = ::operator new(
            total_size,
            type_align
        );

        auto* header = new (block_ptr) MemHeader(
            count,
            false,
            MemHeaderUniformType<T>::value,
            type_align,
            sizeof(T),
            total_size
        );
        new (
            reinterpret_cast<std::byte*>(block_ptr) +
            MemHeaderSize<T>::value
        ) T[count](); // calling the `()` constructor is important here as we
        // need to ensure that all values are zero when allocated. It is
        // especially important for Str and Ref types so that the GC ignores
        // uninitialized (null) strings and references
        return header;
    }

    inline void MemHeader::deallocate(MemHeader* header) {
        void* block_ptr = header;
        ::operator delete(block_ptr, header->type_align);
    }

    inline Value* MemHeader::values() {
        return reinterpret_cast<Value*>(
            reinterpret_cast<std::byte*>(this) + MemHeaderSize<Value>::value
        );
    }

    inline const Value* MemHeader::values() const {
        return reinterpret_cast<const Value*>(
            reinterpret_cast<const std::byte*>(this) + 
            MemHeaderSize<Value>::value
        );
    }

    template<typename T>
    inline T* MemHeader::values_uniform() {
        return reinterpret_cast<T*>(
            reinterpret_cast<std::byte*>(this) +
            MemHeaderSize<T>::value
        );
    }

    template<typename T>
    inline const T* MemHeader::values_uniform() const {
        return reinterpret_cast<const T*>(
            reinterpret_cast<const std::byte*>(this) +
            MemHeaderSize<Value>::value
        );
    }

    class Mem {
        static_assert(std::is_trivially_destructible_v<Value>, 
            "`Value` type must be trivially destructible");

        static_assert(std::is_trivially_destructible_v<StrHeader>,
            "`StrHeader` type must be trivially destructible");

        static_assert(std::is_trivially_destructible_v<MemHeader>,
            "`MemHeader` type must be trivially destructible");

    public:
        Mem(const Mem&) = delete;

        Mem(Mem&&) = delete;

        inline Mem(Stack<Value>& stack, LocalStack& local_stack) :
            m_tracked_blocks(),
            m_gray(),
            m_total_allocated(0),
            m_live_bytes(0),
            m_alloc_since_gc(0),
            m_gc_threshold(1024 * 1024 * 4),
            m_stack(stack),
            m_local_stack(local_stack) { 
            m_gray.reserve(256);
        }

        Mem& operator=(const Mem&) = delete;

        Mem& operator=(Mem&&) = delete;

        ~Mem() {
            for (const auto& block : m_tracked_blocks) {
                MemHeader::deallocate(block);
            }

            for (const auto& [key, str] : m_strings) {
                StrHeader::deallocate(str);
            }
        }

        inline MemHeader* make(std::uint64_t count) {
            if (count == 0) return nullptr;

            m_alloc_since_gc += MemHeader::allocated_size_in_bytes(count);

            run_gc_if_needed();

            return alloc_block(count);
        }

        template<typename T>
        inline MemHeader* make_uniform(std::uint64_t count) {
            if (count == 0) return nullptr;
            
            m_alloc_since_gc += 
                MemHeader::allocated_size_in_bytes_uniform<T>(count);

            run_gc_if_needed();

            return alloc_block_uniform<T>(count);
        }

        inline Value* get(MemHeader* header, std::uint64_t idx) {
            if (idx >= header->count) {
                return nullptr;
            }
            return &header->values()[idx];
        }

        template<typename T>
        inline T* get_uniform(MemHeader* header, std::uint64_t idx) {
            if (idx >= header->count) {
                return nullptr;
            }
            return &header->values_uniform<T>()[idx];
        }

        inline StrHeader* str_intern(const StrKey& key) {
            auto found = m_strings.find(key);
            if (found == m_strings.end()) {
                auto* header = StrHeader::allocate(key.size, false);
                auto new_key = key;
                new_key.data = header->data();
                std::memcpy(header->data(), key.data, key.size);
                m_strings.insert({ new_key, header });
                return header;
            }
            return found->second;
        }

    private:
        void run_gc_if_needed() {
            if (m_alloc_since_gc >= m_gc_threshold) {
                run_gc();
                m_alloc_since_gc = 0;

                m_gc_threshold = std::max(
                    m_live_bytes + m_live_bytes / 2, 
                    std::size_t(1024 * 1024)
                );
            }
        }

        inline MemHeader* alloc_block(std::uint64_t count) {
            auto* header = MemHeader::allocate(count);

            m_tracked_blocks.push_back(header);
            m_total_allocated += header->total_size;

            return header;
        }

        template<typename T>
        inline MemHeader* alloc_block_uniform(std::uint64_t count) {
            auto* header = MemHeader::allocate_uniform<T>(count);

            m_tracked_blocks.push_back(header);
            m_total_allocated += header->total_size;

            return header;
        }

        inline void run_gc() {
            run_mark();
            run_sweep();
        }

        void run_mark() {
            m_gray.clear();

            auto mark_str_header = [](StrHeader* header) {
                if (header == nullptr) return; // ignore uninitialized slots
                header->mark = true;
            };

            auto mark_mem_header = [this](MemHeader* header) {
                if (header == nullptr) return; // ignore uninitialized slots
                if (!header->mark) {
                    header->mark = true;
                    m_gray.push_back(header);
                }
            };

            auto mark_value = [&](Value& value) {
                switch (value.type()) {
                case Type::Int:
                case Type::Uint:
                case Type::Byte:
                case Type::Float:
                case Type::Bool:
                    break;
                case Type::Str: {
                    StrHeader* header = value.string();
                    mark_str_header(header);
                } break;
                case Type::Ref: {
                    MemHeader* header = value.reference();
                    mark_mem_header(header);
                } break;
                }
            };

            for (auto& value : m_stack) {
                mark_value(value);
            }

            for (auto& value : m_local_stack) {
                mark_value(value);
            }

            while (!m_gray.empty()) {
                MemHeader* header = m_gray.back();
                m_gray.pop_back();

                switch (header->uniform_type) {
                case MemHeader::UniformType::None: {
                    Value* values = header->values();
                    for (std::size_t i = 0; i < header->count; i++) {
                        Value& value = values[i];
                        mark_value(value);
                    }
                } break;
                case MemHeader::UniformType::Primitive: {
                    // ignore, block contains primitive type values
                } break;
                case MemHeader::UniformType::Str: {
                    Str* strings = header->values_uniform<Str>();
                    for (std::size_t i = 0; i < header->count; i++) {
                        Str& string = strings[i];
                        mark_str_header(string);
                    }
                } break;
                case MemHeader::UniformType::Ref: {
                    Ref* references = header->values_uniform<Ref>();
                    for (std::size_t i = 0; i < header->count; i++) {
                        MemHeader* header = references[i];
                        mark_mem_header(header);
                    }
                } break;
                }
            }
        }

        void run_sweep() {
            m_live_bytes = 0;

            std::size_t write = 0;
            for (std::size_t read = 0; read < m_tracked_blocks.size(); read++) {
                auto& header = m_tracked_blocks[read];
                if (header->mark) {
                    header->mark = false;
                    m_live_bytes += header->total_size;
                    m_tracked_blocks[write++] = header;
                } else {
                    MemHeader::deallocate(header);
                }
            }

            m_tracked_blocks.resize(write);

            for (auto it = m_strings.begin(); it != m_strings.end(); ) {
                auto* header = it->second;
                if (header->mark) {
                    header->mark = false;
                    m_live_bytes += header->total_size;
                    ++it;
                } else {
                    it = m_strings.erase(it);
                }
            }
        }

        std::vector<MemHeader*> m_tracked_blocks;
        phmap::flat_hash_map<StrKey, StrHeader*> m_strings;
        std::vector<MemHeader*> m_gray;
        std::size_t m_total_allocated;
        std::size_t m_live_bytes;
        std::size_t m_alloc_since_gc;
        std::size_t m_gc_threshold;
        Stack<Value>& m_stack;
        LocalStack& m_local_stack;
    };

}

#endif // AVM_MEM_H_