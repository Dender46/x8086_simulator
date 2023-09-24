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
