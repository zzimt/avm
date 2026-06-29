#include <iostream>
#include <vector>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <unordered_map>
#include <algorithm>
#include <new>
#include <cmath>

namespace avm {

    enum class Type : std::uint8_t {
        Int,
        Uint,
        Float,
        Bool,
        Ref,
    };

    struct MemHeader;

    class Value {
    public:
        union Data {
            std::int64_t integer;
            std::uint64_t uinteger;
            double floating;
            std::uint8_t boolean;
            MemHeader* reference;
        };

        static inline Value integer(std::int64_t value) {
            return Value(Type::Int, { .integer = value });
        }

        inline std::int64_t integer() const {
            return m_data.integer;
        }

        static inline Value uinteger(std::uint64_t value) {
            return Value(Type::Uint, { .uinteger = value });
        }

        inline std::uint64_t uinteger() const {
            return m_data.uinteger;
        }

        static inline Value floating(double value) {
            return Value(Type::Float, { .floating = value });
        }

        inline float floating() const {
            return m_data.floating;
        }

        static inline Value boolean(std::uint8_t value) {
            return Value(Type::Bool, { .boolean = value });
        }

        inline std::uint8_t boolean() const {
            return m_data.boolean;
        }

        static inline Value reference(MemHeader* value) {
            return Value(Type::Ref, { .reference = value });
        }

        inline MemHeader* reference() const {
            return m_data.reference;
        }

        inline Type type() const {
            return m_type;
        }
        
        inline Value() :
            m_type(Type::Int),
            m_data({ .integer = 0 }) { }

        inline Value(Type type, Data data) :
            m_type(type),
            m_data(data) { }

    private:
        Type m_type;
        Data m_data;
    };

    enum class Op : std::uint8_t {
        Nop = 0,

        StoreLocal,
        LoadLocal, 
        Push,

        MakeMem,
        MakeMemCount,
        MakeMemCountIm,
        StoreMem,
        StoreMemIm,
        StoreMemIdx,
        StoreMemImIdx,
        StoreMemIdxIm,
        StoreMemImIdxIm,
        LoadMem,
        LoadMemIdx,
        LoadMemIdxIm,

        Copy,
        Swap,

        PrintInt,
        PrintUint,
        PrintBool,
        PrintRef,
        PrintFloat,

        AddInt,
        SubInt,
        MulInt,
        DivInt,
        UnMinInt,
        ModInt,
        EqInt,
        NeqInt,
        LtInt,
        GtInt,
        LtEqInt,
        GtEqInt,
        BinAndInt,
        BinOrInt,
        BinXorInt,
        BinNotInt,
        LShiftInt,
        RShiftInt,

        AddUint,
        SubUint,
        MulUint,
        DivUint,
        UnMinUint,
        ModUint,
        EqUint,
        NeqUint,
        LtUint,
        GtUint,
        LtEqUint,
        GtEqUint,
        BinAndUint,
        BinOrUint,
        BinXorUint,
        BinNotUint,
        LShiftUint,
        RShiftUint,

        AddFloat,
        SubFloat,
        MulFloat,
        DivFloat,
        UnMinFloat,
        EqFloat,
        NeqFloat,
        LtFloat,
        GtFloat,
        LtEqFloat,
        GtEqFloat,

        AndBool,
        OrBool,
        XorBool,
        NotBool,
        EqBool,
        NeqBool,

        CIntUint,
        CIntFloat,
        CUintInt,
        CUintFloat,
        CFloatInt,
        CFloatUint,

        If,
        Call,
        CallIm,
        Ret,
        Goto,
        GotoIm,

        CallIntrin,

        Exit,
    };

    enum class Intrin : uint8_t {
        MinInt,
        MaxInt,
        SignInt,
        AbsInt,
        
        MinUint,
        MaxUint,

        FloorFloat,
        CeilFloat,
        RoundFloat,
        TruncFloat,
        IsNanFloat,
        IsInfFloat,
        IsFinFloat,
        SignBitFloat,
        FmaFloat,
        MinFloat,
        MaxFloat,
        SqrtFloat,
        SinFloat,
        CosFloat,
        TanFloat,
        ExpFloat,
        Exp2Float,
        LogFloat,
        Log2Float,
        Log10Float,
        PowFloat,
        SignFloat,
        AbsFloat,
    };

    struct Inst {
        Op op;
        Value a;
        Value b;

        inline Inst(Op op) :
            op(op),
            a(),
            b() { }

        inline Inst(Op op, Value a) :
            op(op),
            a(a),
            b() { }

        inline Inst(Op op, Value a, Value b) :
            op(op),
            a(a),
            b(b) { }
    };


    class Elem {
    public:
        enum class Kind {
            Label,
            If,
            CallIm,
            GotoIm,
            Inst,
        };

        struct Label {
            std::string_view value;
        };

        struct If {
            std::string_view true_label;
            std::string_view false_label; 
        };

        struct CallIm {
            std::string_view label;
        };

        struct GotoIm {
            std::string_view label;
        };

        union Data {
            Label label;
            If if_;
            CallIm call_im;
            GotoIm goto_im;
            Inst inst;
        };

