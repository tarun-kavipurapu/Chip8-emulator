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
{
    // Load fonts into memory
    unsigned int size = FONTSET_SIZE;

    for (unsigned int i = 0; i < FONTSET_SIZE; ++i)
    {
        memory[FONTSET_START_ADDRESS + i] = fontset[i];
    }
    pc = START_ADDRESS;
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
}

// JP V0, address
void Chip8::OP_Bnnn()
{
}

// RND Vx, byte
void Chip8::OP_Cxkk()
{
}

// DRW Vx, Vy, height
void Chip8::OP_Dxyn()
{
}

// SKP Vx
void Chip8::OP_Ex9E()
{
}

// SKNP Vx
void Chip8::OP_ExA1()
{
}

// LD Vx, DT
void Chip8::OP_Fx07()
{
}

// LD Vx, K
void Chip8::OP_Fx0A()
{
}

// LD DT, Vx
void Chip8::OP_Fx15()
{
}

// LD ST, Vx
void Chip8::OP_Fx18()
{
}

// ADD I, Vx
void Chip8::OP_Fx1E()
{
}

// LD F, Vx
void Chip8::OP_Fx29()
{
}

// LD B, Vx
void Chip8::OP_Fx33()
{
}

// LD [I], Vx
void Chip8::OP_Fx55()
{
}

// LD Vx, [I]
void Chip8::OP_Fx65()
{
}
