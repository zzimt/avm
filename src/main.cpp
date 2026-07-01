#include <iostream>
#include <cstdint>

#include "resolver.h"
#include "avm.h"
#include "value.h"
#include "externs.h"
#include "stringstore.h"

static void test_extern(avm::Avm& avm, [[maybe_unused]] void* user_data) {
    avm::Value value = avm.stack_pop();
    std::cout << "Hello from `test_extern`, the value is "
              << value.floating()
              << std::endl;
}

int main() {
    using namespace avm;

    std::vector<Elem> elems = {
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

        Elem::label("test_extern"),
            Elem::inst({ Op::Push, Value::floating(10.0) }),
            Elem::inst({ Op::Push, Value::floating(12.1) }),
            Elem::inst({ Op::AddFloat }),
            Elem::inst({ Op::CallExternIm, Value::uinteger(0) }),
            Elem::inst({ Op::Ret }),

        Elem::label("test_strings"),
            Elem::inst({ Op::PushStr, Value::uinteger(0) }),
            Elem::inst({ Op::PrintStr }),
            Elem::inst({ Op::PushStr, Value::uinteger(0) }),
            Elem::inst({ Op::CallIntrin, Value::uinteger(
                static_cast<std::uint8_t>(Intrin::LenStr)
            )}),
            Elem::inst({ Op::PrintUint }),
            Elem::inst({ Op::Ret }),
    };

    Resolver resolver(elems);

    auto program = resolver.resolve();

    Externs externs;
    externs.register_func(0, test_extern, nullptr);

    StringStore string_store;
    string_store.add(0, "hello world!");

    Avm avm(program, externs, string_store);

    {
        std::int64_t n = 5;
        avm.stack_push(Value::integer(n));
        auto factorial_addr = *resolver.get_label_addr("factorial");
        avm.call(factorial_addr);
        Value ret = avm.stack_pop();
        std::cout << "factorial(" 
                  << n 
                  << ") = " 
                  << ret.integer() 
                  << std::endl;
    }

    {
        std::int64_t n = 12;
        avm.stack_push(Value::integer(n));
        auto fib_addr = *resolver.get_label_addr("fib");
        avm.call(fib_addr);
        Value ret = avm.stack_pop();
        std::cout << "fib("
                  << n
                  << ") = "
                  << ret.integer()
                  << std::endl;
    }

    {
        auto test_extern_addr = *resolver.get_label_addr("test_extern");
        avm.call(test_extern_addr);
    }

    {
        auto test_strings_addr = *resolver.get_label_addr("test_strings");
        avm.call(test_strings_addr);
    }

    return 0;
}