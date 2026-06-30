#ifndef AVM_MEM_H_
#define AVM_MEM_H_

#include <cstddef>
#include <algorithm>
#include <vector>

#include "value.h"
#include "stack.h"
#include "localstack.h"

namespace avm {

    #define AVM_ALIGN_UP(n, alignment) \
        (((n) + (alignment) - 1) & ~((alignment) - 1))

    struct MemHeader {
        std::size_t count;
        bool mark;

        static inline std::size_t allocated_size_in_bytes(
            std::size_t elem_count
        );

        static inline MemHeader* allocate(std::size_t count, bool mark);

        static inline void deallocate(MemHeader* header);

        inline MemHeader(std::size_t count, bool mark) :
            count(count),
            mark(mark) { }

        inline Value* data();

        inline const Value* values() const;

        inline std::size_t allocated_size_in_bytes() const;
    };

    static constexpr std::size_t MEM_HEADER_ALIGN =
        std::max(alignof(MemHeader), alignof(Value));

    static constexpr std::size_t MEM_HEADER_SIZE =
        AVM_ALIGN_UP(sizeof(MemHeader), MEM_HEADER_ALIGN);

    inline std::size_t MemHeader::allocated_size_in_bytes(
        std::size_t elem_count
    ) {
        return MEM_HEADER_SIZE + sizeof(Value) * elem_count;
    }

    inline MemHeader* MemHeader::allocate(std::size_t count, bool mark) {
        std::size_t total_size = allocated_size_in_bytes(count);
        
        void* block_ptr = ::operator new(
            total_size, 
            std::align_val_t(MEM_HEADER_ALIGN)
        );

        auto* header = new (block_ptr) MemHeader(count, mark);
        new (
            reinterpret_cast<std::byte*>(block_ptr) + MEM_HEADER_SIZE
        ) Value[count];
        return header;
    }

    inline void MemHeader::deallocate(MemHeader* header) {
        void* block_ptr = header;
        ::operator delete(block_ptr, std::align_val_t(MEM_HEADER_ALIGN));
    }

    inline Value* MemHeader::data() {
        return reinterpret_cast<Value*>(
            reinterpret_cast<std::byte*>(this) + MEM_HEADER_SIZE
        );
    }

    inline const Value* MemHeader::values() const {
        return reinterpret_cast<const Value*>(
            reinterpret_cast<const std::byte*>(this) + MEM_HEADER_SIZE
        );
    }

    inline std::size_t MemHeader::allocated_size_in_bytes() const {
        return MEM_HEADER_SIZE + sizeof(Value) * count;
    }

    class Mem {
        static_assert(std::is_trivially_destructible_v<Value>, 
            "`Value` type must be trivially destructible");

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
                free_block(block);
            }
        }

        inline MemHeader* make(std::uint64_t count) {
            m_alloc_since_gc += MemHeader::allocated_size_in_bytes(count);

            if (m_alloc_since_gc >= m_gc_threshold) {
                run_gc();
                m_alloc_since_gc = 0;

                m_gc_threshold = std::max(
                    m_live_bytes + m_live_bytes / 2, 
                    std::size_t(1024 * 1024)
                );
            }

            return alloc_block(count);
        }

        inline Value& get(MemHeader* header, std::uint64_t idx) {
            return header->data()[idx];
        }

    private:
        inline MemHeader* alloc_block(std::uint64_t count) {
            auto* header = MemHeader::allocate(count, false);

            m_tracked_blocks.push_back(header);
            m_total_allocated += header->allocated_size_in_bytes();

            return header;
        }

        inline void free_block(MemHeader* header) {
            MemHeader::deallocate(header);
        }

        inline void run_gc() {
            run_mark();
            run_sweep();
        }

        inline void run_mark() {
            m_gray.clear();

            auto mark_value = [this](Value& value) {
                if (value.type() == Type::Ref) {
                    MemHeader* header = value.reference();
                    if (!header->mark) {
                        header->mark = true;
                        m_gray.push_back(header);
                    }
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

                Value* data = header->data();
                for (std::size_t i = 0; i < header->count; i++) {
                    Value& value = data[i];
                    mark_value(value);
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

                    m_live_bytes += header->allocated_size_in_bytes();
                    m_tracked_blocks[write++] = header;
                } else {
                    free_block(header);
                }
            }
            m_tracked_blocks.resize(write);
        }

        std::vector<MemHeader*> m_tracked_blocks;
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