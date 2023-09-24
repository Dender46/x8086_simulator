#pragma once

constexpr const char* registerNames[] = {
    "__",
    "al", "ah", "ax",
    "bl", "bh", "bx",
    "cl", "ch", "cx",
    "dl", "dh", "dx",
    "sp",
    "bp",
    "si",
    "di",
};

constexpr const char* effectiveAddressesStr[] = {
    {"[bx + si"},
    {"[bx + di"},
    {"[bp + si"},
    {"[bp + di"},
    {"[si"},
    {"[di"},
    {"[bp"},
    {"[bx"},
};

constexpr const char* jumpNames[] = {
    "jo ",
    "jno ",
    "jb ",  //"jnae ",
    "jnb ", //"jae ",
    "je ",  //"jz ",
    "jne ", //"jnz ",
    "jbe ", //"jna ",
    "ja ",  //"jnbe ",
    "js ",
    "jns ",
    "jp ",  //"jpe ",
    "jnp ", //"jpo ",
    "jl ",  //"jnge ",
    "jnl ", //"jge ",
    "jle ", //"jng ",
    "jnle ",//"jg ",
};

constexpr const char* loopNames[] = {
    "loopnz ",  //"loopne ",
    "loopz ",   //"loope ",
    "loop ",
    "jcxz ",
};

constexpr const char* operationNames[] = {
    "add ", // 0
    "mov ", // 1
    "___ ", // 2
    "___ ", // 3
    "___ ", // 4
    "sub ", // 5
    "___ ", // 6
    "cmp ", // 7
};
