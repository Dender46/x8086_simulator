#pragma once

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
bool flags[Flag::FLAG_COUNT] = {};
