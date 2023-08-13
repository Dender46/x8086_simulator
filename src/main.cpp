#include <iostream>
#include <fstream>
#include <filesystem>
#include <bitset>
#include <string>

constexpr uint8_t opcodeMOVImmediateToREG       = 0b1011'0000;
constexpr uint8_t opcodeMOVAccumulator          = 0b1010'0000;

void PrintByte(uint8_t byte)
{
    for (int k = 7; k >= 0; k--)
    {
        int bit = (byte & (1 << k)) >> k;
        std::cout << bit;
    }
    std::cout << " ";
}

std::string ByteInStr(uint8_t byte)
{
    std::string result;
    for (int k = 7; k >= 0; k--)
    {
        int bit = (byte & (1 << k)) >> k;
        result += std::to_string(bit);
    }
    return result;
}

std::string ByteInStr2(uint16_t byte)
{
    std::string result;
    for (int k = 15; k >= 0; k--)
    {
        int bit = (byte & (1 << k)) >> k;
        result += std::to_string(bit);
    }
    return result;
}

const char* GetSign8(int8_t byte)
{
    return byte >= 0 ? " + " : " - ";
}

const char* GetSign16(int16_t byte)
{
    return byte >= 0 ? " + " : " - ";
}

bool IsReg_Mem_Reg(uint8_t byte)
{
    uint8_t mask = 0b1111'1100;
    return (byte & mask) == 0b1000'1000  // MOV
        || (byte & mask) == 0b0000'0000  // ADD
        || (byte & mask) == 0b0010'1000  // SUB
        || (byte & mask) == 0b0011'1000; // CMP
}

bool IsImm_Reg_Mem(uint8_t byte)
{
    return ((byte >> 1) << 1) == 0b1100'0110  // MOV
        || ((byte >> 2) << 2) == 0b1000'0000; // ADD, SUB, CMPs
}

bool IsImm_Accumulator(uint8_t byte)
{
    uint8_t mask = 0b1111'1110;
    return (byte & mask) == 0b0000'0100  // ADD
        || (byte & mask) == 0b0010'1100  // SUB
        || (byte & mask) == 0b0011'1100; // CMP
}

