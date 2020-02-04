//
// Created by abel on 28/01/2020.
//

#include <functional>
#include <fstream>
#include <cstring>
#include <unistd.h>
#include <iostream>
#include "Chip8.hpp"

Chip8::Chip8(const std::string &filePath) : window(sf::VideoMode(800, 600, 32), sf::String("chip8"))
{
    texture.create(64, 32);
    graphicsPixels = new sf::Uint8[64 * 32 * 4];
    sprite.setTexture(texture);
    sprite.setScale({10.0, 10.0});
    sprite.setPosition({0.0, 0.0});
    resetMemory();
    mapPixels();
    loadFile(filePath);
}

void Chip8::loadFile(const std::string &filePath)
{
    std::ifstream inFile(filePath);
    std::streampos fileSize;

    inFile.seekg(0, std::ios::end);
    fileSize = inFile.tellg();
    inFile.seekg(0, std::ios::beg);
    inFile.read(reinterpret_cast<char *>(memory) + 0x200, fileSize);
}

void Chip8::resetMemory()
{
    opcode = 0x00;
    std::memset(memory, 0x00, sizeof(memory));
    std::memset(reg, 0x00, sizeof(reg));
    indexRegister = 0x00;
    programCounter = 0x200;
    std::memset(pixels, 0x00, sizeof(pixels));
    delayTimer = 0x00;
    soundTimer = 0x00;
    std::memset(stack, 0x00, sizeof(stack));
    stackPtr = 0x00;
    std::memset(key, 0x00, sizeof(key));
    for (int i = 0; i < 80; ++i)
        memory[i] = fontset[i];
}

Chip8::Chip8()
{
    resetMemory();
}

void Chip8::runGame()
{
    isGameStarted = true;

    while (isGameStarted && window.isOpen()) {
        if (drawFlag) {
            this->draw();
//            std::cout << graphicsPixels << std::endl;
            drawFlag = false;
        }
        updateKeyMap();
        keyLol();
        fetchOpCode();
        getInstruction();
        executeOpCode();
        if (delayTimer > 0)
            delayTimer--;
        if (soundTimer > 0)
            soundTimer--;
        usleep(1666);
        std::cout << opCodeNames[actualInstruction] << std::endl;
    }
}

void Chip8::fetchOpCode()
{
    opcode = memory[programCounter] << 8 | memory[programCounter + 1];
}

void Chip8::executeOpCode()
{
    std::invoke(this->opCodeMap[actualInstruction], *this);
}

void Chip8::clearScreen() {
    std::memset(pixels, 0x00, sizeof(pixels));
    drawFlag = true;
    programCounter += 2;
}

void Chip8::subroutine_return() {
    --stackPtr;
    programCounter = this->stack[stackPtr] + 2;
}

void Chip8::jump()
{
    programCounter = opcode & 0x0FFF;
}

void Chip8::subroutine_call()
{
    stack[stackPtr] = programCounter;
    stackPtr++;
    programCounter = opcode & 0x0FFF;
}

void Chip8::jump_eq()
{
    short x = (opcode & 0x0F00) >> 8;
    unsigned char n = opcode & 0x00FF;

    if (reg[x] == n)
        programCounter += 2;
    programCounter += 2;
}

void Chip8::jump_neq()
{
    short x = (opcode & 0x0F00) >> 8;
    unsigned char n = opcode & 0x00FF;

    if (reg[x] != n)
        programCounter += 2;
    programCounter += 2;
}

void Chip8::jump_eq_reg()
{
    short x = (opcode & 0x0F00) >> 8;
    short y = (opcode & 0x00F0) >> 4;

    if (reg[x] == reg[y])
        programCounter += 2;
    programCounter += 2;
}

void Chip8::set_val()
{
    short x = (opcode & 0x0F00) >> 8;
    unsigned char n = opcode & 0x00FF;

    reg[x] = n;
    programCounter += 2;
}

void Chip8::add_val()
{
    short x = (opcode & 0x0F00) >> 8;
    unsigned char n = opcode & 0x00FF;

    reg[x] += n;
    programCounter += 2;
}

void Chip8::set_reg()
{
    short x = (opcode & 0x0F00) >> 8;
    short y = (opcode & 0x00F0) >> 4;

    reg[x] = reg[y];
    programCounter += 2;
}

