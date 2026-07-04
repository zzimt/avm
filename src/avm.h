#ifndef AVM_AVM_H_
#define AVM_AVM_H_

#include <cstddef>
#include <vector>
#include <iostream>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <type_traits>
#include <optional>

#include "inst.h"
#include "stack.h"
#include "localstack.h"
#include "mem.h"
#include "stringstore.h"
#include "externs.h"

namespace avm {

    enum class AvmPanicWhy {
        DivisionByZero,
        OutOfBoundsMemAccess,
        AttemptToAllocateZeroElements,
        ExplicitPanic,
    };

    class AvmPanic {
    public:
        inline AvmPanic(
            AvmPanicWhy why, 
            const std::vector<std::size_t>& stack_trace
        ) : m_why(why),
            m_stack_trace(stack_trace) { }

        inline AvmPanicWhy why() const {
            return m_why;
        }

        inline const std::vector<std::size_t>& stack_trace() const {
            return m_stack_trace;
        }

    private:
        AvmPanicWhy m_why;
        std::vector<std::size_t> m_stack_trace;
    };

    class AvmResult {
    public:
        static inline AvmResult ok() {
            return AvmResult(std::nullopt);
        }

        static inline AvmResult panic(const AvmPanic& panic) {
            return AvmResult(std::make_optional(panic));
        }

        inline bool is_ok() const {
            return !m_panic.has_value();
        }

        inline AvmPanic panic() const {
            return *m_panic;
        }

    private:
        AvmResult(std::optional<AvmPanic> panic) :
            m_panic(panic) { }

        std::optional<AvmPanic> m_panic;
    };

    class Avm {
    public:
        Avm(
            const std::vector<Inst>& program,
            Externs& externs,
            StringStore& string_store
        ) :
            m_program(program),
            m_program_cnt(0),
            m_stack(),
            m_ret_stack(),
            m_local_stack(),
            m_mem(m_stack, m_local_stack),
            m_exit_requested(false),
            m_externs(externs),
            m_string_store(string_store) {
            m_stack.reserve(1024);
            m_ret_stack.reserve(1024);
            m_local_stack.reserve(1024);
        }

