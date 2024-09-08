#include <fstream>
#include <chrono>
#include <random>
#include <chip8.hpp>
#include <cstring>
using namespace std;

const unsigned int START_ADDRESS = 0x200;
const unsigned int FONTSET_SIZE = 80;
uint8_t fontset[FONTSET_SIZE] =
    {
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};
const unsigned int FONTSET_START_ADDRESS = 0x50;

// constructer
Chip8::Chip8()
    : randGen(std::chrono::system_clock::now().time_since_epoch().count()),
      randByte(0, 255U)
{
    // inititalize pc
    //  Load fonts into memory
    unsigned int size = FONTSET_SIZE;

    for (unsigned int i = 0; i < FONTSET_SIZE; ++i)
    {
        memory[FONTSET_START_ADDRESS + i] = fontset[i];
    }
    pc = START_ADDRESS;

    table[0x1] = &Chip8::OP_1nnn;
    table[0x2] = &Chip8::OP_2nnn;
    table[0x3] = &Chip8::OP_3xkk;
    table[0x4] = &Chip8::OP_4xkk;
    table[0x5] = &Chip8::OP_5xy0;
    table[0x6] = &Chip8::OP_6xkk;
    table[0x7] = &Chip8::OP_7xkk;
    table[0x8] = &Chip8::Table8;
    table[0x9] = &Chip8::OP_9xy0;
    table[0xA] = &Chip8::OP_Annn;
    table[0xB] = &Chip8::OP_Bnnn;
    table[0xC] = &Chip8::OP_Cxkk;
    table[0xD] = &Chip8::OP_Dxyn;
    table[0xE] = &Chip8::TableE;
    table[0xF] = &Chip8::TableF;

    for (size_t i = 0; i <= 0xE; i++)
    {
        table0[i] = &Chip8::OP_NULL;
        table8[i] = &Chip8::OP_NULL;
        tableE[i] = &Chip8::OP_NULL;
    }
    for (size_t i = 0; i <= 0x65; i++)
    {
        tableF[i] = &Chip8::OP_NULL;
    }
    // first point all of them to null pointer so that there are no non empty ones if any memeory adress is missed

    table0[0x0] = &Chip8::OP_00E0;
    table0[0xE] = &Chip8::OP_00EE;

    tableE[0xA1] = &Chip8::OP_ExA1;
    tableE[0x9E] = &Chip8::OP_Ex9E;

    tableF[0x07] = &Chip8::OP_Fx07;
    tableF[0x0A] = &Chip8::OP_Fx0A;
    tableF[0x15] = &Chip8::OP_Fx15;
    tableF[0x18] = &Chip8::OP_Fx18;
    tableF[0x1E] = &Chip8::OP_Fx1E;
    tableF[0x29] = &Chip8::OP_Fx29;
    tableF[0x33] = &Chip8::OP_Fx33;
    tableF[0x55] = &Chip8::OP_Fx55;
    tableF[0x65] = &Chip8::OP_Fx65;

    table8[0x0] = &Chip8::OP_8xy0;
    table8[0x1] = &Chip8::OP_8xy1;
    table8[0x2] = &Chip8::OP_8xy2;
    table8[0x3] = &Chip8::OP_8xy3;
    table8[0x4] = &Chip8::OP_8xy4;
    table8[0x5] = &Chip8::OP_8xy5;
    table8[0x6] = &Chip8::OP_8xy6;
    table8[0x7] = &Chip8::OP_8xy7;
    table8[0xE] = &Chip8::OP_8xyE;
    randByte = std::uniform_int_distribution<uint8_t>(0, 255U);
}

//////////////////////////////////////////////////////
void Chip8::Cycle()
{
    // decode and ffetch is hanndled bu the opcode fucntions but now we nneed to code the fetch functionality
    opcode = (memory[pc] << 8u | memory[pc + 1]);
    // chip is 16bit system first we take 8bit from memory and left shift it 8 places and insert the next 8 bits from its next location memory[pc+1] and insert it after its position
    pc = pc + 2;
    // decodde and execute
    ((this)->*(table[(opcode & 0xF000u) >> 12u]))();
    if (delayTimer > 0)
    {
        delayTimer--;
    }
    if (soundTimer > 0)
    {
        soundTimer--;
    }
}

void Chip8::LoadRom(char const *filename)
{
    ifstream file(filename, ios::binary | ios::ate);
    if (file.is_open())
    {
        streampos size = file.tellg();
        char *buffer = new char[size];

        file.seekg(0, ios::beg);
        file.read(buffer, size);
        file.close();

        if (buffer != nullptr && size > 0)
        {
            for (int i = 0; i < size; i++)
            {
                memory[START_ADDRESS + i] = buffer[i];
            }
        }

        delete[] buffer;
    }
}

