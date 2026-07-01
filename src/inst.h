#ifndef AVM_INST_H_
#define AVM_INST_H_

#include <cstdint>

#include "value.h"

namespace avm {

    enum class Op : std::uint8_t {
        Nop = 0,

        StoreLocal,
        LoadLocal, 
        Push,
        PushStr,

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
        PrintFloat,
        PrintBool,
        PrintStr,
        PrintRef,

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
        CallExtern,
        CallExternIm,
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

        LenStr,
        ConcatStr,
        SubStr,
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

        inline Inst(Intrin intrin) :
            op(Op::CallIntrin),
            a(Value::uinteger(static_cast<std::uint64_t>(intrin))),
            b() { }
    };

}

#endif // AVM_INST_H_