        static inline Elem label(const std::string_view& value) {
            return Elem(
                Kind::Label, 
                { 
                    .label = { .value = value } 
                }
            );
        }

        inline Label label() const {
            return m_data.label;
        }

        static inline Elem if_(
            const std::string_view& true_label, 
            const std::string_view& false_label
        ) {
            return Elem(
                Kind::If, 
                { 
                    .if_ = { 
                        .true_label = true_label, 
                        .false_label = false_label
                    } 
                }
            );
        }

        inline If if_() const {
            return m_data.if_;
        }

        static inline Elem call_im(std::string_view label) {
            return Elem(
                Kind::CallIm, 
                { 
                    .call_im = { .label = label } 
                }
            );
        }

        inline CallIm call_im() const {
            return m_data.call_im;
        }

        static inline Elem goto_im(std::string_view label) {
            return Elem(
                Kind::GotoIm,
                {
                    .goto_im = { .label = label }
                }
            );
        }

        inline GotoIm goto_im() const {
            return m_data.goto_im;
        }

        static inline Elem inst(const Inst& inst) {
            return Elem(Kind::Inst, { .inst = inst });
        }

        inline Inst inst() const {
            return m_data.inst;
        }

        inline Kind kind() const {
            return m_kind;
        }

        inline Elem(Kind kind, const Data& data) :
            m_kind(kind),
            m_data(data) { }

    private:
        Kind m_kind;
        Data m_data;
    };

    class Resolver {
    public:
        Resolver(const std::vector<Elem>& elems) :
            m_elems(elems),
            m_labels_to_addrs() { }
        
        std::vector<Inst> resolve() {
            collect_labels();
            return resolve_labels();
        }
        
    private:
        void collect_labels() {
            size_t inst_addr = 0;
            for (size_t elem_id = 0; elem_id < m_elems.size(); elem_id++) {
                auto& elem = m_elems[elem_id];
                switch (elem.kind()) {
                case Elem::Kind::Label:
                    m_labels_to_addrs.insert({elem.label().value, inst_addr});
                    break;
                case Elem::Kind::If:
                case Elem::Kind::CallIm:
                case Elem::Kind::GotoIm:
                case Elem::Kind::Inst:
                    inst_addr++;
                    break;
                }
            }
        }

        std::vector<Inst> resolve_labels() {
            std::vector<Inst> res;
            for (size_t elem_id = 0; elem_id < m_elems.size(); elem_id++) {
                auto& elem = m_elems[elem_id];
                switch (elem.kind()) {
                case Elem::Kind::Label:
                    break;
                case Elem::Kind::If: {
                    std::uint64_t true_addr = 
                        m_labels_to_addrs[elem.if_().true_label];
                    std::uint64_t false_addr = 
                        m_labels_to_addrs[elem.if_().false_label];
                    res.push_back(
                        Inst(
                            Op::If, 
                            Value::uinteger(true_addr), 
                            Value::uinteger(false_addr)
                        )
                    );
                } break;
                case Elem::Kind::CallIm: {
                    std::uint64_t addr = 
                        m_labels_to_addrs[elem.call_im().label];
                    res.push_back(
                        Inst(
                            Op::CallIm,
                            Value::uinteger(addr)
                        )
                    );
                } break;
                case Elem::Kind::GotoIm: {
                    std::uint64_t addr = 
                        m_labels_to_addrs[elem.goto_im().label];
                    res.push_back(
                        Inst(
                            Op::GotoIm,
                            Value::uinteger(addr)
                        )
                    );
                } break;
                case Elem::Kind::Inst:
                    res.push_back(elem.inst());
                    break;
                }
            }
            return res;
        }

        std::vector<Elem> m_elems;
        std::unordered_map<std::string_view, std::uint64_t> m_labels_to_addrs;
    };

    template <typename T>
    class Stack {
    public:
        inline Stack() :
            m_elements() { }

        inline void reserve(std::size_t capacity) {
            m_elements.reserve(capacity);
        }

        inline void push(const T& elem) {
            m_elements.push_back(elem);
        }

        inline T pop() {
            T back = std::move(m_elements.back());
            m_elements.pop_back();
            return back;
        }

        inline const T& operator[](std::size_t index) const {
            return m_elements[m_elements.size() - 1 - index];
        }

        inline T& operator[](std::size_t index) {
            return m_elements[m_elements.size() - 1 - index];
        }

        auto begin() {
            return m_elements.begin();
        }

        auto end() {
            return m_elements.end();
        }

        auto begin() const {
            return m_elements.begin();
        }

        auto end() const {
            return m_elements.end();
        }

    private:
        std::vector<T> m_elements;
    };

    class LocalStack {
    public:
        inline LocalStack() :
            m_values(),
            m_frame_indices() { }

        inline void reserve(std::size_t capacity) {
            m_values.reserve(capacity);
        }

        inline void push() {
            m_frame_indices.push_back(m_values.size());
        }

        inline void pop() {
            m_frame_indices.pop_back();
        }

        inline Value& operator[](std::size_t id) {
            std::size_t last_index = m_frame_indices.back();
            m_values.resize(last_index + id + 1);
            return m_values[last_index + id];
        }

