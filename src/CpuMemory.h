#pragma once

#include <cassert>

enum Flag { FLAG_ZERO, FLAG_SIGNED,   FLAG_COUNT };

enum RegisterIndex
{
    None,
    al, ah, ax,
    bl, bh, bx,
    cl, ch, cx,
    dl, dh, dx,
    sp,
    bp,
    si,
    di,
};

constexpr RegisterIndex registersMap[][2] = {
    {RegisterIndex::al, RegisterIndex::ax},
    {RegisterIndex::cl, RegisterIndex::cx},
    {RegisterIndex::dl, RegisterIndex::dx},
    {RegisterIndex::bl, RegisterIndex::bx},
    {RegisterIndex::ah, RegisterIndex::sp},
    {RegisterIndex::ch, RegisterIndex::bp},
    {RegisterIndex::dh, RegisterIndex::si},
    {RegisterIndex::bh, RegisterIndex::di},
};

u16 registersMem[8] = {};

u16* GetRegisterMem(RegisterIndex regIndex)
{
    switch (regIndex)
    {
    case None:  assert(false); return 0;    break;
    case al:
    case ah:
    case ax:    return &registersMem[0];    break;
    case cl:
    case ch:
    case cx:    return &registersMem[1];    break;
    case dl:
    case dh:
    case dx:    return &registersMem[2];    break;
    case bl:
    case bh:
    case bx:    return &registersMem[3];    break;
    case sp:    return &registersMem[4];    break;
    case bp:    return &registersMem[5];    break;
    case si:    return &registersMem[6];    break;
    case di:    return &registersMem[7];    break;
    }

    assert(false);
    return 0;
}

bool flags[Flag::FLAG_COUNT] = {};

#define MAIN_MEMORY_LIMIT 65536
u8 mainMemory[MAIN_MEMORY_LIMIT] = {};

struct MemoryAccess
{
    enum class Type {None, Byte, Word, Full};

    struct Word
    {
        u8* low{nullptr};
        u8* high{nullptr};
    };

    Type type;
    union {
        u8* byte;
        Word word;
        u16* full;
    };

    void SetAddress(u8* mainMemmoryAddr)
    {
        switch (type)
        {
        case Type::Byte:
            byte = mainMemmoryAddr;
            break;
        case Type::Word:
            word.low = mainMemmoryAddr;
            word.high = mainMemmoryAddr + 1;
            break;
        }
    }

    void SetData(u16 data)
    {
        switch (type)
        {
        case Type::None:
        case Type::Byte:
            *byte = data & 255;
            break;
        case Type::Word:
            *(word.low)  = data & 0b1111'1111;
            *(word.high) = data & 0b1111'1111'0000'0000;
            break;
        case Type::Full:
            *full = data;
            break;
        default:
            break;
        }
    }

    u16 operator+(u16 data) const
    {
        switch (type)
        {
        case Type::None:
        case Type::Byte:
            return *byte + data & 255;
        case Type::Word:
        {
            u8 tmpLow =  *(word.low) + data & 0b1111'1111;
            u8 tmpHigh = *(word.high) + data & 0b1111'1111'0000'0000;
            return (tmpHigh << 8) | tmpLow;
        }
        case Type::Full:
            return *full + data;
        }
        assert(false);
        return 0;
    }

    u16 operator-(u16 data) const
    {
        switch (type)
        {
        case Type::None:
        case Type::Byte:
            return *byte - data & 255;
        case Type::Word:
        {
            u8 tmpLow =  *(word.low) - data & 0b1111'1111;
            u8 tmpHigh = *(word.high) - data & 0b1111'1111'0000'0000;
            return (tmpHigh << 8) | tmpLow;
        }
        case Type::Full:
            return *full - data;
        }
        assert(false);
        return 0;
    }

    u16 const& operator*() const
    {
        switch (type)
        {
        case Type::Byte:
            return *byte;
            break;
        case Type::Word:
            return ((*word.high) << 8) | (*word.low);
            break;
        case Type::Full:
            return *full;
            break;
        }
        assert(false);
        return 0;
    }

    bool IsWide()
    {
        return type == Type::Word || type == Type::Full;
    }

};