        void execute_inst(const Inst& inst) {
            switch (inst.op) {
            case Op::Nop:
                break;
            
            case Op::StoreLocal: {
                Uint id = inst.a.uinteger();
                m_local_stack[id] = m_stack.pop();
            } break;
            case Op::LoadLocal: {
                Uint id = inst.a.uinteger();
                m_stack.push(m_local_stack[id]);
            } break;
            case Op::Push: {
                m_stack.push(inst.a);
            } break;
            case Op::PushStr: {
                Uint id = inst.a.uinteger();
                StrHeader* header = m_mem.str_intern(m_string_store[id]);
                m_stack.push(Value::string(header));
            } break;

            case Op::MakeMem: {
                Ref header = m_mem.make(1);
                m_stack.push(Value::reference(header));
            } break;
            case Op::MakeMemCount: {
                Uint count = m_stack.pop().uinteger();
                Ref header = m_mem.make(count);
                if (!header) {
                    panic(AvmPanicWhy::AttemptToAllocateZeroElements);
                    return;
                }
                m_stack.push(Value::reference(header));
            } break;
            case Op::MakeMemCountIm: {
                Uint count = inst.a.uinteger();
                Ref header = m_mem.make(count);
                if (!header) {
                    panic(AvmPanicWhy::AttemptToAllocateZeroElements);
                    return;
                }
                m_stack.push(Value::reference(header));
            } break;
            case Op::StoreMem: {
                Value value = m_stack.pop();
                Ref header = m_stack.pop().reference();
                auto* storage = m_mem.get(header, 0);
                if (!storage) {
                    panic(AvmPanicWhy::OutOfBoundsMemAccess);
                    return;
                }
                *storage = value;
            } break;
            case Op::StoreMemIm: {
                Value value = inst.a;
                Ref header = m_stack.pop().reference();
                auto* storage = m_mem.get(header, 0);
                if (!storage) {
                    panic(AvmPanicWhy::OutOfBoundsMemAccess);
                    return;
                }
                *storage = value;
            } break;
            case Op::StoreMemIdx: {
                Value value = m_stack.pop();
                Uint idx = m_stack.pop().uinteger();
                Ref header = m_stack.pop().reference();
                auto* storage = m_mem.get(header, idx);
                if (!storage) {
                    panic(AvmPanicWhy::OutOfBoundsMemAccess);
                    return;
                }
                *storage = value;
            } break;
            case Op::StoreMemImIdx: {
                Value value = inst.a;
                Uint idx = m_stack.pop().uinteger();
                Ref header = m_stack.pop().reference();
                auto* storage = m_mem.get(header, idx);
                if (!storage) {
                    panic(AvmPanicWhy::OutOfBoundsMemAccess);
                    return;
                }
                *storage = value;
            } break;
            case Op::StoreMemIdxIm: {
                Value value = m_stack.pop();
                Uint idx = inst.a.uinteger();
                Ref header = m_stack.pop().reference();
                auto* storage = m_mem.get(header, idx);
                if (!storage) {
                    panic(AvmPanicWhy::OutOfBoundsMemAccess);
                    return;
                }
                *storage = value;
            } break;
            case Op::StoreMemImIdxIm: {
                Value value = inst.b;
                Uint idx = inst.a.uinteger();
                Ref header = m_stack.pop().reference();
                auto* storage = m_mem.get(header, idx);
                if (!storage) {
                    panic(AvmPanicWhy::OutOfBoundsMemAccess);
                    return;
                }
                *storage = value;
            } break;
            case Op::LoadMem: {
                Ref header = m_stack.pop().reference();
                auto* storage = m_mem.get(header, 0);
                if (!storage) {
                    panic(AvmPanicWhy::OutOfBoundsMemAccess);
                    return;
                }
                m_stack.push(*storage);
            } break;
            case Op::LoadMemIdx: {
                Uint idx = m_stack.pop().uinteger();
                Ref header = m_stack.pop().reference();
                auto* storage = m_mem.get(header, idx);
                if (!storage) {
                    panic(AvmPanicWhy::OutOfBoundsMemAccess);
                    return;
                }
                m_stack.push(*storage);
            } break;
            case Op::LoadMemIdxIm: {
                Uint idx = inst.a.uinteger();
                Ref header = m_stack.pop().reference();
                auto* storage = m_mem.get(header, idx);
                if (!storage) {
                    panic(AvmPanicWhy::OutOfBoundsMemAccess);
                    return;
                }
                m_stack.push(*storage);
            } break;

            case Op::MakeMemUniformInt: {
                Uint count = m_stack.pop().uinteger();
                Ref header = m_mem.make_uniform<Int>(count);
                if (!header) {
                    panic(AvmPanicWhy::AttemptToAllocateZeroElements);
                    return;
                }
                m_stack.push(Value::reference(header));
            } break;
            case Op::StoreMemUniformInt: {
                Int value = m_stack.pop().integer();
                Uint index = m_stack.pop().uinteger();
                Ref header = m_stack.pop().reference();
                auto* storage = m_mem.get_uniform<Int>(header, index);
                if (!storage) {
                    panic(AvmPanicWhy::OutOfBoundsMemAccess);
                    return;
                }
                *storage = value;
            } break;
            case Op::LoadMemUniformInt: {
                Uint index = m_stack.pop().uinteger();
                Ref header = m_stack.pop().reference();
                auto* storage = m_mem.get_uniform<Int>(header, index);
                if (!storage) {
                    panic(AvmPanicWhy::OutOfBoundsMemAccess);
                    return;
                }
                Int res = *storage;
                m_stack.push(Value::integer(res));
            } break;

            case Op::MakeMemUniformUint: {
                Uint count = m_stack.pop().uinteger();
                Ref header = m_mem.make_uniform<Uint>(count);
                if (!header) {
                    panic(AvmPanicWhy::AttemptToAllocateZeroElements);
                    return;
                }
                m_stack.push(Value::reference(header));
            } break;
            case Op::StoreMemUniformUint: {
                Uint value = m_stack.pop().uinteger();
                Uint index = m_stack.pop().uinteger();
                Ref header = m_stack.pop().reference();
                auto* storage = m_mem.get_uniform<Uint>(header, index);
                if (!storage) {
                    panic(AvmPanicWhy::OutOfBoundsMemAccess);
                    return;
                }
                *storage = value;
            } break;
            case Op::LoadMemUniformUint: {
                Uint index = m_stack.pop().uinteger();
                Ref header = m_stack.pop().reference();
                auto* storage = m_mem.get_uniform<Uint>(header, index);
                if (!storage) {
                    panic(AvmPanicWhy::OutOfBoundsMemAccess);
                    return;
                }
                Uint res = *storage;
                m_stack.push(Value::uinteger(res));
            } break;

            case Op::MakeMemUniformByte: {
                Uint count = m_stack.pop().uinteger();
                Ref header = m_mem.make_uniform<Byte>(count);
                if (!header) {
                    panic(AvmPanicWhy::AttemptToAllocateZeroElements);
                    return;
                }
                m_stack.push(Value::reference(header));
            } break;
            case Op::StoreMemUniformByte: {
                Byte value = m_stack.pop().byte();
                Uint index = m_stack.pop().uinteger();
                Ref header = m_stack.pop().reference();
                auto* storage = m_mem.get_uniform<Byte>(header, index);
                if (!storage) {
                    panic(AvmPanicWhy::OutOfBoundsMemAccess);
                    return;
                }
                *storage = value;
            } break;
            case Op::LoadMemUniformByte: {
                Uint index = m_stack.pop().uinteger();
                Ref header = m_stack.pop().reference();
                auto* storage = m_mem.get_uniform<Byte>(header, index);
                if (!storage) {
                    panic(AvmPanicWhy::OutOfBoundsMemAccess);
                    return;
                }
                Byte res = *storage;
                m_stack.push(Value::byte(res));
            } break;

            case Op::MakeMemUniformFloat: {
                Uint count = m_stack.pop().uinteger();
                Ref header = m_mem.make_uniform<Float>(count);
                if (!header) {
                    panic(AvmPanicWhy::AttemptToAllocateZeroElements);
                    return;
                }
                m_stack.push(Value::reference(header));
            } break;
            case Op::StoreMemUniformFloat: {
                Float value = m_stack.pop().floating();
                Uint index = m_stack.pop().uinteger();
                Ref header = m_stack.pop().reference();
                auto* storage = m_mem.get_uniform<Float>(header, index);
                if (!storage) {
                    panic(AvmPanicWhy::OutOfBoundsMemAccess);
                    return;
                }
                *storage = value;
            } break;
            case Op::LoadMemUniformFloat: {
                Uint index = m_stack.pop().uinteger();
                Ref header = m_stack.pop().reference();
                auto* storage = m_mem.get_uniform<Float>(header, index);
                if (!storage) {
                    panic(AvmPanicWhy::OutOfBoundsMemAccess);
                    return;
                }
                Float res = *storage;
                m_stack.push(Value::floating(res));
            } break;

            case Op::MakeMemUniformBool: {
                Uint count = m_stack.pop().uinteger();
                Ref header = m_mem.make_uniform<Bool>(count);
                if (!header) {
                    panic(AvmPanicWhy::AttemptToAllocateZeroElements);
                    return;
                }
                m_stack.push(Value::reference(header));
            } break;
            case Op::StoreMemUniformBool: {
                Bool value = m_stack.pop().boolean();
                Uint index = m_stack.pop().uinteger();
                Ref header = m_stack.pop().reference();
                auto* storage = m_mem.get_uniform<Bool>(header, index);
                if (!storage) {
                    panic(AvmPanicWhy::OutOfBoundsMemAccess);
                    return;
                }
                *storage = value;
            } break;
            case Op::LoadMemUniformBool: {
                Uint index = m_stack.pop().uinteger();
                Ref header = m_stack.pop().reference();
                auto* storage = m_mem.get_uniform<Bool>(header, index);
                if (!storage) {
                    panic(AvmPanicWhy::OutOfBoundsMemAccess);
                    return;
                }
                Bool res = *storage;
                m_stack.push(Value::boolean(res));
            } break;

            case Op::MakeMemUniformStr: {
                Uint count = m_stack.pop().uinteger();
                Ref header = m_mem.make_uniform<Str>(count);
                if (!header) {
                    panic(AvmPanicWhy::AttemptToAllocateZeroElements);
                    return;
                }
                m_stack.push(Value::reference(header));
            } break;
            case Op::StoreMemUniformStr: {
                Str value = m_stack.pop().string();
                Uint index = m_stack.pop().uinteger();
                Ref header = m_stack.pop().reference();
                auto* storage = m_mem.get_uniform<Str>(header, index);
                if (!storage) {
                    panic(AvmPanicWhy::OutOfBoundsMemAccess);
                    return;
                }
                *storage = value;
            } break;
            case Op::LoadMemUniformStr: {
                Uint index = m_stack.pop().uinteger();
                Ref header = m_stack.pop().reference();
                auto* storage = m_mem.get_uniform<Str>(header, index);
                if (!storage) {
                    panic(AvmPanicWhy::OutOfBoundsMemAccess);
                    return;
                }
                Str res = *storage;
                m_stack.push(Value::string(res));
            } break;

            case Op::MakeMemUniformRef: {
                Uint count = m_stack.pop().uinteger();
                Ref header = m_mem.make_uniform<Ref>(count);
                if (!header) {
                    panic(AvmPanicWhy::AttemptToAllocateZeroElements);
                    return;
                }
                m_stack.push(Value::reference(header));
            } break;
            case Op::StoreMemUniformRef: {
                Ref value = m_stack.pop().reference();
                Uint index = m_stack.pop().uinteger();
                Ref header = m_stack.pop().reference();
                auto* storage = m_mem.get_uniform<Ref>(header, index);
                if (!storage) {
                    panic(AvmPanicWhy::OutOfBoundsMemAccess);
                    return;
                }
                *storage = value;
            } break;
            case Op::LoadMemUniformRef: {
                Uint index = m_stack.pop().uinteger();
                Ref header = m_stack.pop().reference();
                auto* storage = m_mem.get_uniform<Ref>(header, index);
                if (!storage) {
                    panic(AvmPanicWhy::OutOfBoundsMemAccess);
                    return;
                }
                Ref res = *storage;
                m_stack.push(Value::reference(res));
            } break;

            case Op::Copy: {
                Uint addr = inst.a.uinteger();
                m_stack.push(m_stack[addr]);
            } break;
            case Op::Swap: {
                Uint addr = inst.a.uinteger();
                Value temp = m_stack[0];
                m_stack[0] = m_stack[addr];
                m_stack[addr] = temp;
            } break;

            case Op::PrintInt: {
                Int val = m_stack.pop().integer();
                std::cout << val << std::endl;
            } break;
            case Op::PrintUint: {
                Uint val = m_stack.pop().uinteger();
                std::cout << val << std::endl;
            } break;
            case Op::PrintByte: {
                Byte val = m_stack.pop().byte();
                std::cout << Uint(val) << std::endl;
            } break;
            case Op::PrintFloat: {
                Float val = m_stack.pop().floating();
                std::cout << val << std::endl;
            } break;
            case Op::PrintBool: {
                Bool val = m_stack.pop().boolean();
                std::cout << (val ? "true" : "false") << std::endl;
            } break;
            case Op::PrintStr: {
                StrHeader* val = m_stack.pop().string();
                std::cout << "\""
                          << std::string_view(val->data(), val->size)
                          << "\""
                          << std::endl;
            } break;
            case Op::PrintRef: {
                MemHeader* val = m_stack.pop().reference();
                std::cout << "0x" 
                          << std::hex 
                          << std::uintptr_t(val) 
                          << std::dec 
                          << std::endl;
            } break;

            case Op::AddInt: {
                Int b = m_stack.pop().integer();
                Int a = m_stack.pop().integer();
                m_stack.push(Value::integer(wrap_add<Int>(a, b)));
            } break;
            case Op::SubInt: {
                Int b = m_stack.pop().integer();
                Int a = m_stack.pop().integer();
                m_stack.push(Value::integer(wrap_sub<Int>(a, b)));
            } break;
            case Op::MulInt: {
                Int b = m_stack.pop().integer();
                Int a = m_stack.pop().integer();
                m_stack.push(Value::integer(wrap_mul<Int>(a, b)));
            } break;
            case Op::DivInt: {
                Int b = m_stack.pop().integer();
                Int a = m_stack.pop().integer();
                if (b == 0) {
                    panic(AvmPanicWhy::DivisionByZero);
                    return;
                }
                m_stack.push(Value::integer(wrap_div<Int>(a, b)));
            } break;
            case Op::UnMinInt: {
                Int a = m_stack.pop().integer();
                m_stack.push(Value::integer(-a));
            } break;
            case Op::ModInt: {
                Int b = m_stack.pop().integer();
                Int a = m_stack.pop().integer();
                m_stack.push(Value::integer(a % b));
            } break;
            case Op::EqInt: {
                Int b = m_stack.pop().integer();
                Int a = m_stack.pop().integer();
                m_stack.push(Value::boolean(a == b));
            } break;
            case Op::NeqInt: {
                Int b = m_stack.pop().integer();
                Int a = m_stack.pop().integer();
                m_stack.push(Value::boolean(a != b));
            } break;
            case Op::LtInt: {
                Int b = m_stack.pop().integer();
                Int a = m_stack.pop().integer();
                m_stack.push(Value::boolean(a < b));
            } break;
            case Op::GtInt: {
                Int b = m_stack.pop().integer();
                Int a = m_stack.pop().integer();
                m_stack.push(Value::boolean(a > b));
            } break;
            case Op::LtEqInt: {
                Int b = m_stack.pop().integer();
                Int a = m_stack.pop().integer();
                m_stack.push(Value::boolean(a <= b));
            } break;
            case Op::GtEqInt: {
                Int b = m_stack.pop().integer();
                Int a = m_stack.pop().integer();
                m_stack.push(Value::boolean(a >= b));
            } break;
            case Op::BinAndInt: {
                Int b = m_stack.pop().integer();
                Int a = m_stack.pop().integer();
                m_stack.push(Value::integer(a & b));
            } break;
            case Op::BinOrInt: {
                Int b = m_stack.pop().integer();
                Int a = m_stack.pop().integer();
                m_stack.push(Value::integer(a | b));
            } break;
            case Op::BinXorInt: {
                Int b = m_stack.pop().integer();
                Int a = m_stack.pop().integer();
                m_stack.push(Value::integer(a ^ b));
            } break;
            case Op::BinNotInt: {
                Int val = m_stack.pop().integer();
                m_stack.push(Value::integer(~val));
            } break;
            case Op::LShiftInt: {
                Uint b = m_stack.pop().uinteger();
                Int a = m_stack.pop().integer();
                m_stack.push(Value::integer(wrap_lshift(a, b)));
            } break;
            case Op::RShiftInt: {
                Uint b = m_stack.pop().uinteger();
                Int a = m_stack.pop().integer();
                m_stack.push(Value::integer(wrap_rshift(a, b)));
            } break;
            case Op::RShiftArithmInt: {
                Uint b = m_stack.pop().uinteger();
                Int a = m_stack.pop().integer();
                m_stack.push(Value::integer(wrap_rshift_arithm(a, b)));
            } break;

            case Op::AddUint: {
                Uint b = m_stack.pop().uinteger();
                Uint a = m_stack.pop().uinteger();
                m_stack.push(Value::uinteger(a + b));
            } break;
            case Op::SubUint: {
                Uint b = m_stack.pop().uinteger();
                Uint a = m_stack.pop().uinteger();
                m_stack.push(Value::uinteger(a - b));
            } break;
            case Op::MulUint: {
                Uint b = m_stack.pop().uinteger();
                Uint a = m_stack.pop().uinteger();
                m_stack.push(Value::uinteger(a * b));
            } break;
            case Op::DivUint: {
                Uint b = m_stack.pop().uinteger();
                Uint a = m_stack.pop().uinteger();
                m_stack.push(Value::uinteger(a / b));
            } break;
            case Op::ModUint: {
                Uint b = m_stack.pop().uinteger();
                Uint a = m_stack.pop().uinteger();
                m_stack.push(Value::uinteger(a % b));
            } break;
            case Op::EqUint: {
                Uint b = m_stack.pop().uinteger();
                Uint a = m_stack.pop().uinteger();
                m_stack.push(Value::boolean(a == b));
            } break;
            case Op::NeqUint: {
                Uint b = m_stack.pop().uinteger();
                Uint a = m_stack.pop().uinteger();
                m_stack.push(Value::boolean(a != b));
            } break;
            case Op::LtUint: {
                Uint b = m_stack.pop().uinteger();
                Uint a = m_stack.pop().uinteger();
                m_stack.push(Value::boolean(a < b));
            } break;
            case Op::GtUint: {
                Uint b = m_stack.pop().uinteger();
                Uint a = m_stack.pop().uinteger();
                m_stack.push(Value::boolean(a > b));
            } break;
            case Op::LtEqUint: {
                Uint b = m_stack.pop().uinteger();
                Uint a = m_stack.pop().uinteger();
                m_stack.push(Value::boolean(a <= b));
            } break;
            case Op::GtEqUint: {
                Uint b = m_stack.pop().uinteger();
                Uint a = m_stack.pop().uinteger();
                m_stack.push(Value::boolean(a >= b));
            } break;
            case Op::BinAndUint: {
                Uint b = m_stack.pop().uinteger();
                Uint a = m_stack.pop().uinteger();
                m_stack.push(Value::uinteger(a & b));
            } break;
            case Op::BinOrUint: {
                Uint b = m_stack.pop().uinteger();
                Uint a = m_stack.pop().uinteger();
                m_stack.push(Value::uinteger(a | b));
            } break;
            case Op::BinXorUint: {
                Uint b = m_stack.pop().uinteger();
                Uint a = m_stack.pop().uinteger();
                m_stack.push(Value::uinteger(a ^ b));
            } break;
            case Op::BinNotUint: {
                Uint val = m_stack.pop().uinteger();
                m_stack.push(Value::uinteger(~val));
            } break;
            case Op::LShiftUint: {
                Uint b = m_stack.pop().uinteger();
                Uint a = m_stack.pop().uinteger();
                m_stack.push(Value::uinteger(wrap_lshift<Uint>(a, b)));
            } break;
            case Op::RShiftUint: {
                Uint b = m_stack.pop().uinteger();
                Uint a = m_stack.pop().uinteger();
                m_stack.push(Value::uinteger(wrap_rshift<Uint>(a, b)));
            } break;

            case Op::AddByte: {
                Byte b = m_stack.pop().byte();
                Byte a = m_stack.pop().byte();
                m_stack.push(Value::byte(a + b));
            } break;
            case Op::SubByte: {
                Byte b = m_stack.pop().byte();
                Byte a = m_stack.pop().byte();
                m_stack.push(Value::byte(a - b));
            } break;
            case Op::MulByte: {
                Byte b = m_stack.pop().byte();
                Byte a = m_stack.pop().byte();
                m_stack.push(Value::byte(a * b));
            } break;
            case Op::DivByte: {
                Byte b = m_stack.pop().byte();
                Byte a = m_stack.pop().byte();
                m_stack.push(Value::byte(a / b));
            } break;
            case Op::ModByte: {
                Byte b = m_stack.pop().byte();
                Byte a = m_stack.pop().byte();
                m_stack.push(Value::byte(a % b));
            } break;
            case Op::EqByte: {
                Byte b = m_stack.pop().byte();
                Byte a = m_stack.pop().byte();
                m_stack.push(Value::boolean(a == b));
            } break;
            case Op::NeqByte: {
                Byte b = m_stack.pop().byte();
                Byte a = m_stack.pop().byte();
                m_stack.push(Value::boolean(a != b));
            } break;
            case Op::LtByte: {
                Byte b = m_stack.pop().byte();
                Byte a = m_stack.pop().byte();
                m_stack.push(Value::boolean(a < b));
            } break;
            case Op::GtByte: {
                Byte b = m_stack.pop().byte();
                Byte a = m_stack.pop().byte();
                m_stack.push(Value::boolean(a > b));
            } break;
            case Op::LtEqByte: {
                Byte b = m_stack.pop().byte();
                Byte a = m_stack.pop().byte();
                m_stack.push(Value::boolean(a <= b));
            } break;
            case Op::GtEqByte: {
                Byte b = m_stack.pop().byte();
                Byte a = m_stack.pop().byte();
                m_stack.push(Value::boolean(a >= b));
            } break;
            case Op::BinAndByte: {
                Byte b = m_stack.pop().byte();
                Byte a = m_stack.pop().byte();
                m_stack.push(Value::byte(a & b));
            } break;
            case Op::BinOrByte: {
                Byte b = m_stack.pop().byte();
                Byte a = m_stack.pop().byte();
                m_stack.push(Value::byte(a | b));
            } break;
            case Op::BinXorByte: {
                Byte b = m_stack.pop().byte();
                Byte a = m_stack.pop().byte();
                m_stack.push(Value::byte(a ^ b));
            } break;
            case Op::BinNotByte: {
                Byte val = m_stack.pop().byte();
                m_stack.push(Value::byte(~val));
            } break;
            case Op::LShiftByte: {
                Uint b = m_stack.pop().uinteger();
                Byte a = m_stack.pop().byte();
                m_stack.push(Value::byte(wrap_lshift<Byte>(a, b)));
            } break;
            case Op::RShiftByte: {
                Uint b = m_stack.pop().uinteger();
                Byte a = m_stack.pop().byte();
                m_stack.push(Value::byte(wrap_rshift<Byte>(a, b)));
            } break;

            case Op::AddFloat: {
                Float b = m_stack.pop().floating();
                Float a = m_stack.pop().floating();
                m_stack.push(Value::floating(a + b));
            } break;
            case Op::SubFloat: {
                Float b = m_stack.pop().floating();
                Float a = m_stack.pop().floating();
                m_stack.push(Value::floating(a - b));
            } break;
            case Op::MulFloat: {
                Float b = m_stack.pop().floating();
                Float a = m_stack.pop().floating();
                m_stack.push(Value::floating(a * b));
            } break;
            case Op::DivFloat: {
                Float b = m_stack.pop().floating();
                Float a = m_stack.pop().floating();
                m_stack.push(Value::floating(a / b));
            } break;
            case Op::UnMinFloat: {
                Float a = m_stack.pop().floating();
                m_stack.push(Value::floating(-a));
            } break;
            case Op::EqFloat: {
                Float b = m_stack.pop().floating();
                Float a = m_stack.pop().floating();
                m_stack.push(Value::boolean(a == b));
            } break;
            case Op::NeqFloat: {
                Float b = m_stack.pop().floating();
                Float a = m_stack.pop().floating();
                m_stack.push(Value::boolean(a != b));
            } break;
            case Op::LtFloat: {
                Float b = m_stack.pop().floating();
                Float a = m_stack.pop().floating();
                m_stack.push(Value::boolean(a < b));
            } break;
            case Op::GtFloat: {
                Float b = m_stack.pop().floating();
                Float a = m_stack.pop().floating();
                m_stack.push(Value::boolean(a > b));
            } break;
            case Op::LtEqFloat: {
                Float b = m_stack.pop().floating();
                Float a = m_stack.pop().floating();
                m_stack.push(Value::boolean(a <= b));
            } break;
            case Op::GtEqFloat: {
                Float b = m_stack.pop().floating();
                Float a = m_stack.pop().floating();
                m_stack.push(Value::boolean(a >= b));
            } break;

            case Op::AndBool: {
                Bool b = m_stack.pop().boolean();
                Bool a = m_stack.pop().boolean();
                m_stack.push(Value::boolean(a && b));
            } break;
            case Op::OrBool: {
                Bool b = m_stack.pop().boolean();
                Bool a = m_stack.pop().boolean();
                m_stack.push(Value::boolean(a || b));
            } break;
            case Op::XorBool: {
                Bool b = m_stack.pop().boolean();
                Bool a = m_stack.pop().boolean();
                m_stack.push(Value::boolean(a ^ b));
            } break;
            case Op::NotBool: {
                Bool val = m_stack.pop().boolean();
                m_stack.push(Value::boolean(!val));
            } break;
            case Op::EqBool: {
                Bool b = m_stack.pop().boolean();
                Bool a = m_stack.pop().boolean();
                m_stack.push(Value::boolean(a == b));
            } break;
            case Op::NeqBool: {
                Bool b = m_stack.pop().boolean();
                Bool a = m_stack.pop().boolean();
                m_stack.push(Value::boolean(a != b));
            } break;

            case Op::EqStr: {
                StrHeader* b = m_stack.pop().string();
                StrHeader* a = m_stack.pop().string();
                m_stack.push(Value::boolean(a == b));
            } break;

            case Op::CIntUint: {
                Int a = m_stack.pop().integer();
                m_stack.push(Value::uinteger(Uint(a)));
            } break;
            case Op::CIntFloat: {
                Int a = m_stack.pop().integer();
                m_stack.push(Value::floating(Float(a)));
            } break;
            case Op::CIntByte: {
                Int a = m_stack.pop().integer();
                m_stack.push(Value::byte(Byte(a)));
            } break;
            case Op::CUintInt: {
                Uint a = m_stack.pop().uinteger();
                m_stack.push(Value::integer(Int(a)));
            } break;
            case Op::CUintFloat: {
                Uint a = m_stack.pop().uinteger();
                m_stack.push(Value::floating(Float(a)));
            } break;
            case Op::CUintByte: {
                Uint a = m_stack.pop().uinteger();
                m_stack.push(Value::byte(Byte(a)));
            } break;
            case Op::CFloatInt: {
                Float a = m_stack.pop().floating();
                m_stack.push(Value::integer(Int(a)));
            } break;
            case Op::CFloatUint: {
                Float a = m_stack.pop().floating();
                m_stack.push(Value::uinteger(Uint(a)));
            } break;
            case Op::CFloatByte: {
                Float a = m_stack.pop().floating();
                m_stack.push(Value::byte(Byte(a)));
            } break;
            case Op::CByteInt: {
                Byte a = m_stack.pop().byte();
                m_stack.push(Value::integer(Int(a)));
            } break;
            case Op::CByteUint: {
                Byte a = m_stack.pop().byte();
                m_stack.push(Value::uinteger(Uint(a)));
            } break;
            case Op::CByteFloat: {
                Byte a = m_stack.pop().byte();
                m_stack.push(Value::floating(Float(a)));
            } break;

            case Op::If: {
                Uint true_addr = inst.a.uinteger();
                Uint false_addr = inst.b.uinteger();
                Bool cond = m_stack.pop().boolean();
                m_program_cnt = cond ? true_addr : false_addr;
                return;
            } break;
            case Op::JumpIf: {
                Uint addr = inst.a.uinteger();
                Bool cond = m_stack.pop().boolean();
                if (cond) {
                    m_program_cnt = addr;
                    return;
                }
            } break;
            case Op::Call: {
                Uint addr = m_stack.pop().uinteger();
                m_ret_stack.push(ReturnFrame::addr(m_program_cnt + 1));
                m_program_cnt = addr;
                m_local_stack.push();
                return;
            } break;
            case Op::CallIm: {
                Uint addr = inst.a.uinteger();
                m_ret_stack.push(ReturnFrame::addr(m_program_cnt + 1));
                m_program_cnt = addr;
                m_local_stack.push();
                return;
            } break;
            case Op::CallExtern: {
                Uint slot = m_stack.pop().uinteger();
                const auto& extern_ = m_externs[slot];
                extern_.func(*this, extern_.user_data);
            } break;
            case Op::CallExternIm: {
                Uint slot = inst.a.uinteger();
                auto& extern_ = m_externs[slot];
                extern_.func(*this, extern_.user_data);
            } break;
            case Op::Ret: {
                ReturnFrame frame = m_ret_stack.pop();
                m_local_stack.pop();
                if (frame.is_exit()) {
                    m_exit_requested = true;
                    return;
                }
                m_program_cnt = frame.addr();
                return;
            } break;
            case Op::Goto: {
                std::size_t addr = m_stack.pop().uinteger();
                m_program_cnt = addr;
                return;
            } break;
            case Op::GotoIm: {
                std::size_t addr = inst.a.uinteger();
                m_program_cnt = addr;
                return;
            } break;

            case Op::CallIntrin: {
                Uint intrin_num = inst.a.uinteger();
                Intrin intrin = static_cast<Intrin>(intrin_num);
                call_intrin(intrin);
            } break;

            case Op::Panic: {
                panic(AvmPanicWhy::ExplicitPanic);
                return;
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
                Int b = m_stack.pop().integer();
                Int a = m_stack.pop().integer();
                m_stack.push(Value::integer(std::min(a, b)));
            } break;
            case Intrin::MaxInt: {
                Int b = m_stack.pop().integer();
                Int a = m_stack.pop().integer();
                m_stack.push(Value::integer(std::max(a, b)));
            } break;
            case Intrin::SignInt: {
                Int a = m_stack.pop().integer();
                Int sign = (a > 0) ? 1 : ((a < 0) ? -1 : 0);
                m_stack.push(Value::integer(sign));
            } break;
            case Intrin::AbsInt: {
                Int a = m_stack.pop().integer();
                m_stack.push(Value::integer(std::abs(a)));
            } break;

            case Intrin::MinUint: {
                Uint b = m_stack.pop().uinteger();
                Uint a = m_stack.pop().uinteger();
                m_stack.push(Value::uinteger(std::min(a, b)));
            } break;
            case Intrin::MaxUint: {
                Uint b = m_stack.pop().uinteger();
                Uint a = m_stack.pop().uinteger();
                m_stack.push(Value::uinteger(std::max(a, b)));
            } break;

            case Intrin::FloorFloat: {
                Float a = m_stack.pop().floating();
                m_stack.push(Value::floating(std::floor(a)));
            } break;
            case Intrin::CeilFloat: {
                Float a = m_stack.pop().floating();
                m_stack.push(Value::floating(std::ceil(a)));
            } break;
            case Intrin::RoundFloat: {
                Float a = m_stack.pop().floating();
                m_stack.push(Value::floating(std::round(a)));
            } break;
            case Intrin::TruncFloat: {
                Float a = m_stack.pop().floating();
                m_stack.push(Value::floating(std::trunc(a)));
            } break;
            case Intrin::IsNanFloat: {
                Float a = m_stack.pop().floating();
                m_stack.push(Value::boolean(std::isnan(a)));
            } break;
            case Intrin::IsInfFloat: {
                Float a = m_stack.pop().floating();
                m_stack.push(Value::boolean(std::isinf(a)));
            } break;
            case Intrin::IsFinFloat: {
                Float a = m_stack.pop().floating();
                m_stack.push(Value::boolean(std::isfinite(a)));
            } break;
            case Intrin::SignBitFloat: {
                Float a = m_stack.pop().floating();
                m_stack.push(Value::boolean(std::signbit(a)));
            } break;
            case Intrin::FmaFloat: {
                Float c = m_stack.pop().floating();
                Float b = m_stack.pop().floating();
                Float a = m_stack.pop().floating();
                m_stack.push(Value::floating(std::fma(a, b, c)));
            } break;
            case Intrin::MinFloat: {
                Float b = m_stack.pop().floating();
                Float a = m_stack.pop().floating();
                m_stack.push(Value::floating(std::min(a, b)));
            } break;
            case Intrin::MaxFloat: {
                Float b = m_stack.pop().floating();
                Float a = m_stack.pop().floating();
                m_stack.push(Value::floating(std::max(a, b)));
            } break;
            case Intrin::SqrtFloat: {
                Float a = m_stack.pop().floating();
                m_stack.push(Value::floating(std::sqrt(a)));
            } break;
            case Intrin::SinFloat: {
                Float a = m_stack.pop().floating();
                m_stack.push(Value::floating(std::sin(a)));
            } break;
            case Intrin::CosFloat: {
                Float a = m_stack.pop().floating();
                m_stack.push(Value::floating(std::cos(a)));
            } break;
            case Intrin::TanFloat: {
                Float a = m_stack.pop().floating();
                m_stack.push(Value::floating(std::tan(a)));
            } break;
            case Intrin::ExpFloat: {
                Float a = m_stack.pop().floating();
                m_stack.push(Value::floating(std::exp(a)));
            } break;
            case Intrin::Exp2Float: {
                Float a = m_stack.pop().floating();
                m_stack.push(Value::floating(std::exp2(a)));
            } break;
            case Intrin::LogFloat: {
                Float a = m_stack.pop().floating();
                m_stack.push(Value::floating(std::log(a)));
            } break;
            case Intrin::Log2Float: {
                Float a = m_stack.pop().floating();
                m_stack.push(Value::floating(std::log2(a)));
            } break;
            case Intrin::Log10Float: {
                Float a = m_stack.pop().floating();
                m_stack.push(Value::floating(std::log10(a)));
            } break;
            case Intrin::PowFloat: {
                Float b = m_stack.pop().floating();
                Float a = m_stack.pop().floating();
                m_stack.push(Value::floating(std::pow(a, b)));
            } break;
            case Intrin::SignFloat: {
                Float a = m_stack.pop().floating();
                Int sign = (a > 0.0) ? 1 : ((a < 0.0) ? -1 : 0);
                m_stack.push(Value::integer(sign));
            } break;
            case Intrin::AbsFloat: {
                Float a = m_stack.pop().floating();
                m_stack.push(Value::floating(std::abs(a)));
            } break;

            case Intrin::LenStr: {
                StrHeader* a = m_stack.pop().string();
                m_stack.push(Value::uinteger(a->size));
            } break;
            case Intrin::ConcatStr: {
                StrHeader* b = m_stack.pop().string();
                StrHeader* a = m_stack.pop().string();
                char* buf = new char[a->size + b->size];
                memcpy(buf, a->data(), a->size);
                memcpy(buf + a->size, b->data(), b->size);
                StrKey key(buf, a->size + b->size);
                StrHeader* res = m_mem.str_intern(key);
                delete[] buf;
                m_stack.push(Value::string(res));
            } break;
            case Intrin::SubStr: {
                Uint b = m_stack.pop().uinteger();
                Uint a = m_stack.pop().uinteger();
                StrHeader* str = m_stack.pop().string();
                std::size_t size = b - a;
                char* buf = new char[size];
                memcpy(buf, str->data() + a, size);
                StrKey key(buf, size);
                StrHeader* res = m_mem.str_intern(key);
                delete[] buf;
                m_stack.push(Value::string(res));
            } break;
            }
        }

