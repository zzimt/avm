#include <iostream>

#include "resolver.h"
#include "avm.h"
#include "value.h"
#include "externs.h"
#include "stringstore.h"

static void test_extern(avm::Avm& avm, [[maybe_unused]] void* user_data) {
    using namespace avm;

    Resolver* resolver = static_cast<Resolver*>(user_data);

    Value value = avm.stack_pop();
    std::cout << "Hello from `test_extern`, the value is "
              << value.floating()
              << std::endl;

    {
        Uint n = 12345600;
        auto should_be_called_from_extern_addr = 
            *resolver->get_label_addr("should_be_called_from_extern");
        avm.stack_push(Value::uinteger(n));
        avm.call(should_be_called_from_extern_addr);
    }
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

        Elem::label("should_be_called_from_extern"),
            Elem::inst({ Op::StoreLocal, Value::uinteger(0) }),
            Elem::inst({ Op::PushStr, Value::uinteger(4) }),
            Elem::inst({ Op::PrintStr }),
            Elem::inst({ Op::PushStr, Value::uinteger(5) }),
            Elem::inst({ Op::PrintStr }),
            Elem::inst({ Op::LoadLocal, Value::uinteger(0) }),
            Elem::inst({ Op::PrintUint }),
            Elem::inst({ Op::Ret }),

        Elem::label("test_strings"),
            Elem::inst({ Op::PushStr, Value::uinteger(0) }),
            Elem::inst({ Op::PrintStr }),
            Elem::inst({ Op::PushStr, Value::uinteger(0) }),
            Elem::inst({ Intrin::LenStr }),
            Elem::inst({ Op::PrintUint }),
            Elem::inst({ Op::PushStr, Value::uinteger(0) }),
            Elem::inst({ Op::PushStr, Value::uinteger(1) }),
            Elem::inst({ Intrin::ConcatStr }),
            Elem::inst({ Op::PrintStr }),
            Elem::inst({ Op::PushStr, Value::uinteger(0) }),
            Elem::inst({ Op::Push, Value::uinteger(1) }),
            Elem::inst({ Op::Push, Value::uinteger(7)}),
            Elem::inst({ Intrin::SubStr }),
            Elem::inst({ Op::PrintStr }),
            Elem::inst({ Op::PushStr, Value::uinteger(2) }),
            Elem::inst({ Op::PushStr, Value::uinteger(3) }),
            Elem::inst({ Op::EqStr }),
            Elem::inst({ Op::PrintBool }),
            Elem::inst({ Op::Ret }),

        Elem::label("test_uniforms"),
            Elem::inst({ Op::Push, Value::uinteger(10) }),
            Elem::inst({ Op::MakeMemUniformInt }),
            Elem::inst({ Op::StoreLocal, Value::uinteger(0) }),

            Elem::inst({ Op::LoadLocal, Value::uinteger(0) }), // array
            Elem::inst({ Op::Push, Value::uinteger(0) }), // [0]
            Elem::inst({ Op::Push, Value::integer(123) }), // = 123
            Elem::inst({ Op::StoreMemUniformInt }),

            Elem::inst({ Op::LoadLocal, Value::uinteger(0) }), // array
            Elem::inst({ Op::Push, Value::uinteger(1) }), // [1]
            Elem::inst({ Op::Push, Value::integer(124) }), // = 124
            Elem::inst({ Op::StoreMemUniformInt }),

            Elem::inst({ Op::LoadLocal, Value::uinteger(0) }), // array
            Elem::inst({ Op::Push, Value::uinteger(2) }), // [2]
            Elem::inst({ Op::Push, Value::integer(125) }), // = 125
            Elem::inst({ Op::StoreMemUniformInt }),

            Elem::inst({ Op::LoadLocal, Value::uinteger(0) }), // array
            Elem::inst({ Op::Push, Value::uinteger(3) }), // [3]
            Elem::inst({ Op::Push, Value::integer(126) }), // = 126
            Elem::inst({ Op::StoreMemUniformInt }),

            Elem::inst({ Op::LoadLocal, Value::uinteger(0) }), // array
            Elem::inst({ Op::Push, Value::uinteger(4) }), // [4]
            Elem::inst({ Op::Push, Value::integer(127) }), // = 127
            Elem::inst({ Op::StoreMemUniformInt }),

            Elem::inst({ Op::LoadLocal, Value::uinteger(0) }), // array
            Elem::inst({ Op::Push, Value::uinteger(5) }), // [5]
            Elem::inst({ Op::Push, Value::integer(128) }), // = 128
            Elem::inst({ Op::StoreMemUniformInt }),

            Elem::inst({ Op::LoadLocal, Value::uinteger(0) }), // array
            Elem::inst({ Op::Push, Value::uinteger(6) }), // [6]
            Elem::inst({ Op::Push, Value::integer(129) }), // = 129
            Elem::inst({ Op::StoreMemUniformInt }),

            Elem::inst({ Op::LoadLocal, Value::uinteger(0) }), // array
            Elem::inst({ Op::Push, Value::uinteger(7) }), // [7]
            Elem::inst({ Op::Push, Value::integer(130) }), // = 130
            Elem::inst({ Op::StoreMemUniformInt }),

            Elem::inst({ Op::LoadLocal, Value::uinteger(0) }), // array
            Elem::inst({ Op::Push, Value::uinteger(8) }), // [8]
            Elem::inst({ Op::Push, Value::integer(131) }), // = 131
            Elem::inst({ Op::StoreMemUniformInt }),

            Elem::inst({ Op::LoadLocal, Value::uinteger(0) }), // array
            Elem::inst({ Op::Push, Value::uinteger(9) }), // [9]
            Elem::inst({ Op::Push, Value::integer(132) }), // = 132
            Elem::inst({ Op::StoreMemUniformInt }),

            Elem::inst({ Op::LoadLocal, Value::uinteger(0) }),
            Elem::inst({ Op::Push, Value::uinteger(0) }),
            Elem::inst({ Op::LoadMemUniformInt }),
            Elem::inst({ Op::PrintInt }),

            Elem::inst({ Op::LoadLocal, Value::uinteger(0) }),
            Elem::inst({ Op::Push, Value::uinteger(1) }),
            Elem::inst({ Op::LoadMemUniformInt }),
            Elem::inst({ Op::PrintInt }),

            Elem::inst({ Op::LoadLocal, Value::uinteger(0) }),
            Elem::inst({ Op::Push, Value::uinteger(2) }),
            Elem::inst({ Op::LoadMemUniformInt }),
            Elem::inst({ Op::PrintInt }),

            Elem::inst({ Op::LoadLocal, Value::uinteger(0) }),
            Elem::inst({ Op::Push, Value::uinteger(3) }),
            Elem::inst({ Op::LoadMemUniformInt }),
            Elem::inst({ Op::PrintInt }),

            Elem::inst({ Op::LoadLocal, Value::uinteger(0) }),
            Elem::inst({ Op::Push, Value::uinteger(4) }),
            Elem::inst({ Op::LoadMemUniformInt }),
            Elem::inst({ Op::PrintInt }),

            Elem::inst({ Op::LoadLocal, Value::uinteger(0) }),
            Elem::inst({ Op::Push, Value::uinteger(5) }),
            Elem::inst({ Op::LoadMemUniformInt }),
            Elem::inst({ Op::PrintInt }),

            Elem::inst({ Op::LoadLocal, Value::uinteger(0) }),
            Elem::inst({ Op::Push, Value::uinteger(6) }),
            Elem::inst({ Op::LoadMemUniformInt }),
            Elem::inst({ Op::PrintInt }),

            Elem::inst({ Op::LoadLocal, Value::uinteger(0) }),
            Elem::inst({ Op::Push, Value::uinteger(7) }),
            Elem::inst({ Op::LoadMemUniformInt }),
            Elem::inst({ Op::PrintInt }),

            Elem::inst({ Op::LoadLocal, Value::uinteger(0) }),
            Elem::inst({ Op::Push, Value::uinteger(8) }),
            Elem::inst({ Op::LoadMemUniformInt }),
            Elem::inst({ Op::PrintInt }),

            Elem::inst({ Op::LoadLocal, Value::uinteger(0) }),
            Elem::inst({ Op::Push, Value::uinteger(9) }),
            Elem::inst({ Op::LoadMemUniformInt }),
            Elem::inst({ Op::PrintInt }),

            Elem::inst({ Op::Ret }),
    };

    Resolver resolver(elems);

    auto program = resolver.resolve();

    Externs externs;
    externs.register_func(0, test_extern, &resolver);

    StringStore string_store;
    string_store.add(0, "hello world!");
    string_store.add(1, " goodbye world!");
    string_store.add(2, "equal");
    string_store.add(3, "equal");
    string_store.add(4, "this vm function was called from extern function!");
    string_store.add(5, "the value is:");

    Avm avm(program, externs, string_store);

    std::cout << "----------------------------------------" << std::endl;
    std::cout << "-- factorial ---------------------------" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    std::cout << std::endl;
    {
        Int n = 20;
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
    std::cout << std::endl;

    std::cout << "----------------------------------------" << std::endl;
    std::cout << "-- fib ---------------------------------" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    std::cout << std::endl;
    {
        Int n = 30;
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
    std::cout << std::endl;

    std::cout << "----------------------------------------" << std::endl;
    std::cout << "-- test_extern -------------------------" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    std::cout << std::endl;
    {
        auto test_extern_addr = *resolver.get_label_addr("test_extern");
        avm.call(test_extern_addr);
    }
    std::cout << std::endl;

    std::cout << "----------------------------------------" << std::endl;
    std::cout << "-- test_strings ------------------------" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    std::cout << std::endl;
    {
        auto test_strings_addr = *resolver.get_label_addr("test_strings");
        avm.call(test_strings_addr);
    }
    std::cout << std::endl;

    std::cout << "----------------------------------------" << std::endl;
    std::cout << "-- test_uniforms -----------------------" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    std::cout << std::endl;
    {
        auto test_uniforms_addr = *resolver.get_label_addr("test_uniforms");
        avm.call(test_uniforms_addr);
    }

    return 0;
}