void Chip8::or_op()
{
    short x = (opcode & 0x0F00) >> 8;
    short y = (opcode & 0x00F0) >> 4;

    reg[x] |= reg[y];
    programCounter += 2;
}

void Chip8::and_op()
{
    short x = (opcode & 0x0F00) >> 8;
    short y = (opcode & 0x00F0) >> 4;

    reg[x] &= reg[y];
    programCounter += 2;
}

void Chip8::xor_op()
{
    short x = (opcode & 0x0F00) >> 8;
    short y = (opcode & 0x00F0) >> 4;

    reg[x] ^= reg[y];
    programCounter += 2;
}

void Chip8::add_reg()
{
    short x = (opcode & 0x0F00) >> 8;
    short y = (opcode & 0x00F0) >> 4;

    if (reg[y] > 0xFF + reg[x])
        reg[0xF] = 1;
    else
        reg[0xF] = 0;
    reg[x] += reg[y];
    programCounter += 2;
}

void Chip8::sub_reg()
{
    short x = (opcode & 0x0F00) >> 8;
    short y = (opcode & 0x00F0) >> 4;

    if (reg[y] > reg[x])
        reg[0xF] = 0;
    else
        reg[0xF] = 1;
    reg[x] -= reg[y];
    programCounter += 2;
}

void Chip8::rshift_reg()
{
    short x = (opcode & 0x0F00) >> 8;

    reg[0xF] = reg[x] & 1;
    reg[x] >>= 1;
    programCounter += 2;
}

void Chip8::sub_reg_bis()
{
    short x = (opcode & 0x0F00) >> 8;
    short y = (opcode & 0x00F0) >> 4;

    if (reg[x] > reg[y])
        reg[0xF] = 0;
    else
        reg[0xF] = 1;
    reg[x] = reg[y] - reg[x];
    programCounter += 2;
}

void Chip8::lshift_reg()
{
    short x = (opcode & 0x0F00) >> 8;

    reg[0xF] = getMSB(reg[x]);
    reg[x] <<= 1;
    programCounter += 2;
}

int Chip8::getMSB(int nb)
{
    if (nb == 0)
        return 0;

    int msb = 0;
    while (nb != 0) {
        nb = nb / 2;
        msb++;
    }

    return 1 << msb;
}

void Chip8::jump_neq_reg()
{
    short x = (opcode & 0x0F00) >> 8;
    short y = (opcode & 0x00F0) >> 4;

    if (reg[x] != reg[y])
        programCounter += 2;
    programCounter += 2;
}

void Chip8::set_i()
{
    indexRegister = opcode & 0x0FFF;
    programCounter += 2;
}

void Chip8::jump_to()
{
    programCounter = reg[0] + (opcode & 0x0FFF);
}

void Chip8::set_reg_rand()
{
    //Have not energy to use c++11 random library ....
    unsigned char val = std::rand() % 0xFF;
    unsigned char n = (opcode & 0x00FF);
    short x = (opcode & 0x0F00) >> 8;

    reg[x] = val & n;
    programCounter += 2;
}

void Chip8::draw_sprite()
{
    unsigned short vx = reg[(opcode & 0x0F00) >> 8];
    unsigned short vy = reg[(opcode & 0x00F0) >> 4];
    unsigned short height = opcode & 0x000F;
    unsigned short pixel;
    reg[0xF] = 0;

    for (int yLine = 0; yLine < height; yLine++)
    {
        pixel = memory[indexRegister + yLine];
        for(int xLine = 0; xLine < 8; xLine++)
        {
            if((pixel & (0x80 >> xLine)) != 0)
            {
                if(pixels[(vx + xLine + ((vy + yLine) * 64))] == 1)
                    reg[0xF] = 1;
                pixels[vx + xLine + ((vy + yLine) * 64)] ^= 1;
                if (pixels[vx + xLine + ((vy + yLine) * 64)] != 0)
                    std::cout << "la bite" << std::endl;
            }
        }
    }
    drawFlag = true;
    programCounter += 2;
}

void Chip8::jump_key_pressed()
{
    short x = (opcode & 0x0F00) >> 8;

    if (keyPressed[reg[x]])
        programCounter += 2;
    programCounter += 2;
}