        inline void stack_push(const Value& value) {
            m_stack.push(value);
        }

        inline Value stack_pop() {
            return m_stack.pop();
        }

        AvmResult call(std::uint64_t addr) {
            m_exit_requested = false;
            m_program_cnt = addr;
            m_ret_stack.push(ReturnFrame::exit(m_stack.size()));
            m_local_stack.push();
            while (!m_exit_requested) {
                execute_inst(m_program[m_program_cnt]);
            }
            if (m_panic) {
                auto result = AvmResult::panic(*m_panic);
                m_panic.reset();
                return result;
            }
            return AvmResult::ok();
        }

    private:
        template<typename T>
        static inline T wrap_add(T a, T b) {
            using U = std::make_unsigned_t<T>;
            U ua = static_cast<U>(a);
            U ub = static_cast<U>(b);
            U ures = ua + ub;
            return static_cast<T>(ures);
        }

        template<typename T>
        static inline T wrap_sub(T a, T b) {
            using U = std::make_unsigned_t<T>;
            U ua = static_cast<U>(a);
            U ub = static_cast<U>(b);
            U ures = ua - ub;
            return static_cast<T>(ures);
        }

        template<typename T>
        static inline T wrap_mul(T a, T b) {
            using U = std::make_unsigned_t<T>;
            U ua = static_cast<U>(a);
            U ub = static_cast<U>(b);
            U ures = ua * ub;
            return static_cast<T>(ures);
        }