uint16_t CombineLoAndHiToWord(const std::vector<uint8_t>& bytesArr, int& byteIndex)
{
    const uint16_t byteLow  = bytesArr[++byteIndex];
    const uint16_t byteHigh = bytesArr[++byteIndex];
    return (byteHigh << 8) | ((uint16_t)byteLow);
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
    std::vector<uint8_t> bytes(
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

    // mov: reg_mem_reg 10'001'0dw  imm_reg_mem 1100011w  imm_to_reg 1011wreg
    // add: reg_mem_reg 00'000'0dw  imm_reg_mem 100000sw  imm_accum  0000010w
    // sub: reg_mem_reg 00'101'0dw  imm_reg_mem 100000sw  imm_accum  0010110w
    // cmp: reg_mem_reg 00'111'0dw  imm_reg_mem 100000sw  imm_accum  0011110w

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
        uint8_t byte = bytes[byteIndex];

        if (IsReg_Mem_Reg(byte))
        {
            uint8_t opIndex = (byte & 0b0011'1000) >> 3;
            std::cout << operations[opIndex];
            uint8_t bitD = (byte & 2) >> 1;
            uint8_t bitW = (byte & 1);

            byte = bytes[++byteIndex];

            uint8_t mod = (byte >> 6);
            uint8_t reg = (byte & 0b00'111'000) >> 3;
            uint8_t rm =  (byte & 0b00'000'111);
            if (mod == 3) // register mode
            {
                std::cout << registers[rm][bitW] << ", " << registers[reg][bitW];
            }
            else
            {
                if (bitD == 1)
                {
                    std::cout << registers[reg][bitW] << ", ";
                }
                if (mod == 0) // memory mode, no displacement follows
                {
                    if (rm == 0b0110) // BUT if a special occassion for a DIRECT ADDRESS case
                    {
                        if (bitW == 0)
                        {
                            uint16_t disp = bytes[++byteIndex];
                            std::cout << '[' << disp << ']';
                        }
                        else
                        {
                            uint16_t disp = CombineLoAndHiToWord(bytes, byteIndex);
                            std::cout << '[' << disp << ']';
                        }
                    }
                    else
                    {
                        std::cout << effectiveAddresses[rm] << ']';
                    }
                }
                else if (mod == 1) // memory mode, 8-bit displacement follows
                {
                    uint16_t disp = bytes[++byteIndex];
                    if (bitW == 1)
                        std::cout << effectiveAddresses[rm] << GetSign8(disp) << std::to_string(abs((int8_t)disp)) << ']';
                    else
                        std::cout << effectiveAddresses[rm] << " + " << disp << ']';
                }
                else if (mod == 2) // memory mode, 16-bit displacement follows
                {
                    uint16_t disp = CombineLoAndHiToWord(bytes, byteIndex);
                    if (bitW == 1)
                        std::cout << effectiveAddresses[rm] << GetSign16(disp) << (int16_t)disp << ']';
                    else
                        std::cout << effectiveAddresses[rm] << " + " << disp << ']';
                }
                if (bitD == 0)
                {
                    std::cout << ", " << registers[reg][bitW];
                }
            }
        }
        else if (IsImm_Reg_Mem(byte))
        {
            uint8_t bitW = (byte & 1);
            uint8_t bitS = (byte & 2) >> 1; // bit s near w

            byte = bytes[++byteIndex];
            uint8_t mod     = (byte >> 6);
            uint8_t opIndex = (byte & 0b00'111'000) >> 3;
            uint8_t rm      = (byte & 0b00'000'111);

            std::cout << operations[opIndex];

            if (mod == 3) // register mode
            {
                std::cout << registers[rm][bitW] << ", ";
                if (bitS == 0 && bitW == 1)
                {
                    uint16_t data = CombineLoAndHiToWord(bytes, byteIndex);
                    std::cout << data;
                }
                else
                {
                    uint16_t data = bytes[++byteIndex];
                    std::cout << data;
                }
            }
            else if (mod == 0) // memory mode, no displacement follows
            {
                if (opIndex == OpIndex::MOV)
                {
                    std::cout << effectiveAddresses[rm] << "], ";
                    if (bitW == 0)
                    {
                        uint16_t data = bytes[++byteIndex];
                        std::cout << "byte " << data;
                    }
                    else
                    {
                        uint16_t data = CombineLoAndHiToWord(bytes, byteIndex);
                        std::cout << "word " << data;
                    }
                }
                else
                {
                    if (bitW == 1)
                    {
                        std::cout << "word " << effectiveAddresses[rm] << "], ";
                        if (bitS == 0)
                            std::cout << CombineLoAndHiToWord(bytes, byteIndex);
                        else
                            std::cout << std::to_string(bytes[++byteIndex]);
                    }
                    else
                    {
                        std::cout << "byte " << effectiveAddresses[rm] << "], " << std::to_string(bytes[++byteIndex]);
                    }
                }
            }
            else if (mod == 1) // memory mode, 8-bit displacement follows
            {
                uint16_t disp = bytes[++byteIndex];
                if (bitW == 1)
                    std::cout << effectiveAddresses[rm] << GetSign16(disp) << abs((int16_t)disp) << ']';
                else
                    std::cout << effectiveAddresses[rm] << " + " << disp << ']';
            }
            else if (mod == 2) // memory mode, 16-bit displacement follows
            {
                uint16_t disp = CombineLoAndHiToWord(bytes, byteIndex);
                //if (bitW == 1)
                //    std::cout << effectiveAddresses[rm] << " - " << std::numeric_limits<uint16_t>::max() - disp + 1 << ']';
                //else
                if (opIndex == OpIndex::MOV)
                {
                    std::cout << effectiveAddresses[rm] << " + " << disp << "], ";
                    if (bitW == 0)
                    {
                        uint16_t data = bytes[++byteIndex];
                        std::cout << "byte " << data;
                    }
                    else
                    {
                        uint16_t data = CombineLoAndHiToWord(bytes, byteIndex);
                        std::cout << "word " << data;
                    }
                }
                else
                {
                    if (bitW == 1)
                    {
                        std::cout << "word " << effectiveAddresses[rm] << " + " << disp << "], ";
                        if (bitS == 0)
                            std::cout << CombineLoAndHiToWord(bytes, byteIndex);
                        else
                            std::cout << std::to_string(bytes[++byteIndex]);
                    }
                    else
                    {
                        uint16_t data = bytes[++byteIndex];
                        std::cout << "byte " << effectiveAddresses[rm] << " + " << disp << "], " << data;
                    }
                }
            }
        }
        else if (((byte >> 4) << 4) == opcodeMOVImmediateToREG)
        {
            std::cout << "mov ";
            uint8_t bitW = (byte & 0b0000'1000) >> 3;
            uint8_t reg = (byte & 0b0000'0111);

            std::cout << registers[reg][bitW] << ", ";
            if (bitW == 0)
            {
                std::cout << (int16_t)bytes[++byteIndex];
            }
            else
            {
                std::cout << CombineLoAndHiToWord(bytes, byteIndex);
            }
        }
        else if (((byte >> 2) << 2) == opcodeMOVAccumulator)
        {
            std::cout << "mov ";
            uint8_t bitD = (byte & 2) >> 1; // specification doesn't really say that this is bit D, but idea seems the same with the direction
            uint8_t bitW = (byte & 1);

            uint16_t data = CombineLoAndHiToWord(bytes, byteIndex);
            if (bitD == 0)
            {
                std::cout << "ax, [" << data << "]";
            }
            else
            {
                std::cout << "[" << data << "], ax";
            }
        }
        else if (IsImm_Accumulator(byte))
        {
            const auto opIndex = 
                (byte & 0b0000'0100) == 0b0000'0100 ? OpIndex::ADD :
                (byte & 0b0010'1100) == 0b0000'0100 ? OpIndex::SUB :
                (byte & 0b0011'1100) == 0b0000'0100 ? OpIndex::CMP : OpIndex::UNDEFINED;
            std::cout << operations[opIndex];
            uint8_t bitW = (byte & 1);
            if (bitW == 0)
            {
                uint8_t data = bytes[++byteIndex];
                std::cout << "al, " << GetSign8(data) << std::to_string(abs((int8_t)data));
            }
            else
            {
                uint16_t data = CombineLoAndHiToWord(bytes, byteIndex);
                std::cout << "ax, " << GetSign16(data) << abs((int16_t)data);
            }
        }
        std::cout << '\n';
        byteIndex++;

    }
    return 0;
}