void Chip8::jump_nkey_pressed()
{
    short x = (opcode & 0x0F00) >> 8;

    if (!keyPressed[reg[x]])
        programCounter += 2;
    programCounter += 2;
}

void Chip8::get_delay()
{
    short x = (opcode & 0x0F00) >> 8;

    reg[x] = delayTimer;
    programCounter += 2;
}

void Chip8::get_key()
{
    short x = (opcode & 0x0F00) >> 8;

    //TODO: THIS
    //REG[X] = BLOCKING_GET_KEY();
    programCounter += 2;
}

void Chip8::set_delay()
{
    short x = (opcode & 0x0F00) >> 8;

    delayTimer = reg[x];
    programCounter += 2;
}

void Chip8::set_sound()
{
    short x = (opcode & 0x0F00) >> 8;

    soundTimer = reg[x];
    programCounter += 2;
}

void Chip8::add_i()
{
    short x = (opcode & 0x0F00) >> 8;

    indexRegister += reg[x];
    programCounter += 2;
}

void Chip8::set_i_char()
{
    short x = (opcode & 0x0F00) >> 8;

    indexRegister = reg[x] * 5;
    programCounter += 2;
}

void Chip8::store_binary()
{
    short x = (opcode & 0x0F00) >> 8;

    memory[indexRegister]     = reg[x] / 100;
    memory[indexRegister + 1] = (reg[x] / 10) % 10;
    memory[indexRegister + 2] = (reg[x] % 100) % 10;
    programCounter += 2;
}

void Chip8::reg_dump()
{
    short x = (opcode & 0x0F00) >> 8;

    for (int i = 0; i <= x; i++)
        memory[indexRegister + i] = reg[i];
    programCounter += 2;
}

void Chip8::reg_load()
{
    short x = (opcode & 0x0F00) >> 8;

    for (int i = 0; i <= x; i++)
        reg[i] = memory[indexRegister + i];
    programCounter += 2;
}

void Chip8::getInstruction()
{
    switch (opcode & 0xF000) {
        case 0x0000:
            switch (opcode & 0x000F) {
                case 0x0000:
                    actualInstruction = CLEAR_SCREEN;
                    break;
                case 0x000E:
                    actualInstruction = RETURN;
                    break;
                default:
                    printf ("Unknown opcode [0x0000]: 0x%X\n", opcode);
            }
            break;
        case 0x1000:
            actualInstruction = GOTO;
            break;
        case 0x2000:
            actualInstruction = SUBR_CALL;
            break;
        case 0x3000:
            actualInstruction = JMP_EQ;
            break;
        case 0x4000:
            actualInstruction = JMP_NEQ;
            break;
        case 0x5000:
            actualInstruction = JMP_EQ_REG;
            break;
        case 0x6000:
            actualInstruction = SET_VAL;
            break;
        case 0x7000:
            actualInstruction = ADD_VAL;
            break;
        case 0x8000:
            switch (opcode & 0x000F) {
                case 0x0000:
                    actualInstruction = SET_REG;
                    break;
                case 0x0001:
                    actualInstruction = OR;
                    break;
                case 0x0002:
                    actualInstruction = AND;
                    break;
                case 0x0003:
                    actualInstruction = XOR;
                    break;
                case 0x0004:
                    actualInstruction = ADD_REG;
                    break;
                case 0x0005:
                    actualInstruction = SUB_REG;
                    break;
                case 0x0006:
                    actualInstruction = RSHIFT_REG;
                    break;
                case 0x0007:
                    actualInstruction = SUB_REG_BIS;
                    break;
                case 0x000E:
                    actualInstruction = LSHIFT_REG;
                    break;
                default:
                    printf ("Unknown opcode [0x0000]: 0x%X\n", opcode);
            }
            break;
        case 0x9000:
            actualInstruction = JMP_NEQ_REG;
            break;
        case 0xA000:
            actualInstruction = SET_I;
            break;
        case 0xB000:
            actualInstruction = JMP_TO;
            break;
        case 0xC000:
            actualInstruction = SET_REG_RAND;
            break;
        case 0xD000:
            actualInstruction = DRAW_SPRITE;
            break;
        case 0xE000:
            switch (opcode & 0x000F) {
                case 0x000E:
                    actualInstruction = JMP_KEY_PRESSED;
                    break;
                case 0x0001:
                    actualInstruction = JMP_NKEY_PRESSED;
                    break;
                default:
                    printf ("Unknown opcode [0x0000]: 0x%X\n", opcode);
            }
            break;
        case 0xF000:
            switch (opcode & 0x000F) {
                case 0x0007:
                    actualInstruction = GET_DELAY;
                    break;
                case 0x000A:
                    actualInstruction = GET_KEY;
                    break;
                case 0x0005:
                    switch (opcode & 0x00F0) {
                        case 0x0010:
                            actualInstruction = SET_DELAY_TMR;
                            break;
                        case 0x0050:
                            actualInstruction = REG_DUMP;
                            break;
                        case 0x0060:
                            actualInstruction = REG_LOAD;
                            break;
                        default:
                            printf ("Unknown opcode [0x0000]: 0x%X\n", opcode);
                    }
                    break;
                case 0x0008:
                    actualInstruction = SET_SOUND_TMR;
                    break;
                case 0x000E:
                    actualInstruction = ADD_I;
                    break;
                case 0x0009:
                    actualInstruction = SET_I_CHAR;
                    break;
                case 0x0003:
                    actualInstruction = STORES_BINARY;
                    break;
                default:
                    printf ("Unknown opcode [0x0000]: 0x%X\n", opcode);
            }
            break;
        default:
            printf ("Unknown opcode [0x0000]: 0x%X\n", opcode);
    }
}