        auto begin() {
            return m_values.begin();
        }

        auto end() {
            return m_values.end();
        }

        auto begin() const {
            return m_values.begin();
        }

        auto end() const {
            return m_values.end();
        }

    private:
        std::vector<Value> m_values;
        std::vector<std::size_t> m_frame_indices;
    };

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

    class Avm {
    public:
        Avm(const std::vector<Inst>& program) :
            m_program(program),
            m_program_cnt(0),
            m_stack(),
            m_ret_stack(),
            m_local_stack(),
            m_mem(m_stack, m_local_stack),
            m_exit_requested(false) { 
            m_stack.reserve(1024);
            m_ret_stack.reserve(1024);
            m_local_stack.reserve(1024);
        }

        void execute_inst(const Inst& inst) {
            switch (inst.op) {
            case Op::Nop:
                break;
            
            case Op::StoreLocal: {
                std::uint64_t id = inst.a.uinteger();
                m_local_stack[id] = m_stack.pop();
            } break;
            case Op::LoadLocal: {
                std::uint64_t id = inst.a.uinteger();
                m_stack.push(m_local_stack[id]);
            } break;
            case Op::Push: {
                m_stack.push(inst.a);
            } break;

            case Op::MakeMem: {
                MemHeader* header = m_mem.make(1);
                m_stack.push(Value::reference(header));
            } break;
            case Op::MakeMemCount: {
                std::uint64_t count = m_stack.pop().uinteger();
                MemHeader* header = m_mem.make(count);
                m_stack.push(Value::reference(header));
            } break;
            case Op::MakeMemCountIm: {
                std::uint64_t count = inst.a.uinteger();
                MemHeader* header = m_mem.make(count);
                m_stack.push(Value::reference(header));
            } break;
            case Op::StoreMem: {
                Value value = m_stack.pop();
                MemHeader* header = m_stack.pop().reference();
                m_mem.get(header, 0) = value;
            } break;
            case Op::StoreMemIm: {
                Value value = inst.a;
                MemHeader* header = m_stack.pop().reference();
                m_mem.get(header, 0) = value;
            } break;
            case Op::StoreMemIdx: {
                Value value = m_stack.pop();
                std::uint64_t idx = m_stack.pop().uinteger();
                MemHeader* header = m_stack.pop().reference();
                m_mem.get(header, idx) = value;
            } break;
            case Op::StoreMemImIdx: {
                Value value = inst.a;
                std::uint64_t idx = m_stack.pop().uinteger();
                MemHeader* header = m_stack.pop().reference();
                m_mem.get(header, idx) = value;
            } break;
            case Op::StoreMemIdxIm: {
                Value value = m_stack.pop();
                std::uint64_t idx = inst.a.uinteger();
                MemHeader* header = m_stack.pop().reference();
                m_mem.get(header, idx) = value;
            } break;
            case Op::StoreMemImIdxIm: {
                Value value = inst.b;
                std::uint64_t idx = inst.a.uinteger();
                MemHeader* header = m_stack.pop().reference();
                m_mem.get(header, idx) = value;
            } break;
            case Op::LoadMem: {
                MemHeader* header = m_stack.pop().reference();
                m_stack.push(m_mem.get(header, 0));
            } break;
            case Op::LoadMemIdx: {
                std::uint64_t idx = m_stack.pop().uinteger();
                MemHeader* header = m_stack.pop().reference();
                m_stack.push(m_mem.get(header, idx));
            } break;
            case Op::LoadMemIdxIm: {
                std::uint64_t idx = inst.a.uinteger();
                MemHeader* header = m_stack.pop().reference();
                m_stack.push(m_mem.get(header, idx));
            } break;

            case Op::Copy: {
                std::uint64_t addr = inst.a.uinteger();
                m_stack.push(m_stack[addr]);
            } break;
            case Op::Swap: {
                std::uint64_t addr = inst.a.uinteger();
                Value temp = m_stack[0];
                m_stack[0] = m_stack[addr];
                m_stack[addr] = temp;
            } break;

            case Op::PrintInt: {
                std::int64_t val = m_stack.pop().integer();
                std::cout << val << std::endl;
            } break;
            case Op::PrintUint: {
                std::uint64_t val = m_stack.pop().uinteger();
                std::cout << val << std::endl;
            } break;
            case Op::PrintBool: {
                std::uint8_t val = m_stack.pop().boolean();
                std::cout << (val ? "true" : "false") << std::endl;
            } break;
            case Op::PrintRef: {
                MemHeader* val = m_stack.pop().reference();
                std::cout << "0x" 
                          << std::hex 
                          << std::uintptr_t(val) 
                          << std::dec 
                          << std::endl;
            } break;
            case Op::PrintFloat: {
                double val = m_stack.pop().floating();
                std::cout << val << std::endl;
            } break;

            case Op::AddInt: {
                std::int64_t b = m_stack.pop().integer();
                std::int64_t a = m_stack.pop().integer();
                m_stack.push(Value::integer(a + b));
            } break;
            case Op::SubInt: {
                std::int64_t b = m_stack.pop().integer();
                std::int64_t a = m_stack.pop().integer();
                m_stack.push(Value::integer(a - b));
            } break;
            case Op::MulInt: {
                std::int64_t b = m_stack.pop().integer();
                std::int64_t a = m_stack.pop().integer();
                m_stack.push(Value::integer(a * b));
            } break;
            case Op::DivInt: {
                std::int64_t b = m_stack.pop().integer();
                std::int64_t a = m_stack.pop().integer();
                m_stack.push(Value::integer(a / b));
            } break;
            case Op::UnMinInt: {
                std::int64_t a = m_stack.pop().integer();
                m_stack.push(Value::integer(-a));
            } break;
            case Op::ModInt: {
                std::int64_t b = m_stack.pop().integer();
                std::int64_t a = m_stack.pop().integer();
                m_stack.push(Value::integer(a % b));
            } break;
            case Op::EqInt: {
                std::int64_t b = m_stack.pop().integer();
                std::int64_t a = m_stack.pop().integer();
                m_stack.push(Value::boolean(a == b));
            } break;
            case Op::NeqInt: {
                std::int64_t b = m_stack.pop().integer();
                std::int64_t a = m_stack.pop().integer();
                m_stack.push(Value::boolean(a != b));
            } break;
            case Op::LtInt: {
                std::int64_t b = m_stack.pop().integer();
                std::int64_t a = m_stack.pop().integer();
                m_stack.push(Value::boolean(a < b));
            } break;
            case Op::GtInt: {
                std::int64_t b = m_stack.pop().integer();
                std::int64_t a = m_stack.pop().integer();
                m_stack.push(Value::boolean(a > b));
            } break;
            case Op::LtEqInt: {
                std::int64_t b = m_stack.pop().integer();
                std::int64_t a = m_stack.pop().integer();
                m_stack.push(Value::boolean(a <= b));
            } break;
            case Op::GtEqInt: {
                std::int64_t b = m_stack.pop().integer();
                std::int64_t a = m_stack.pop().integer();
                m_stack.push(Value::boolean(a >= b));
            } break;
            case Op::BinAndInt: {
                std::int64_t b = m_stack.pop().integer();
                std::int64_t a = m_stack.pop().integer();
                m_stack.push(Value::integer(a & b));
            } break;
            case Op::BinOrInt: {
                std::int64_t b = m_stack.pop().integer();
                std::int64_t a = m_stack.pop().integer();
                m_stack.push(Value::integer(a | b));
            } break;
            case Op::BinXorInt: {
                std::int64_t b = m_stack.pop().integer();
                std::int64_t a = m_stack.pop().integer();
                m_stack.push(Value::integer(a ^ b));
            } break;
            case Op::BinNotInt: {
                std::int64_t val = m_stack.pop().integer();
                m_stack.push(Value::integer(~val));
            } break;
            case Op::LShiftInt: {
                std::uint64_t b = m_stack.pop().uinteger();
                std::int64_t a = m_stack.pop().integer();
                m_stack.push(Value::integer(a << b));
            } break;
            case Op::RShiftInt: {
                std::uint64_t b = m_stack.pop().uinteger();
                std::int64_t a = m_stack.pop().integer();
                m_stack.push(Value::integer(a >> b));
            } break;

            case Op::AddUint: {
                std::uint64_t b = m_stack.pop().uinteger();
                std::uint64_t a = m_stack.pop().uinteger();
                m_stack.push(Value::uinteger(a + b));
            } break;
            case Op::SubUint: {
                std::uint64_t b = m_stack.pop().uinteger();
                std::uint64_t a = m_stack.pop().uinteger();
                m_stack.push(Value::uinteger(a - b));
            } break;
            case Op::MulUint: {
                std::uint64_t b = m_stack.pop().uinteger();
                std::uint64_t a = m_stack.pop().uinteger();
                m_stack.push(Value::uinteger(a * b));
            } break;
            case Op::DivUint: {
                std::uint64_t b = m_stack.pop().uinteger();
                std::uint64_t a = m_stack.pop().uinteger();
                m_stack.push(Value::uinteger(a / b));
            } break;
            case Op::UnMinUint: {
                std::uint64_t a = m_stack.pop().uinteger();
                m_stack.push(Value::uinteger(-a));
            } break;
            case Op::ModUint: {
                std::uint64_t b = m_stack.pop().uinteger();
                std::uint64_t a = m_stack.pop().uinteger();
                m_stack.push(Value::uinteger(a % b));
            } break;
            case Op::EqUint: {
                std::uint64_t b = m_stack.pop().uinteger();
                std::uint64_t a = m_stack.pop().uinteger();
                m_stack.push(Value::boolean(a == b));
            } break;
            case Op::NeqUint: {
                std::uint64_t b = m_stack.pop().uinteger();
                std::uint64_t a = m_stack.pop().uinteger();
                m_stack.push(Value::boolean(a != b));
            } break;
            case Op::LtUint: {
                std::uint64_t b = m_stack.pop().uinteger();
                std::uint64_t a = m_stack.pop().uinteger();
                m_stack.push(Value::boolean(a < b));
            } break;
            case Op::GtUint: {
                std::uint64_t b = m_stack.pop().uinteger();
                std::uint64_t a = m_stack.pop().uinteger();
                m_stack.push(Value::boolean(a > b));
            } break;
            case Op::LtEqUint: {
                std::uint64_t b = m_stack.pop().uinteger();
                std::uint64_t a = m_stack.pop().uinteger();
                m_stack.push(Value::boolean(a <= b));
            } break;
            case Op::GtEqUint: {
                std::uint64_t b = m_stack.pop().uinteger();
                std::uint64_t a = m_stack.pop().uinteger();
                m_stack.push(Value::boolean(a >= b));
            } break;
            case Op::BinAndUint: {
                std::uint64_t b = m_stack.pop().uinteger();
                std::uint64_t a = m_stack.pop().uinteger();
                m_stack.push(Value::uinteger(a & b));
            } break;
            case Op::BinOrUint: {
                std::uint64_t b = m_stack.pop().uinteger();
                std::uint64_t a = m_stack.pop().uinteger();
                m_stack.push(Value::uinteger(a | b));
            } break;
            case Op::BinXorUint: {
                std::uint64_t b = m_stack.pop().uinteger();
                std::uint64_t a = m_stack.pop().uinteger();
                m_stack.push(Value::uinteger(a ^ b));
            } break;
            case Op::BinNotUint: {
                std::uint64_t val = m_stack.pop().uinteger();
                m_stack.push(Value::uinteger(~val));
            } break;
            case Op::LShiftUint: {
                std::uint64_t b = m_stack.pop().uinteger();
                std::uint64_t a = m_stack.pop().uinteger();
                m_stack.push(Value::uinteger(a << b));
            } break;
            case Op::RShiftUint: {
                std::uint64_t b = m_stack.pop().uinteger();
                std::uint64_t a = m_stack.pop().uinteger();
                m_stack.push(Value::uinteger(a >> b));
            } break;

            case Op::AddFloat: {
                double b = m_stack.pop().floating();
                double a = m_stack.pop().floating();
                m_stack.push(Value::floating(a + b));
            } break;
            case Op::SubFloat: {
                double b = m_stack.pop().floating();
                double a = m_stack.pop().floating();
                m_stack.push(Value::floating(a - b));
            } break;
            case Op::MulFloat: {
                double b = m_stack.pop().floating();
                double a = m_stack.pop().floating();
                m_stack.push(Value::floating(a * b));
            } break;
            case Op::DivFloat: {
                double b = m_stack.pop().floating();
                double a = m_stack.pop().floating();
                m_stack.push(Value::floating(a / b));
            } break;
            case Op::UnMinFloat: {
                double a = m_stack.pop().floating();
                m_stack.push(Value::floating(-a));
            } break;
            case Op::EqFloat: {
                double b = m_stack.pop().floating();
                double a = m_stack.pop().floating();
                m_stack.push(Value::boolean(a == b));
            } break;
            case Op::NeqFloat: {
                double b = m_stack.pop().floating();
                double a = m_stack.pop().floating();
                m_stack.push(Value::boolean(a != b));
            } break;
            case Op::LtFloat: {
                double b = m_stack.pop().floating();
                double a = m_stack.pop().floating();
                m_stack.push(Value::boolean(a < b));
            } break;
            case Op::GtFloat: {
                double b = m_stack.pop().floating();
                double a = m_stack.pop().floating();
                m_stack.push(Value::boolean(a > b));
            } break;
            case Op::LtEqFloat: {
                double b = m_stack.pop().floating();
                double a = m_stack.pop().floating();
                m_stack.push(Value::boolean(a <= b));
            } break;
            case Op::GtEqFloat: {
                double b = m_stack.pop().floating();
                double a = m_stack.pop().floating();
                m_stack.push(Value::boolean(a >= b));
            } break;

            case Op::AndBool: {
                std::uint8_t b = m_stack.pop().boolean();
                std::uint8_t a = m_stack.pop().boolean();
                m_stack.push(Value::boolean(a && b));
            } break;
            case Op::OrBool: {
                std::uint8_t b = m_stack.pop().boolean();
                std::uint8_t a = m_stack.pop().boolean();
                m_stack.push(Value::boolean(a || b));
            } break;
            case Op::XorBool: {
                std::uint8_t b = m_stack.pop().boolean();
                std::uint8_t a = m_stack.pop().boolean();
                m_stack.push(Value::boolean(a ^ b));
            } break;
            case Op::NotBool: {
                std::uint8_t val = m_stack.pop().boolean();
                m_stack.push(Value::boolean(!val));
            } break;
            case Op::EqBool: {
                std::uint8_t b = m_stack.pop().boolean();
                std::uint8_t a = m_stack.pop().boolean();
                m_stack.push(Value::boolean(a == b));
            } break;
            case Op::NeqBool: {
                std::uint8_t b = m_stack.pop().boolean();
                std::uint8_t a = m_stack.pop().boolean();
                m_stack.push(Value::boolean(a != b));
            } break;

            case Op::CIntUint: {
                std::int64_t a = m_stack.pop().integer();
                m_stack.push(Value::uinteger(std::uint64_t(a)));
            } break;
            case Op::CIntFloat: {
                std::int64_t a = m_stack.pop().integer();
                m_stack.push(Value::floating(float(a)));
            } break;
            case Op::CUintInt: {
                std::uint64_t a = m_stack.pop().uinteger();
                m_stack.push(Value::integer(std::int64_t(a)));
            } break;
            case Op::CUintFloat: {
                std::uint64_t a = m_stack.pop().uinteger();
                m_stack.push(Value::floating(float(a)));
            } break;
            case Op::CFloatInt: {
                float a = m_stack.pop().floating();
                m_stack.push(Value::integer(std::int64_t(a)));
            } break;
            case Op::CFloatUint: {
                float a = m_stack.pop().floating();
                m_stack.push(Value::uinteger(std::uint64_t(a)));
            } break;

            case Op::If: {
                std::uint64_t true_addr = inst.a.uinteger();
                std::uint64_t false_addr = inst.b.uinteger();
                std::uint8_t cond = m_stack.pop().boolean();
                m_program_cnt = cond ? true_addr : false_addr;
                return;
            } break;
            case Op::Call: {
                std::uint64_t addr = m_stack.pop().uinteger();
                m_ret_stack.push(m_program_cnt + 1);
                m_program_cnt = addr;
                m_local_stack.push();
                return;
            } break;
            case Op::CallIm: {
                std::uint64_t addr = inst.a.uinteger();
                m_ret_stack.push(m_program_cnt + 1);
                m_program_cnt = addr;
                m_local_stack.push();
                return;
            } break;
            case Op::Ret: {
                std::uint64_t addr = m_ret_stack.pop();
                m_program_cnt = addr;
                m_local_stack.pop();
                return;
            } break;
            case Op::Goto: {
                std::uint64_t addr = m_stack.pop().uinteger();
                m_program_cnt = addr;
                return;
            } break;
            case Op::GotoIm: {
                std::uint64_t addr = inst.a.uinteger();
                m_program_cnt = addr;
                return;
            } break;

            case Op::CallIntrin: {
                std::uint8_t intrin_num = inst.a.uinteger();
                Intrin intrin = static_cast<Intrin>(intrin_num);
                call_intrin(intrin);
            } break;

            case Op::Exit: {
                m_exit_requested = true;
                return;
            } break;
            }
            m_program_cnt++;
        }

