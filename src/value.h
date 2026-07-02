#ifndef AVM_VALUE_H_
#define AVM_VALUE_H_

#include <cstdint>

#include "memfwd.h"

namespace avm {

    enum class Type : std::uint8_t {
        Int,
        Uint,
        Byte,
        Float,
        Bool,
        Str,
        Ref,
    };

    using Int = std::int64_t;
    using Uint = std::uint64_t;
    using Byte = std::uint8_t;
    using Float = double;
    using Bool = std::uint8_t;
    using Str = StrHeader*;
    using Ref = MemHeader*;

    class Value {
    public:
        union Data {
            Int integer;
            Uint uinteger;
            Byte byte;
            Float floating;
            Bool boolean;
            Str string;
            Ref reference;
        };

        static inline Value integer(Int value) {
            Data data;
            data.integer = value;
            return Value(Type::Int, data);
        }

        inline Int integer() const {
            return m_data.integer;
        }

        static inline Value uinteger(Uint value) {
            Data data;
            data.uinteger = value;
            return Value(Type::Uint, data);
        }

        inline Uint uinteger() const {
            return m_data.uinteger;
        }

        static inline Value byte(Byte value) {
            Data data;
            data.byte = value;
            return Value(Type::Byte, data);
        }

        inline Byte byte() {
            return m_data.byte;
        }

        static inline Value floating(Float value) {
            Data data;
            data.floating = value;
            return Value(Type::Float, data);
        }

        inline Float floating() const {
            return m_data.floating;
        }

        static inline Value boolean(Bool value) {
            Data data;
            data.boolean = value;
            return Value(Type::Bool, data);
        }

        inline Bool boolean() const {
            return m_data.boolean;
        }

        static inline Value string(Str value) {
            Data data;
            data.string = value;
            return Value(Type::Str, data);
        }

        inline Str string() {
            return m_data.string;
        }

        static inline Value reference(Ref value) {
            Data data;
            data.reference = value;
            return Value(Type::Ref, data);
        }

        inline Ref reference() const {
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