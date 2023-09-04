#pragma once

constexpr const char* registers[][2] = {
    {"al", "ax"},
    {"cl", "cx"},
    {"dl", "dx"},
    {"bl", "bx"},
    {"ah", "sp"},
    {"ch", "bp"},
    {"dh", "si"},
    {"bh", "di"},
};

constexpr const char* effectiveAddresses[] = {
    {"[bx + si"},
    {"[bx + di"},
    {"[bp + si"},
    {"[bp + di"},
    {"[si"},
    {"[di"},
    {"[bp"},
    {"[bx"},
};

constexpr const char* jumps[] = {
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

constexpr const char* loops[] = {
    "loopnz ",  //"loopne ",
    "loopz ",   //"loope ",
    "loop ",
    "jcxz ",
};

constexpr const char* operations[] = {
    {"add "}, // 0
    {"mov "}, // 1
    {"___ "}, // 2
    {"___ "}, // 3
    {"___ "}, // 4
    {"sub "}, // 5
    {"___ "}, // 6
    {"cmp "}, // 7
};
