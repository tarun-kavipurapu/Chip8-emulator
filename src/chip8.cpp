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

Chip8::Chip8()
    : randGen(std::chrono::system_clock::now().time_since_epoch().count()),
      randByte(0, 255U)
{
    // Load fonts into memory
    unsigned int size = FONTSET_SIZE;

    for (unsigned int i = 0; i < FONTSET_SIZE; ++i)
    {
        memory[FONTSET_START_ADDRESS + i] = fontset[i];
    }
    pc = START_ADDRESS;
}
Chip8::Chip8()
{
    srand(static_cast<unsigned int>(std::time(nullptr)));
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
    uint8_t Vx = (opcode & 0xFFFu) >> 8u;
    uint8_t byte = opcode & 0x0FFu;
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
    registers[Vx] = registers[Vy] - registers[Vy];
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

    registers[byte] = randByte(randGen) & byte;
}
// DRW Vx, Vy, height
void Chip8::OP_Dxyn()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;
    uint8_t
}

// SKP Vx
{
    void Chip8::OP_Ex9E()
    {
        uint8_t Vx = (opcode & 0x0F00u) >> 8u;

        if (inputKeys[registers[Vx]])
        {
            pc = pc + 2;
        }
    }

    // SKNP Vx
    void Chip8::OP_ExA1()
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
    sp = registers[Vx];
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
