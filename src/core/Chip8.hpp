//
// Created by abel on 28/01/2020.
//

#ifndef NESEMULATOR_CHIP8_HPP
#define NESEMULATOR_CHIP8_HPP

#include <SFML/Graphics.hpp>
#include <string>
#include <map>

#define FONTSET_SIZE 80

const unsigned char fontset[FONTSET_SIZE] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0,		// 0
        0x20, 0x60, 0x20, 0x20, 0x70,		// 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0,		// 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0,		// 3
        0x90, 0x90, 0xF0, 0x10, 0x10,		// 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0,		// 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0,		// 6
        0xF0, 0x10, 0x20, 0x40, 0x40,		// 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0,		// 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0,		// 9
        0xF0, 0x90, 0xF0, 0x90, 0x90,		// A
        0xE0, 0x90, 0xE0, 0x90, 0xE0,		// B
        0xF0, 0x80, 0x80, 0x80, 0xF0,		// C
        0xE0, 0x90, 0x90, 0x90, 0xE0,		// D
        0xF0, 0x80, 0xF0, 0x80, 0xF0,		// E
        0xF0, 0x80, 0xF0, 0x80, 0x80		// F
};


enum OpCode {
    CLEAR_SCREEN,
    RETURN,
    GOTO,
    SUBR_CALL,
    JMP_EQ,
    JMP_NEQ,
    JMP_EQ_REG,
    SET_VAL,
    ADD_VAL,
    SET_REG,
    OR,
    AND,
    XOR,
    ADD_REG,
    SUB_REG,
    RSHIFT_REG,
    SUB_REG_BIS,
    LSHIFT_REG,
    JMP_NEQ_REG,
    SET_I,
    JMP_TO,
    SET_REG_RAND,
    DRAW_SPRITE,
    JMP_KEY_PRESSED,
    JMP_NKEY_PRESSED,
    GET_DELAY,
    GET_KEY,
    SET_DELAY_TMR,
    SET_SOUND_TMR,
    ADD_I,
    SET_I_CHAR,
    STORES_BINARY,
    REG_DUMP,
    REG_LOAD,
    CALL
};

class Chip8 {
    public:
        Chip8();
        explicit Chip8(const std::string &filePath);
        void resetMemory();
        void runGame();
        void executeOpCode();
    private:
        static int getMSB(int nb);
        void jump();
        void clearScreen();
        void subroutine_return();
        void subroutine_call();
        void loadFile(const std::string &filePath);
        void fetchOpCode();
        void jump_eq();
        void jump_neq();
        void jump_eq_reg();
        void set_val();
        void add_val();
        void set_reg();
        void or_op();
        void and_op();
        void xor_op();
        void add_reg();
        void sub_reg();
        void rshift_reg();
        void sub_reg_bis();
        void lshift_reg();
        void jump_neq_reg();
        void set_i();
        void jump_to();
        void set_reg_rand();
        void draw_sprite();
        void jump_key_pressed();
        void jump_nkey_pressed();
        void get_delay();
        void get_key();
        void set_delay();
        void set_sound();
        void getInstruction();
        void add_i();
        void set_i_char();
        void store_binary();
        void reg_dump();
        void reg_load();
        void draw();
        void mapPixels();
        void pixelsLol();
        void updateKeyMap();
        void keyLol();
        unsigned short opcode{};
        unsigned char memory[4096]{};
        unsigned char reg[16]{};
        unsigned short indexRegister{};
        unsigned short programCounter{};
        unsigned char pixels[64 * 32]{};
        unsigned char delayTimer{};
        unsigned char soundTimer{};
        unsigned short stack[16]{};
        unsigned short stackPtr{};
        unsigned char key[16]{};
        bool isGameStarted = false;
        bool drawFlag = false;
        bool keyPressed[16] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};
        OpCode actualInstruction = CLEAR_SCREEN;
        sf::RenderWindow window;
        sf::Texture texture;
        sf::Sprite sprite;
        sf::Uint8 *graphicsPixels = nullptr;
        std::map<OpCode, void(Chip8::*)(void)> opCodeMap = {
                {CLEAR_SCREEN, &Chip8::clearScreen},
                {RETURN, &Chip8::subroutine_return},
                {GOTO, &Chip8::jump},
                {SUBR_CALL, &Chip8::subroutine_call},
                {JMP_EQ, &Chip8::jump_eq},
                {JMP_NEQ, &Chip8::jump_neq},
                {JMP_EQ_REG, &Chip8::jump_eq_reg},
                {SET_VAL, &Chip8::set_val},
                {ADD_VAL, &Chip8::add_val},
                {SET_REG, &Chip8::set_reg},
                {OR, &Chip8::or_op},
                {AND, &Chip8::and_op},
                {XOR, &Chip8::xor_op},
                {ADD_REG, &Chip8::add_reg},
                {SUB_REG, &Chip8::sub_reg},
                {RSHIFT_REG, &Chip8::rshift_reg},
                {SUB_REG_BIS, &Chip8::sub_reg_bis},
                {LSHIFT_REG, &Chip8::lshift_reg},
                {JMP_NEQ_REG, &Chip8::jump_neq_reg},
                {SET_I, &Chip8::set_i},
                {JMP_TO, &Chip8::jump_to},
                {SET_REG_RAND, &Chip8::set_reg_rand},
                {DRAW_SPRITE, &Chip8::draw_sprite},
                {JMP_KEY_PRESSED, &Chip8::jump_key_pressed},
                {JMP_NKEY_PRESSED, &Chip8::jump_nkey_pressed},
                {GET_DELAY, &Chip8::get_delay},
                {GET_KEY, &Chip8::get_key},
                {SET_DELAY_TMR, &Chip8::set_delay},
                {SET_SOUND_TMR, &Chip8::set_sound},
                {ADD_I, &Chip8::add_i},
                {SET_I_CHAR, &Chip8::set_i_char},
                {STORES_BINARY, &Chip8::store_binary},
                {REG_DUMP, &Chip8::reg_dump},
                {REG_LOAD, &Chip8::reg_load}

        };
    std::string opCodeNames[CALL + 1] =
            {
                    "clearScreen",
                    "return",
                    "goto",
                    "subr_call",
                    "jmp_eq",
                    "jmp_neq",
                    "jmp_eq_reg",
                    "set_val",
                    "add_val",
                    "set_reg",
                    "or",
                    "and",
                    "xor",
                    "add_reg",
                    "sub_reg",
                    "rshift_reg",
                    "sub_reg_bis",
                    "lshift_reg",
                    "jmp_neq_reg",
                    "set_i",
                    "jmp_to",
                    "set_reg_rand",
                    "draw_sprite",
                    "jmp_key_press",
                    "jmp_nkey_press",
                    "get_delay",
                    "get_key",
                    "set_delay_tmr",
                    "set_snd_tmr",
                    "add_i",
                    "set_i_char",
                    "store_binary",
                    "reg_dump",
                    "reg_load",
                    "call",
            };
};


#endif //NESEMULATOR_CHIP8_HPP