void Chip8::draw()
{
    mapPixels();
    window.clear(sf::Color::Black);
    window.draw(sprite);
    window.display();
}

void Chip8::mapPixels()
{
    for (int i = 0; i < 64 * 32; i++)
    {
        graphicsPixels[i*4] = pixels[i] == 0 ? 0 : 255;
        graphicsPixels[i*4 + 1] = pixels[i] == 0 ? 0 : 255;
        graphicsPixels[i*4 + 2] = pixels[i] == 0 ? 0 : 255;
        graphicsPixels[i*4 + 3] = 255; //no opacity
    }
    texture.update(graphicsPixels);
//    sprite.setTexture(texture);
}

void Chip8::pixelsLol()
{
    for (int i = 0; i < 64; i++)
    {
        for (int j = 0; j < 32; j++)
        {
            std::cout << (int)pixels[i*32 +j]<< " ";
        }
        std::cout << std::endl;
    }
}

void Chip8::updateKeyMap()
{
    keyPressed[0x0] = sf::Keyboard::isKeyPressed(sf::Keyboard::X);
    keyPressed[0x1] = sf::Keyboard::isKeyPressed(sf::Keyboard::Num1);
    keyPressed[0x2] = sf::Keyboard::isKeyPressed(sf::Keyboard::Num2);
    keyPressed[0x3] = sf::Keyboard::isKeyPressed(sf::Keyboard::Num3);
    keyPressed[0x4] = sf::Keyboard::isKeyPressed(sf::Keyboard::A);
    keyPressed[0x5] = sf::Keyboard::isKeyPressed(sf::Keyboard::Z);
    keyPressed[0x6] = sf::Keyboard::isKeyPressed(sf::Keyboard::E);
    keyPressed[0x7] = sf::Keyboard::isKeyPressed(sf::Keyboard::Q);
    keyPressed[0x8] = sf::Keyboard::isKeyPressed(sf::Keyboard::S);
    keyPressed[0x9] = sf::Keyboard::isKeyPressed(sf::Keyboard::D);
    keyPressed[0xA] = sf::Keyboard::isKeyPressed(sf::Keyboard::W);
    keyPressed[0xB] = sf::Keyboard::isKeyPressed(sf::Keyboard::C);
    keyPressed[0xC] = sf::Keyboard::isKeyPressed(sf::Keyboard::Num4);
    keyPressed[0xD] = sf::Keyboard::isKeyPressed(sf::Keyboard::R);
    keyPressed[0xE] = sf::Keyboard::isKeyPressed(sf::Keyboard::F);
    keyPressed[0xF] = sf::Keyboard::isKeyPressed(sf::Keyboard::V);
}

void Chip8::keyLol()
{
    for (int i = 0; i <= 0xF; ++i)
    {
        std::cout << key[i] << " ";
    }
    std::cout << std::endl;
}
