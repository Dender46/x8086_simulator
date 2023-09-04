#pragma once

enum Flag { FLAG_ZERO, FLAG_SIGNED,   FLAG_COUNT };

u16 registersMem[8] = {};
bool flags[Flag::FLAG_COUNT] = {};
