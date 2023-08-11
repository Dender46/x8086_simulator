#include <iostream>
#include <fstream>
#include <filesystem>
#include <bitset>
#include <string>

void PrintBits(uint8_t byte)
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

uint16_t CombineLoAndHiToWord(const std::vector<uint8_t>& bytesArr, int& byteIndex)
{
    const uint16_t byteLow  = bytesArr[++byteIndex];
    const uint16_t byteHigh = bytesArr[++byteIndex];
    return (byteHigh << 8) | ((uint16_t)byteLow);
}

int main()
{
    //std::ifstream file("0038_many_register_mov", std::ios::binary);
    std::ifstream file("listing_0039_more_movs", std::ios::binary);
    //std::ifstream file("listing_0040_challenge_movs", std::ios::binary);
    if (!file.is_open())
    {
        std::cerr << "!!! Can't open file !!!";
    }
    std::vector<uint8_t> bytes(
        (std::istreambuf_iterator<char>(file)),
        (std::istreambuf_iterator<char>())
    );

    for (const auto& byte : bytes)
    {
        //PrintBits(byte);
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

    std::cout << "bits 16\n";
    constexpr uint8_t opcodeMOV                     = 0b1000'1000;
    constexpr uint8_t opcodeMOVImmediateToREG_MEM   = 0b1100'0110;
    constexpr uint8_t opcodeMOVImmediateToREG       = 0b1011'0000;
    constexpr uint8_t opcodeMOVAccumulator          = 0b1010'0000;

    int byteIndex = 0;
    while (byteIndex < bytes.size())
    {
        uint8_t byte = bytes[byteIndex];
        if (((byte >> 3) << 3) == opcodeMOV)
        {
            std::cout << "mov ";
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
                    if (bitW == 1 && disp != 0)
                        std::cout << effectiveAddresses[rm] << " " << std::to_string((int8_t)disp) << ']';
                    else
                        std::cout << effectiveAddresses[rm] << " + " << disp << ']';
                }
                else if (mod == 2) // memory mode, 16-bit displacement follows
                {
                    uint16_t disp = CombineLoAndHiToWord(bytes, byteIndex);
                    if (bitW == 1)
                        std::cout << effectiveAddresses[rm] << " " << (int16_t)disp << ']';
                    else
                        std::cout << effectiveAddresses[rm] << " + " << disp << ']';
                }
                if (bitD == 0)
                {
                    std::cout << ", " << registers[reg][bitW];
                }
            }
        }
        else if (((byte >> 1) << 1) == opcodeMOVImmediateToREG_MEM)
        {
            std::cout << "mov ";
            uint8_t bitW = (byte & 1);

            byte = bytes[++byteIndex];

            uint8_t mod = (byte >> 6);
            uint8_t rm =  (byte & 0b00'000'111);
            if (mod == 0) // memory mode, no displacement follows
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
            else if (mod == 1) // memory mode, 8-bit displacement follows
            {
                uint16_t disp = bytes[++byteIndex];
                if (bitW == 1)
                    std::cout << effectiveAddresses[rm] << " " << (int16_t)disp << ']';
                else
                    std::cout << effectiveAddresses[rm] << " + " << disp << ']';
            }
            else if (mod == 2) // memory mode, 16-bit displacement follows
            {
                uint16_t disp = CombineLoAndHiToWord(bytes, byteIndex);
                //if (bitW == 1)
                //    std::cout << effectiveAddresses[rm] << " - " << std::numeric_limits<uint16_t>::max() - disp + 1 << ']';
                //else
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
        std::cout << '\n';
        byteIndex++;

    }
    return 0;
}