        void call_intrin(Intrin intrin) {
            switch (intrin) {
            case Intrin::MinInt: {
                std::int64_t b = m_stack.pop().integer();
                std::int64_t a = m_stack.pop().integer();
                m_stack.push(Value::integer(std::min(a, b)));
            } break;
            case Intrin::MaxInt: {
                std::int64_t b = m_stack.pop().integer();
                std::int64_t a = m_stack.pop().integer();
                m_stack.push(Value::integer(std::max(a, b)));
            } break;
            case Intrin::SignInt: {
                std::int64_t a = m_stack.pop().integer();
                std::int64_t sign = (a > 0) ? 1 : ((a < 0) ? -1 : 0);
                m_stack.push(Value::integer(sign));
            } break;
            case Intrin::AbsInt: {
                std::int64_t a = m_stack.pop().integer();
                m_stack.push(Value::integer(std::abs(a)));
            } break;

            case Intrin::MinUint: {
                std::uint64_t b = m_stack.pop().uinteger();
                std::uint64_t a = m_stack.pop().uinteger();
                m_stack.push(Value::uinteger(std::min(a, b)));
            } break;
            case Intrin::MaxUint: {
                std::uint64_t b = m_stack.pop().uinteger();
                std::uint64_t a = m_stack.pop().uinteger();
                m_stack.push(Value::uinteger(std::max(a, b)));
            } break;

            case Intrin::FloorFloat: {
                double a = m_stack.pop().floating();
                m_stack.push(Value::floating(std::floor(a)));
            } break;
            case Intrin::CeilFloat: {
                double a = m_stack.pop().floating();
                m_stack.push(Value::floating(std::ceil(a)));
            } break;
            case Intrin::RoundFloat: {
                double a = m_stack.pop().floating();
                m_stack.push(Value::floating(std::round(a)));
            } break;
            case Intrin::TruncFloat: {
                double a = m_stack.pop().floating();
                m_stack.push(Value::floating(std::trunc(a)));
            } break;
            case Intrin::IsNanFloat: {
                double a = m_stack.pop().floating();
                m_stack.push(Value::boolean(std::isnan(a)));
            } break;
            case Intrin::IsInfFloat: {
                double a = m_stack.pop().floating();
                m_stack.push(Value::boolean(std::isinf(a)));
            } break;
            case Intrin::IsFinFloat: {
                double a = m_stack.pop().floating();
                m_stack.push(Value::boolean(std::isfinite(a)));
            } break;
            case Intrin::SignBitFloat: {
                double a = m_stack.pop().floating();
                m_stack.push(Value::boolean(std::signbit(a)));
            } break;
            case Intrin::FmaFloat: {
                double c = m_stack.pop().floating();
                double b = m_stack.pop().floating();
                double a = m_stack.pop().floating();
                m_stack.push(Value::floating(std::fma(a, b, c)));
            } break;
            case Intrin::MinFloat: {
                double b = m_stack.pop().floating();
                double a = m_stack.pop().floating();
                m_stack.push(Value::floating(std::min(a, b)));
            } break;
            case Intrin::MaxFloat: {
                double b = m_stack.pop().floating();
                double a = m_stack.pop().floating();
                m_stack.push(Value::floating(std::max(a, b)));
            } break;
            case Intrin::SqrtFloat: {
                double a = m_stack.pop().floating();
                m_stack.push(Value::floating(std::sqrt(a)));
            } break;
            case Intrin::SinFloat: {
                double a = m_stack.pop().floating();
                m_stack.push(Value::floating(std::sin(a)));
            } break;
            case Intrin::CosFloat: {
                double a = m_stack.pop().floating();
                m_stack.push(Value::floating(std::cos(a)));
            } break;
            case Intrin::TanFloat: {
                double a = m_stack.pop().floating();
                m_stack.push(Value::floating(std::tan(a)));
            } break;
            case Intrin::ExpFloat: {
                double a = m_stack.pop().floating();
                m_stack.push(Value::floating(std::exp(a)));
            } break;
            case Intrin::Exp2Float: {
                double a = m_stack.pop().floating();
                m_stack.push(Value::floating(std::exp2(a)));
            } break;
            case Intrin::LogFloat: {
                double a = m_stack.pop().floating();
                m_stack.push(Value::floating(std::log(a)));
            } break;
            case Intrin::Log2Float: {
                double a = m_stack.pop().floating();
                m_stack.push(Value::floating(std::log2(a)));
            } break;
            case Intrin::Log10Float: {
                double a = m_stack.pop().floating();
                m_stack.push(Value::floating(std::log10(a)));
            } break;
            case Intrin::PowFloat: {
                double b = m_stack.pop().floating();
                double a = m_stack.pop().floating();
                m_stack.push(Value::floating(std::pow(a, b)));
            } break;
            case Intrin::SignFloat: {
                double a = m_stack.pop().floating();
                std::int64_t sign = (a > 0.0) ? 1 : ((a < 0.0) ? -1 : 0);
                m_stack.push(Value::integer(sign));
            } break;
            case Intrin::AbsFloat: {
                double a = m_stack.pop().floating();
                m_stack.push(Value::floating(std::abs(a)));
            } break;
            }
        }