        template<typename T>
        static inline T wrap_div(T a, T b) {
            using U = std::make_unsigned_t<T>;
            U ua = static_cast<U>(a);
            U ub = static_cast<U>(b);
            U ures = ua / ub;
            return static_cast<T>(ures);
        }

        template<typename T>
        static inline T wrap_lshift(T a, Uint b)
        {
            using U = std::make_unsigned_t<T>;
            constexpr Uint N = sizeof(T) * 8;
            b %= N;
            U ua = static_cast<U>(a);
            U ur = static_cast<U>(ua << b);
            return static_cast<T>(ur);
        }

        template<typename T>
        static inline T wrap_rshift(T a, Uint b)
        {
            using U = std::make_unsigned_t<T>;
            constexpr Uint N = sizeof(T) * 8;
            b %= N;
            return static_cast<T>(static_cast<U>(a) >> b);
        }

        template<typename T>
        static inline T wrap_rshift_arithm(T a, Uint b)
        {
            constexpr Uint N = sizeof(T) * 8;
            b %= N;
            if (a > 0) {
                return static_cast<T>((a >> b) | ~(~T(0) >> b));
            } else {
                return static_cast<T>(a >> b);
            }
        }

        void panic(AvmPanicWhy why) {
            m_exit_requested = true;
            auto stack_trace = panic_unwind();
            m_panic = std::make_optional(AvmPanic(why, stack_trace));
        }

