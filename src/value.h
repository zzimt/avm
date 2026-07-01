#ifndef AVM_VALUE_H_
#define AVM_VALUE_H_

#include <cstdint>

namespace avm {

    enum class Type : std::uint8_t {
        Int,
        Uint,
        Float,
        Bool,
        Str,
        Ref,
    };

    struct StrHeader;

    struct MemHeader;

    class Value {
    public:
        union Data {
            std::int64_t integer;
            std::uint64_t uinteger;
            double floating;
            std::uint8_t boolean;
            StrHeader* string;
            MemHeader* reference;
        };

        static inline Value integer(std::int64_t value) {
            Data data;
            data.integer = value;
            return Value(Type::Int, data);
        }

        inline std::int64_t integer() const {
            return m_data.integer;
        }

        static inline Value uinteger(std::uint64_t value) {
            Data data;
            data.uinteger = value;
            return Value(Type::Uint, data);
        }

        inline std::uint64_t uinteger() const {
            return m_data.uinteger;
        }

        static inline Value floating(double value) {
            Data data;
            data.floating = value;
            return Value(Type::Float, data);
        }

        inline float floating() const {
            return m_data.floating;
        }

        static inline Value boolean(std::uint8_t value) {
            Data data;
            data.boolean = value;
            return Value(Type::Bool, data);
        }

        inline std::uint8_t boolean() const {
            return m_data.boolean;
        }

        static inline Value string(StrHeader* value) {
            Data data;
            data.string = value;
            return Value(Type::Str, data);
        }

        inline StrHeader* string() {
            return m_data.string;
        }

        static inline Value reference(MemHeader* value) {
            Data data;
            data.reference = value;
            return Value(Type::Ref, data);
        }

        inline MemHeader* reference() const {
            return m_data.reference;
        }

        inline Type type() const {
            return m_type;
        }
        
        inline Value() :
            m_type(Type::Int),
            m_data({ 0 }) { }

        inline Value(Type type, Data data) :
            m_type(type),
            m_data(data) { }

    private:
        Type m_type;
        Data m_data;
    };
}

#endif // AVM_VALUE_H_