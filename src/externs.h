#ifndef AVM_EXTERNS_H_
#define AVM_EXTERNS_H_

#include <vector>
#include <cstdint>

#include "avmfwd.h"

namespace avm {

    class Externs {
    public:
        using ExternFunc = void (*)(Avm& avm, void* user_data);

        struct Extern {
            ExternFunc func;
            void* user_data;

            Extern() :
                func(nullptr),
                user_data(nullptr) { }

            Extern(ExternFunc func, void* user_data) :
                func(func),
                user_data(user_data) { }
        };

        Externs() :
            m_externs() { }

        void register_func(
            std::uint64_t slot,
            ExternFunc func,
            void* user_data
        ) {
            if (slot >= m_externs.size()) {
                m_externs.resize(slot + 1);
            }
            m_externs[slot] = Extern(func, user_data);
        }

        inline const Extern& operator[](std::uint64_t slot) const {
            return m_externs[slot];
        }

    private:
        std::vector<Extern> m_externs;
    };

}

#endif // AVM_EXTERNS_H_
