#pragma once

enum OpIndex { ADD = 0, MOV = 1, SUB = 5, CMP = 7, UNDEFINED = -1 };

enum OpJump {
    jo,
    jno,
    jb,     //jnae
    jnb,    //jae
    je,     //jz
    jne,    //jnz
    jbe,    //jna
    ja,     //jnbe
    js,
    jns,
    jp,     //jpe
    jnp,    //jpo
    jl,     //jnge
    jnl,    //jge
    jle,    //jng
    jnle,   //jg
};

enum OpLoop {
    loopnz, //loopne
    loopz,  //loope
    loop,
    jcxz,
};

OpIndex IsReg_Mem_Reg(u8 byte)
{
    u8 mask = 0b1111'1100;
    return (byte & mask) == 0b1000'1000 ? OpIndex::MOV
         : (byte & mask) == 0b0000'0000 ? OpIndex::ADD
         : (byte & mask) == 0b0010'1000 ? OpIndex::SUB
         : (byte & mask) == 0b0011'1000 ? OpIndex::CMP
         : OpIndex::UNDEFINED;
}

bool IsImm_Reg_Mem(u8 byte)
{
    u8 movMask    = 0b1111'1110;
    u8 othersMask = 0b1111'1100;
    return (byte & movMask)    == 0b1100'0110
        || (byte & othersMask) == 0b1000'0000;
}

OpIndex IsImm_Accumulator(u8 byte)
{
    u8 mask = 0b1111'1110;
    return (byte & mask) == 0b0000'0100 ? OpIndex::ADD
         : (byte & mask) == 0b0010'1100 ? OpIndex::SUB
         : (byte & mask) == 0b0011'1100 ? OpIndex::CMP
         : OpIndex::UNDEFINED;
}

bool IsJump(u8 byte)
{
    u8 mask = 0b1111'0000;
    return (byte & mask) == 0b0111'0000;
}

bool IsLoop(u8 byte)
{
    u8 mask = 0b1111'0000;
    return (byte & mask) == 0b1110'0000;
}