/// @brief ///////////////////////////////////////////////////////////
//! Function pointers
void Chip8::Table0()
{
    ((this)->*(table0[opcode & 0x000Fu]))();
}
void Chip8::TableE()
{
    (this->*(tableE[opcode & 0x000Fu]))();
}

void Chip8::TableF()
{
    ((*this).*(tableF[opcode & 0x00FFu]))();
}
void Chip8::Table8()
{
    (this->*(table8[opcode & 0x000Fu]))();
}
/// @brief ////////////////////////////
//! OPcode implementation
void Chip8::OP_NULL()
{
}
void Chip8::OP_00E0()
{
    memset(video, 0, sizeof(video));
}

void Chip8::OP_00EE()
{
    sp--;
    pc = stack[sp];
}

// JP address
void Chip8::OP_1nnn()
{
    uint16_t addr = opcode & 0xFFFu;
    pc = addr;
}

// CALL address
void Chip8::OP_2nnn()
{
    uint16_t addr = opcode & 0xFFFu;
    stack[sp] = pc;
    ++sp;
    pc = addr;
}

// SE Vx, byte
void Chip8::OP_3xkk()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t byte = opcode & 0x00FFu;
    if (registers[Vx] = byte)
    {
        pc = pc + 2;
    }
}

// SNE Vx, byte
void Chip8::OP_4xkk()
{

    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t byte = opcode & 0x00FFu;
    if (registers[Vx] != byte)
    {
        pc = pc + 2;
    }
}

// SE Vx, Vy
void Chip8::OP_5xy0()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;
    if (registers[Vx] == registers[Vy])
    {
        pc = pc + 2;
    }
}

// LD Vx, byte
void Chip8::OP_6xkk()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t byte = opcode & 0x00FFu;
    registers[Vx] = byte;
}

// ADD Vx, byte
void Chip8::OP_7xkk()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t byte = opcode & 0x00FFu;

    // Vx = Vx+byte;
    registers[Vx] = registers[Vx] + byte;
}

// LD Vx, Vy
void Chip8::OP_8xy0()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;
    registers[Vx] = registers[Vy];
}

// OR Vx, Vy
void Chip8::OP_8xy1()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;
    registers[Vx] = registers[Vx] | registers[Vy];
}

// AND Vx, Vy
void Chip8::OP_8xy2()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;
    registers[Vx] = registers[Vx] & registers[Vy];
}

// XOR Vx, Vy
void Chip8::OP_8xy3()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;
    registers[Vx] = registers[Vx] ^ registers[Vy];
}

// ADD Vx, Vy
void Chip8::OP_8xy4()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;
    uint16_t result = static_cast<uint16_t>(registers[Vx]) + static_cast<uint16_t>(registers[Vy]);

    if (result > 0xFF)
    {
        registers[15] = 1;
    }
    else
    {
        registers[15] = 0;
    }
    registers[Vx] = result & 0XFFu;
}

// SUB Vx, Vy
void Chip8::OP_8xy5()
{

    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    // Subtract Vy from Vx
    if (registers[Vx] >= registers[Vy])
    {
        // No borrow, set carry flag to 1
        registers[15] = 1;
    }
    else
    {
        // Borrow, set carry flag to 0
        registers[15] = 0;
    }
    registers[Vx] -= registers[Vy];
}

// SHR Vx
void Chip8::OP_8xy6()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t LSB_Vx = Vx & 0x01;
    if (LSB_Vx == 1)
    {
        registers[15] = 1;
    }
    else
    {
        registers[15] = 0;
    }
    registers[Vx] = registers[Vx] >> 1;
}

// SUBN Vx, Vy
void Chip8::OP_8xy7()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;
    if (registers[Vy] > registers[Vx])
    {
        registers[15] = 1;
    }
    else
    {
        registers[15] = 0;
    }
    registers[Vx] = registers[Vy] - registers[Vx];
}

// SHL Vx
void Chip8::OP_8xyE()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    uint8_t MSB_Vx = (Vx >> 7) & 0x01;

    if (MSB_Vx == 1)
    {
        registers[15] = 1;
    }
    else
    {
        registers[15] = 0;
    }
    registers[Vx] = registers[Vx] << 1;
}

// SNE Vx, Vy
void Chip8::OP_9xy0()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    if (registers[Vx] != registers[Vy])
    {
        pc = pc + 2;
    }
}

// LD I, address
void Chip8::OP_Annn()
{
    uint16_t addr = opcode & 0x0FFFu;
    index = addr;
}

// JP V0, address
void Chip8::OP_Bnnn()
{

    uint16_t addr = opcode & 0x0FFFu;

    pc = registers[0] + addr;
}

