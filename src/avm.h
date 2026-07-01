#ifndef AVM_AVM_H_
#define AVM_AVM_H_

#include <cstddef>
#include <vector>
#include <iostream>
#include <cmath>
#include <cstdint>

#include "inst.h"
#include "stack.h"
#include "localstack.h"
#include "mem.h"

namespace avm {

    class Avm {
    public:
        Avm(const std::vector<Inst>& program) :
            m_program(program),
            m_program_cnt(0),
            m_stack(),
            m_ret_stack(),
            m_local_stack(),
            m_mem(m_stack, m_local_stack),
            m_exit_requested(false),
            m_registered_externs(),
            m_string_storage(),
            m_strings() { 
            m_stack.reserve(1024);
            m_ret_stack.reserve(1024);
            m_local_stack.reserve(1024);
            m_string_storage.reserve(1024);
            m_strings.reserve(256);
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
            case Op::PushStr: {
                std::uint64_t id = inst.a.uinteger();
                StrHeader* header = m_mem.str_intern(m_strings[id]);
                m_stack.push(Value::string(header));
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
            case Op::PrintFloat: {
                double val = m_stack.pop().floating();
                std::cout << val << std::endl;
            } break;
            case Op::PrintBool: {
                std::uint8_t val = m_stack.pop().boolean();
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
            case Op::CallExtern: {
                std::uint64_t slot = m_stack.pop().uinteger();
                auto& reg_extern = m_registered_externs[slot];
                reg_extern.func(*this, reg_extern.user_data);
            } break;
            case Op::CallExternIm: {
                std::uint64_t slot = inst.a.uinteger();
                auto& reg_extern = m_registered_externs[slot];
                reg_extern.func(*this, reg_extern.user_data);
            } break;
            case Op::Ret: {
                std::uint64_t addr = m_ret_stack.pop();
                m_program_cnt = addr;
                m_local_stack.pop();
                if (m_ret_stack.empty()) m_exit_requested = true;
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

            case Intrin::LenStr: {
                StrHeader* a = m_stack.pop().string();
                m_stack.push(Value::uinteger(a->size));
            } break;
            case Intrin::ConcatStr: {
                // TODO
            } break;
            case Intrin::SubStr: {
                // TODO
            } break;
            }
        }

        inline void stack_push(const Value& value) {
            m_stack.push(value);
        }

        inline Value stack_pop() {
            return m_stack.pop();
        }

        void call(std::uint64_t addr) {
            m_exit_requested = false;
            m_program_cnt = addr;
            m_ret_stack.push(0);
            m_local_stack.push();
            while (true) {
                if (m_exit_requested) return;
                execute_inst(m_program[m_program_cnt]);
            }
        }

        using ExternFunc = void (*)(Avm& avm, void* user_data);

        void register_extern(
            std::uint64_t slot, 
            ExternFunc func, 
            void* user_data
        ) {
            if (slot >= m_registered_externs.size()) {
                m_registered_externs.resize(slot + 1);
            }
            m_registered_externs[slot] = RegisteredExtern(func, user_data);
        }

        void add_string(std::uint64_t slot, std::string_view string) {
            char* ptr = m_string_storage.data() + m_string_storage.size();
            m_string_storage.insert(
                m_string_storage.end(), 
                string.begin(), 
                string.end()
            );
            StrKey key(ptr, string.size());
            if (slot >= m_strings.size()) {
                m_strings.resize(slot + 1);
            }
            m_strings[slot] = key;
        }

    private:
        struct RegisteredExtern {
            ExternFunc func;
            void* user_data;

            RegisteredExtern() :
                func(nullptr),
                user_data(nullptr) { }

            RegisteredExtern(ExternFunc func, void* user_data) :
                func(func),
                user_data(user_data) { }
        };

        std::vector<Inst> m_program;
        std::size_t m_program_cnt;
        Stack<Value> m_stack;
        Stack<std::uint64_t> m_ret_stack;
        LocalStack m_local_stack;
        Mem m_mem;
        bool m_exit_requested;
        std::vector<RegisteredExtern> m_registered_externs;
        std::vector<char> m_string_storage;
        std::vector<StrKey> m_strings;
    };
    
}

#endif // AVM_AVM_H_