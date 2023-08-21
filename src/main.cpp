#include <iostream>
#include <fstream>
#include <filesystem>
#include <bitset>
#include <string>

typedef uint8_t     u8;
typedef uint16_t    u16;
typedef int8_t      s8;
typedef int16_t     s16;

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

const char* GetSign(s16 byte)
{
    return byte >= 0 ? " + " : " - ";
}

bool IsReg_Mem_Reg(u8 byte)
{
    u8 mask = 0b1111'1100;
    return (byte & mask) == 0b1000'1000  // MOV
        || (byte & mask) == 0b0000'0000  // ADD
        || (byte & mask) == 0b0010'1000  // SUB
        || (byte & mask) == 0b0011'1000; // CMP
}

bool IsImm_Reg_Mem(u8 byte)
{
    return ((byte >> 1) << 1) == 0b1100'0110  // MOV
        || ((byte >> 2) << 2) == 0b1000'0000; // ADD, SUB, CMPs
}

bool IsImm_Accumulator(u8 byte)
{
    u8 mask = 0b1111'1110;
    return (byte & mask) == 0b0000'0100  // ADD
        || (byte & mask) == 0b0010'1100  // SUB
        || (byte & mask) == 0b0011'1100; // CMP
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

u16 CombineLoAndHiToWord(const std::vector<u8>& bytesArr, int* byteIndex)
{
    const u16 byteLow  = bytesArr[++(*byteIndex)];
    const u16 byteHigh = bytesArr[++(*byteIndex)];
    return (byteHigh << 8) | ((u16)byteLow);
}

std::string CombineLoAndHiToString(const std::vector<u8>& bytesArr, int* byteIndex)
{
    return std::to_string(CombineLoAndHiToWord(bytesArr, byteIndex));
}

int main()
{
    //std::ifstream file("listings/0038_many_register_mov", std::ios::binary);
    //std::ifstream file("listings/listing_0039_more_movs", std::ios::binary);
    //std::ifstream file("listings/listing_0040_challenge_movs", std::ios::binary);
    std::ifstream file("listings/listing_0041_add_sub_cmp_jnz", std::ios::binary);
    if (!file.is_open())
    {
        std::cerr << "!!! Can't open file !!!\n";
        return 0;
    }
    std::vector<u8> bytes(
        (std::istreambuf_iterator<char>(file)),
        (std::istreambuf_iterator<char>())
    );

    for (const auto& byte : bytes)
    {
        //PrintByte(byte);
    }

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

    enum OpIndex { ADD = 0, MOV = 1, SUB = 5, CMP = 7, UNDEFINED = -1 };

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

    std::cout << "bits 16\n";

    int byteIndex = 0;
    while (byteIndex < bytes.size())
    {
        std::string leftOperand, rightOperand;

        u8 byte = bytes[byteIndex];
        u8 bitD = (byte & 2) >> 1;
        u8 bitW = (byte & 1);

        if (IsReg_Mem_Reg(byte))
        {
            u8 opIndex = (byte & 0b0011'1000) >> 3;
            std::cout << operations[opIndex];

            byte = bytes[++byteIndex];

            u8 mod = (byte >> 6);
            u8 reg = (byte & 0b00'111'000) >> 3;
            u8 rm =  (byte & 0b00'000'111);
            if (mod == 3) // register mode
            {
                leftOperand  = registers[rm][bitW];
                rightOperand = registers[reg][bitW];
            }
            else
            {
                leftOperand  = registers[reg][bitW];
                rightOperand = effectiveAddresses[rm];

                if (mod == 0) // memory mode, no displacement follows
                {
                    // BUT if a special occassion for a DIRECT ADDRESS case
                    if (rm == 0b0110)
                    {
                        u16 disp = bitW == 0 ? bytes[++byteIndex] : CombineLoAndHiToWord(bytes, &byteIndex);
                        rightOperand = '[' + disp;
                    }
                }
                else if (mod == 1) // memory mode, 8-bit displacement follows
                {
                    s8 disp = bytes[++byteIndex];
                    if (bitW == 1)
                        rightOperand += GetSign(disp) + std::to_string(abs(disp));
                    else
                        rightOperand += " + " + std::to_string(disp);
                }
                else if (mod == 2) // memory mode, 16-bit displacement follows
                {
                    u16 disp = CombineLoAndHiToWord(bytes, &byteIndex);
                    if (bitW == 1)
                        rightOperand += GetSign(disp) + std::to_string(disp);
                    else
                        rightOperand += " + " + std::to_string(disp);
                }

                rightOperand += ']';
                if (bitD == 0)
                {
                    std::swap(leftOperand, rightOperand);
                }
            }
        }
        else if (IsImm_Reg_Mem(byte))
        {
            u8 bitS = bitD;

            byte = bytes[++byteIndex];
            u8 mod     = (byte >> 6);
            u8 opIndex = (byte & 0b00'111'000) >> 3;
            u8 rm      = (byte & 0b00'000'111);

            std::cout << operations[opIndex];

            if (mod == 3) // register mode
            {
                leftOperand = registers[rm][bitW];
                rightOperand = bitS == 0 && bitW == 1 ? CombineLoAndHiToString(bytes, &byteIndex) : std::to_string(bytes[++byteIndex]);
            }
            else if (mod == 0) // memory mode, no displacement follows
            {
                if (opIndex == OpIndex::MOV)
                {
                    leftOperand = effectiveAddresses[rm] + ']';
                    if (bitW == 0)
                        rightOperand = "byte " + bytes[++byteIndex];
                    else
                        rightOperand = "word " + CombineLoAndHiToString(bytes, &byteIndex);
                }
                else
                {
                    if (bitW == 1)
                    {
                         // SPECIAL CASE
                        if (rm == 0b0110)
                            leftOperand = "word [" + CombineLoAndHiToString(bytes, &byteIndex) + ']';
                        else
                            leftOperand = "word " + std::string(effectiveAddresses[rm]) + ']';

                        if (bitS == 0)
                            rightOperand = CombineLoAndHiToString(bytes, &byteIndex);
                        else
                            rightOperand = std::to_string(bytes[++byteIndex]);
                    }
                    else
                    {
                        leftOperand = "byte " + std::string(effectiveAddresses[rm]) + ']';
                        rightOperand = std::to_string(bytes[++byteIndex]);
                    }
                }
            }
            else if (mod == 1) // memory mode, 8-bit displacement follows
            {
                leftOperand = effectiveAddresses[rm];
                u16 disp = bytes[++byteIndex];
                if (bitW == 1)
                    leftOperand += GetSign(disp) + std::to_string(abs((s16)disp)) + ']';
                else
                    leftOperand += " + " + disp + ']';
            }
            else if (mod == 2) // memory mode, 16-bit displacement follows
            {
                const auto disp = CombineLoAndHiToString(bytes, &byteIndex);
                //if (bitW == 1)
                //    std::cout << effectiveAddresses[rm] << " - " << std::numeric_limits<u16>::max() - disp + 1 << ']';
                //else
                if (opIndex == OpIndex::MOV)
                {
                    leftOperand = std::string(effectiveAddresses[rm]) + " + " + disp + ']';
                    if (bitW == 0)
                        rightOperand = "byte " + std::to_string(bytes[++byteIndex]);
                    else
                        rightOperand = "word " + CombineLoAndHiToString(bytes, &byteIndex);
                }
                else
                {
                    if (bitW == 1)
                    {
                        leftOperand = "word " + std::string(effectiveAddresses[rm]) + " + " + disp + ']';
                        if (bitS == 0)
                            rightOperand = CombineLoAndHiToString(bytes, &byteIndex);
                        else
                            rightOperand = std::to_string(bytes[++byteIndex]);
                    }
                    else
                    {
                        leftOperand = "byte " + std::string(effectiveAddresses[rm]) + " + " + disp + ']';
                        rightOperand = std::to_string(bytes[++byteIndex]);
                    }
                }
            }
        }
        else if ((byte & 0b1111'0000) == 0b1011'0000) // MOV immediate to register
        {
            std::cout << "mov ";
            u8 bitW = (byte & 0b0000'1000) >> 3;
            u8 reg  = (byte & 0b0000'0111);

            leftOperand = registers[reg][bitW];
            if (bitW == 0)
                rightOperand = std::to_string(bytes[++byteIndex]);
            else
                rightOperand = CombineLoAndHiToString(bytes, &byteIndex);
        }
        else if ((byte & 0b1111'1100) == 0b1010'0000) // MOV accumulator
        {
            std::cout << "mov ";

            const auto data = CombineLoAndHiToString(bytes, &byteIndex);
            if (bitD == 0)
            {
                leftOperand = "ax";
                rightOperand = '[' + data + ']';
            }
            else
            {
                leftOperand = '[' + data + ']';
                rightOperand = "ax";
            }
        }
        else if (IsImm_Accumulator(byte))
        {
            u8 mask = 0b0011'1100;
            const auto opIndex = 
                (byte & mask) == 0b0000'0100 ? OpIndex::ADD :
                (byte & mask) == 0b0010'1100 ? OpIndex::SUB :
                (byte & mask) == 0b0011'1100 ? OpIndex::CMP : OpIndex::UNDEFINED;
            std::cout << operations[opIndex];
            if (bitW == 0)
            {
                leftOperand = "al";
                rightOperand = std::to_string(bytes[++byteIndex]);
            }
            else
            {
                leftOperand = "ax";
                rightOperand = CombineLoAndHiToString(bytes, &byteIndex);
            }
        }
        else if (IsJump(byte) || IsLoop(byte))
        {
            if (IsJump(byte))
            {
                u8 opIndex = byte & 0b0000'1111;
                std::cout << jumps[opIndex];
            }
            else
            {
                u8 opIndex = byte & 0b0000'0011;
                std::cout << loops[opIndex];
            }

            auto disp = (s8)bytes[++byteIndex] + 2;
            std::string dispStr = std::to_string(disp);
            std::cout << '$';
            if (disp > 0)
                std::cout << '+' << dispStr;
            else if (disp < 0)
                std::cout << dispStr;
            std::cout << "+0";
        }

        if (leftOperand != "" && rightOperand != "")
            std::cout << leftOperand << ", " << rightOperand;
        else
            std::cout << leftOperand;
        std::cout << '\n';
        byteIndex++;
    }
    return 0;
}