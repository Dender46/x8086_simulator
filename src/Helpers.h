#pragma once
#include <cassert>

/* START OF STRING HELPERS */
std::string HexString(u16 byte)
{
    std::stringstream stream;
    stream << std::hex << "0x" << byte;
    return stream.str();
}

const char* FlagStr(Flag flag)
{
    switch (flag)
    {
    case FLAG_ZERO:     return "Z";
    case FLAG_SIGNED:   return "S";
    case FLAG_COUNT:    assert(false);
    }
    return "";
}

const char* GetSign(s16 byte)
{
    return byte >= 0 ? " + " : " - ";
}
/* END OF STRING HELPERS */


/* START OF DEBUG FUNCTIONS */
void PrintByte(u8 byte)
{
    for (int k = 7; k >= 0; k--)
    {
        int bit = (byte & (1 << k)) >> k;
        std::cout << bit;
    }
    std::cout << " ";
}

std::string ByteInStr(u8 byte)
{
    std::string result;
    for (int k = 7; k >= 0; k--)
    {
        int bit = (byte & (1 << k)) >> k;
        result += std::to_string(bit);
    }
    return result;
}

std::string ByteInStr2(u16 byte)
{
    std::string result;
    for (int k = 15; k >= 0; k--)
    {
        int bit = (byte & (1 << k)) >> k;
        result += std::to_string(bit);
    }
    return result;
}
/* END OF DEBUG FUNCTIONS */