        std::vector<std::size_t> panic_unwind() {
            std::vector<std::size_t> stack_trace;
            stack_trace.reserve(m_ret_stack.size() + 1);
            stack_trace.push_back(m_program_cnt);
            
            while (true) {
                auto frame = m_ret_stack.pop();
                m_local_stack.pop();
                if (frame.is_exit()) {
                    std::size_t height = frame.stack_height();
                    while (m_stack.size() != height) {
                        m_stack.pop();
                    }
                    break;
                } else {
                    // subtract one because return frame contains address of 
                    // call instruction + 1
                    stack_trace.push_back(frame.addr() - 1);
                }
            }

            return stack_trace;
        }

        class ReturnFrame {
        public:
            static inline ReturnFrame addr(std::size_t addr) {
                return ReturnFrame(addr, 0, false);
            }

            static inline ReturnFrame exit(std::size_t stack_height) {
                return ReturnFrame(0, stack_height, true);
            }

            inline std::size_t addr() {
                return m_addr;
            }

            inline std::size_t stack_height() {
                return m_stack_height;
            }

            inline bool is_exit() {
                return m_is_exit;
            }

        private:
            std::size_t m_addr;
            std::size_t m_stack_height;
            bool m_is_exit;

            inline ReturnFrame(
                std::size_t addr, 
                std::size_t stack_height,
                bool is_exit
            ) : m_addr(addr),
                m_stack_height(stack_height),
                m_is_exit(is_exit) { }
        };

        std::vector<Inst> m_program;
        std::size_t m_program_cnt;
        Stack<Value> m_stack;
        Stack<ReturnFrame> m_ret_stack;
        LocalStack m_local_stack;
        Mem m_mem;
        bool m_exit_requested;
        Externs& m_externs;
        StringStore& m_string_store;
        std::optional<AvmPanic> m_panic;
    };
    
}

#endif // AVM_AVM_H_