        void run() {
            while (true) {
                if (m_exit_requested) return;
                if (m_program_cnt >= m_program.size()) return;
                execute_inst(m_program[m_program_cnt]);
            }
        }

    private:
        std::vector<Inst> m_program;
        std::size_t m_program_cnt;
        Stack<Value> m_stack;
        Stack<std::uint64_t> m_ret_stack;
        LocalStack m_local_stack;
        Mem m_mem;
        bool m_exit_requested;
    };

}

int main() {
    using namespace avm;

    std::vector<Elem> elems = {
        Elem::call_im("main"),
        Elem::inst({ Op::Exit }),
        
        Elem::label("fib"),
            Elem::inst({ Op::StoreLocal, Value::uinteger(0) }),
            Elem::inst({ Op::LoadLocal, Value::uinteger(0) }),
            Elem::inst({ Op::Push, Value::integer(2) }), 
            Elem::inst({ Op::LtInt }),
            Elem::if_("lt", "gteq"),
            Elem::label("lt"),
                Elem::inst({ Op::LoadLocal, Value::uinteger(0) }),
                Elem::inst({ Op::Ret }),
            Elem::label("gteq"),
                Elem::inst({ Op::LoadLocal, Value::uinteger(0) }),
                Elem::inst({ Op::Push, Value::integer(1) }),
                Elem::inst({ Op::SubInt }),
                Elem::call_im("fib"),
                Elem::inst({ Op::LoadLocal, Value::uinteger(0) }),
                Elem::inst({ Op::Push, Value::integer(2) }),
                Elem::inst({ Op::SubInt }),
                Elem::call_im("fib"),
                Elem::inst({ Op::AddInt }),
                Elem::inst({ Op::Ret }),

        Elem::label("factorial"),
            Elem::inst({ Op::StoreLocal, Value::uinteger(0) }),
            Elem::inst({ Op::LoadLocal, Value::uinteger(0) }),
            Elem::inst({ Op::Push, Value::integer(2) }),
            Elem::inst({ Op::LtEqInt }),
            Elem::if_("lteq", "gt"),
            Elem::label("lteq"),
                Elem::inst({ Op::LoadLocal, Value::uinteger(0) }),
                Elem::inst({ Op::Ret }),
            Elem::label("gt"),
                Elem::inst({ Op::LoadLocal, Value::uinteger(0) }),
                Elem::inst({ Op::Push, Value::integer(1) }),
                Elem::inst({ Op::SubInt }),
                Elem::call_im("factorial"),
                Elem::inst({ Op::LoadLocal, Value::uinteger(0) }),
                Elem::inst({ Op::MulInt }),
                Elem::inst({ Op::Ret }),
    
        Elem::label("main"),

        Elem::inst({ Op::Push, Value::integer(19)}),
        Elem::call_im("fib"),
        Elem::inst({ Op::PrintInt }),

        Elem::inst({ Op::Push, Value::integer(19) }),
        Elem::call_im("factorial"),
        Elem::inst({ Op::PrintInt }),

        Elem::inst({ Op::MakeMem }),
        Elem::inst({ Op::StoreLocal, Value::uinteger(0)}),
        Elem::inst({ Op::LoadLocal, Value::uinteger(0)}),
        Elem::inst({ Op::StoreMemIm, Value::integer(10)}),
        Elem::inst({ Op::LoadLocal, Value::uinteger(0)}),
        Elem::inst({ Op::LoadMem }),
        Elem::inst({ Op::PrintInt }),

        Elem::inst({ Op::MakeMemCountIm, Value::uinteger(10) }),
        Elem::inst({ Op::StoreLocal, Value::uinteger(1) }),
        Elem::inst({ Op::LoadLocal, Value::uinteger(1) }),
        Elem::inst({ Op::StoreMemImIdxIm, Value::uinteger(0), 
            Value::integer(10) }),
        Elem::inst({ Op::LoadLocal, Value::uinteger(1) }),
        Elem::inst({ Op::StoreMemImIdxIm, Value::uinteger(1), 
            Value::integer(11) }),
        Elem::inst({ Op::LoadLocal, Value::uinteger(1) }),
        Elem::inst({ Op::StoreMemImIdxIm, Value::uinteger(2), 
            Value::integer(12) }),
        Elem::inst({ Op::LoadLocal, Value::uinteger(1) }),
        Elem::inst({ Op::StoreMemImIdxIm, Value::uinteger(3), 
            Value::integer(13) }),
        Elem::inst({ Op::LoadLocal, Value::uinteger(1) }),
        Elem::inst({ Op::StoreMemImIdxIm, Value::uinteger(4), 
            Value::integer(14) }),
        Elem::inst({ Op::LoadLocal, Value::uinteger(1) }),
        Elem::inst({ Op::StoreMemImIdxIm, Value::uinteger(5), 
            Value::integer(15) }),
        Elem::inst({ Op::LoadLocal, Value::uinteger(1) }),
        Elem::inst({ Op::StoreMemImIdxIm, Value::uinteger(6), 
            Value::integer(16) }),
        Elem::inst({ Op::LoadLocal, Value::uinteger(1) }),
        Elem::inst({ Op::StoreMemImIdxIm, Value::uinteger(7), 
            Value::integer(17) }),
        Elem::inst({ Op::LoadLocal, Value::uinteger(1) }),
        Elem::inst({ Op::StoreMemImIdxIm, Value::uinteger(8), 
            Value::integer(18) }),
        Elem::inst({ Op::LoadLocal, Value::uinteger(1) }),
        Elem::inst({ Op::StoreMemImIdxIm, Value::uinteger(9), 
            Value::integer(19) }),

        Elem::inst({ Op::LoadLocal, Value::uinteger(1) }),
        Elem::inst({ Op::LoadMemIdxIm, Value::uinteger(0) }),
        Elem::inst({ Op::PrintInt }),
        Elem::inst({ Op::LoadLocal, Value::uinteger(1) }),
        Elem::inst({ Op::LoadMemIdxIm, Value::uinteger(1) }),
        Elem::inst({ Op::PrintInt }),
        Elem::inst({ Op::LoadLocal, Value::uinteger(1) }),
        Elem::inst({ Op::LoadMemIdxIm, Value::uinteger(2) }),
        Elem::inst({ Op::PrintInt }),
        Elem::inst({ Op::LoadLocal, Value::uinteger(1) }),
        Elem::inst({ Op::LoadMemIdxIm, Value::uinteger(3) }),
        Elem::inst({ Op::PrintInt }),
        Elem::inst({ Op::LoadLocal, Value::uinteger(1) }),
        Elem::inst({ Op::LoadMemIdxIm, Value::uinteger(4) }),
        Elem::inst({ Op::PrintInt }),
        Elem::inst({ Op::LoadLocal, Value::uinteger(1) }),
        Elem::inst({ Op::LoadMemIdxIm, Value::uinteger(5) }),
        Elem::inst({ Op::PrintInt }),
        Elem::inst({ Op::LoadLocal, Value::uinteger(1) }),
        Elem::inst({ Op::LoadMemIdxIm, Value::uinteger(6) }),
        Elem::inst({ Op::PrintInt }),
        Elem::inst({ Op::LoadLocal, Value::uinteger(1) }),
        Elem::inst({ Op::LoadMemIdxIm, Value::uinteger(7) }),
        Elem::inst({ Op::PrintInt }),
        Elem::inst({ Op::LoadLocal, Value::uinteger(1) }),
        Elem::inst({ Op::LoadMemIdxIm, Value::uinteger(8) }),
        Elem::inst({ Op::PrintInt }),
        Elem::inst({ Op::LoadLocal, Value::uinteger(1) }),
        Elem::inst({ Op::LoadMemIdxIm, Value::uinteger(9) }),
        Elem::inst({ Op::PrintInt }),

        Elem::inst({ Op::Ret })
    };

    Resolver resolver(elems);
    auto program = resolver.resolve();

    Avm avm(program);

    avm.run();

    return 0;
}