// RND Vx, byte

void Chip8::OP_Cxkk()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t byte = (opcode & 0x00FFu);

    registers[Vx] = randByte(randGen) & byte;
}
// DRW Vx, Vy, height
void Chip8::OP_Dxyn()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;
    uint8_t height = (opcode & 0x000Fu);

    // Wrap if going beyond screen boundaries
    uint8_t xPos = registers[Vx] % VIDEO_WIDTH;
    uint8_t yPos = registers[Vy] % VIDEO_HEIGHT;
    registers[15] = 0;
    for (unsigned int row = 0; row < height; row++)
    {
        uint8_t spriteByte = memory[index + row];

        for (unsigned int col = 0; col < 8; col++)
        {
            uint8_t spritePixel = spriteByte & (0x80u >> col); // specifically extract the col bit in the row byte
            uint32_t *screenPixel = &video[(yPos + row) * VIDEO_HEIGHT + (xPos + col)];

            if (spritePixel)
            {

                if (*screenPixel == 0xFFFFFFFF)
                {
                    registers[15] = 1;
                }
                *screenPixel ^= 0xFFFFFFFF;
            }
        }
    }
}

// SKP Vx

void Chip8::OP_Ex9E()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    if (inputKeys[registers[Vx]])
    {
        pc = pc + 2;
    }
}

// SKNP Vx
// SKNP Vx
void Chip8::OP_ExA1()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    if (!inputKeys[registers[Vx]])
    {
        pc = pc + 2;
    }
}

// LD Vx, DT
void Chip8::OP_Fx07()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    registers[Vx] = delayTimer;
}

// LD Vx, K
// The easiest way to “wait” is to decrement the PC by 2 whenever a keypad
// value is not detected. This has the effect of running the same instruction repeatedly.

void Chip8::OP_Fx0A()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    if (inputKeys[0])
    {
        registers[Vx] = 0;
    }
    else if (inputKeys[1])
    {
        registers[Vx] = 1;
    }
    else if (inputKeys[2])
    {
        registers[Vx] = 2;
    }
    else if (inputKeys[3])
    {
        registers[Vx] = 3;
    }
    else if (inputKeys[4])
    {
        registers[Vx] = 4;
    }
    else if (inputKeys[5])
    {
        registers[Vx] = 5;
    }
    else if (inputKeys[6])
    {
        registers[Vx] = 6;
    }
    else if (inputKeys[7])
    {
        registers[Vx] = 7;
    }
    else if (inputKeys[8])
    {
        registers[Vx] = 8;
    }
    else if (inputKeys[9])
    {
        registers[Vx] = 9;
    }
    else if (inputKeys[10])
    {
        registers[Vx] = 10;
    }
    else if (inputKeys[11])
    {
        registers[Vx] = 11;
    }
    else if (inputKeys[12])
    {
        registers[Vx] = 12;
    }
    else if (inputKeys[13])
    {
        registers[Vx] = 13;
    }
    else if (inputKeys[14])
    {
        registers[Vx] = 14;
    }
    else if (inputKeys[15])
    {
        registers[Vx] = 15;
    }
    else
    {
        pc -= 2;
    }
}

// LD DT, Vx
void Chip8::OP_Fx15()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    delayTimer = registers[Vx];
}

// LD ST, Vx
void Chip8::OP_Fx18()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    soundTimer = registers[Vx];
}

// ADD I, Vx
void Chip8::OP_Fx1E()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    index = index + registers[Vx];
}

// LD F, Vx
// We know the font characters are located at 0x50, and we know they’re
//  five bytes each, so we can get the address of the first byte of any character by taking an offset from the start address.

//
void Chip8::OP_Fx29()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t digit = registers[Vx];

    index = FONTSET_START_ADDRESS + (digit * 5);
}

// LD B, Vx
void Chip8::OP_Fx33()
{
    // Extract Vx from the opcode for decimal interpretation
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    // Get the decimal value of Vx
    uint8_t decimalValue = registers[Vx];
    // Extract the hundreds digit
    // Ones-place
    memory[index + 2] = decimalValue % 10;
    decimalValue /= 10;

    // Tens-place
    memory[index + 1] = decimalValue % 10;
    decimalValue /= 10;

    // Hundreds-place
    memory[index] = decimalValue % 10;
}

// LD [I], Vx
void Chip8::OP_Fx55()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    for (int i = 0; i <= Vx; i++)
    {
        memory[index + i] = registers[i];
    }
}

// LD Vx, [I]
void Chip8::OP_Fx65()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    for (int i = 0; i <= Vx; i++)
    {
        registers[i] = memory[index + i];
    }
}
