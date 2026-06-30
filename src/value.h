#ifndef AVM_VALUE_H_
#define AVM_VALUE_H_

#include <cstdint>

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
}

#endif // AVM_VALUE_H_