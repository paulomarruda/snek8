/**
* @file cpu.h
* @author Paulo Arruda
* @license MIT
* @brief Declaration of the Chip8's CPU and related emulation routines.
*
* @note For the implementation, we follow the following references:
*     - Cowgod's Chip-8 Technical Reference v1.0. Accessed at
*
*           `http://devernay.free.fr/hacks/chip8/C8TECH10.HTM`
*
*     - Guide to making a CHIP-8 emulator by Tobias Langhoff. Accessed at
*
*           `https://tobiasvl.github.io/blog/write-a-chip-8-emulator/`
*
* Diferent Chip8 implementations exist regarding the following instructions:
*     - SHR Vx {, Vy}
*     - SHL Vx {, Vy}
      - JP V0x0, 0x0NNN or JP V0xX, 0x0NNN
*     - LD [I], Vx
*     - LD Vx, [I]
*
* In the original COSMAC-VIP, these instructions do the following:
*     - 0x8XY6: Shift-right the value held in VY by 1 and load the result in VX.
*     - 0x8XYE: Shift-left the value held in VY by 1 and load the result in VX.
*     - 0xBNNN: Jumps to the memory address 0x0NNN + V0.
*
* In modern implementations, these instructions do the following:
*
*/
#ifndef PY8_CPU_H
    #define PY8_CPU_H
#ifdef __cplusplus
    extern "C"{
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#define PY8_SIZE_KEYSET             16
#define PY8_SIZE_STACK              16
#define PY8_SIZE_REGISTERS          16
#define PY8_SIZE_RAM                4096
#define PY8_SIZE_MAX_ROM_FILE       3584
#define PY8_GRAPHICS_WIDTH          64
#define PY8_GRAPHICS_HEIGTH         32
#define PY8_SIZE_GRAPHICS           2048
#define PY8_MEM_ADDR_PROG_START     0x0200
#define PY8_SIZE_FONTSET_PIXEL      80
#define PY8_SIZE_FONTSET_SPRITE     5
#define PY8_MEM_ADDR_FONTSET_START  0x50

#define UNUSED(x) ((void) x)
#define CAST_PTR(type, ptr) ((type*) ptr)

/**
* @brief Controls whether the emulation step was successeful.
*/
enum Py8ExecutionOutput{
    PY8_EXECOUT_SUCCESS,
    PY8_EXECOUT_INVALID_OPCODE,
    PY8_EXECOUT_STACK_EMPTY,
    PY8_EXECOUT_STACK_OVERFLOW,
    PY8_EXECOUT_MEM_ADDR_OUT_OF_BOUNDS,
    PY8_EXECOUT_ROM_FILE_NOT_FOUND,
    PY8_EXECOUT_ROM_FILE_FAILED_TO_OPEN,
    PY8_EXECOUT_ROM_FILE_FAILED_TO_READ,
    PY8_EXECOUT_ROM_FILE_EXCEEDS_MAX_MEM,
    PY8_EXECOUT_EMPTY_STRUCT,
};

/**
* @brief Controls which implementation to use.
*/
enum Py8ImplmMode{
    PY8_IMPLM_MODE_COSMAC_VIP,
    PY8_IMPLM_MODE_MODERN,
};

/**
* @brief Declaration of Chip8's opcode structure as a bitfield.
*
* Chip8's opcode consists of a hexadecimal 16-bit undigned integer that
* we break as follows:
*
*                     0x X Y Z W
*                        ^ ^ ^ ^
*                        | | | |_____ the least significant quarter.
*                        | | |_______ the information quarter 1.
*                        | |_________ the information quarter 2.
*                        |___________ the information quarter 3.
*/
typedef struct{
    uint8_t lsq: 4;
    uint8_t iq1: 4;
    uint8_t iq2: 4;
    uint8_t msq: 4;
} Py8Opcode;

/**
* @brief Create a new opcode based on its integer value.
*
* @param[in] `code`.
* @return A new opcode representation of the integer `code`.
*/
Py8Opcode
py8_opcodeInit(uint16_t code);

/**
* @brief Retrieves the rightmost first 12 bits from the `opcode`, widely used throught
* the implementation to point to memory adresses.
*
*                     0x X N N N
*                         |_____|
*                            |_____ the address.
*
* @param[in] `opcode`
* @return the integer representation of the first 12 bits of the `opcode`.
*/
uint16_t
py8_opcodeGetAddr(Py8Opcode opcode);

/**
* @brief Retrieves the rightmost byte from the `opcode`.
*
*                     0x X Y K K
*                           |___|
*                             |____ the byte.
* @param[in] `opcode`.
* @return A 8-bit unsigned integer represention of the rightmost byte from the
* `opcode`.
*/
uint8_t
py8_opcodeGetByte(Py8Opcode code);

/**
* @brief Implementation of the Chip8's CPU.
*
* @param `registers` Chip8's 16 8-bit registers ranging from 0x0 to 0xF.
* @param `pc` Chip8's program counter.
* @param `sp` Chip8's stack pointer.
* @param `ir` Chip8's 16-bit index register.
* @param `st` Chip8's 8-bit sound timer register.
* @param `dt` Chip8's 8-bit delay timer register.
* @param `stack` Chip8's 16-level 16-bit stack.
* @param `memory` Chip8's memory.
* @param `keys` Chip8's 16 key set. Each bit represent a key that is either pressed or released.
* @param `graphics` Chip8's display implementation.
* @param `mode` Controls which Chip8 implementation to follow.
*/
typedef struct{
    uint8_t memory[PY8_SIZE_RAM];
    uint8_t graphics[PY8_SIZE_GRAPHICS];
    uint8_t registers[PY8_SIZE_REGISTERS];
    uint16_t stack[PY8_SIZE_STACK];
    enum Py8ImplmMode mode;
    uint16_t keys;
    uint16_t pc;
    uint16_t ir;
    uint8_t sp;
    uint8_t st;
    uint8_t dt;
} Py8CPU;

/**
* @brief Set the CPU field members to their initial values.
*
* @param[in, out] `cpu`.
* @param[in] `mode` Determines which CHIP8 implementation to use.
* @return PY8_EXECOUT_SUCCESS.
*/
enum Py8ExecutionOutput
py8_cpuInit(Py8CPU* cpu, enum Py8ImplmMode mode);

/**
* @brief Reads and copy a given CHIP8 ROM into memory.
*
* @param[in, out] `cpu`.
* @param[in] `rom_file_path`.
* @return A code representation on whether the execution was sucesseful. The following
* error codes, with their repsective motives, can be returned:
* - `PY8_EXECOUT_ROM_FILE_NOT_FOUND`, if the file was not found.
* - `PY8_EXECOUT_ROM_FILE_FAILED_TO_OPEN`, the system was unable to read the file.
* - `FAILED_TO_DETERMINE_CHIP8_ROM_FILE_SIZE`, the system failed to determine the file size.
* - `CHIP8_ROM_SIZE_EXCEEDS_MAX_MEM`, the ROM size exceeds CHIP8's RAM capacity.
* - `FAILED_TO_READ_CHIP8_ROM_FILE`, the system failed to read the contents of the ROM file.
*/
enum Py8ExecutionOutput
py8_cpuLoadRom(Py8CPU* cpu, const char* rom_file_path);

enum Py8ExecutionOutput
py8_cpuSetKey(Py8CPU* cpu, size_t key, bool value);

inline bool
py8_cpuGetKeyVal(Py8CPU cpu, size_t key){
    return (cpu.keys & (1 << key)) == 1;
}

/*
* @brief Function pointer representing the exectution of an opcode.
*/
typedef enum Py8ExecutionOutput (*Py8InstructionExec)(Py8CPU* cpu, Py8Opcode opcode, char* code);

/*
* @brief Representation of a Chip8's instruction.
*
* @param `code` The string representation of the instruction, for desassembly purposes.
* @param `exec` The function pointer that executes the instruction on the cpu.
*/
typedef struct{
    char code[20];
    Py8InstructionExec exec;
} Py8Instruction;

/**
* @brief Retrieves the especified instruction from an opcode.
*
* @param[in] `opcode`.
* @return The instruction contained in the `opcode`.
*/
Py8Instruction
py8_getInstruction(Py8Opcode opcode);

/**
* @brief Execute a step in the emulation process.
* 
* A step is here defined as the retrieval and execution of a single opcode and
* the update of the timers.
*
* @param[in, out] cpu
* @param[out] instruction
* @return A code representation on whether the execution was sucesseful indicating,
* if not, the problem ocurred.
*/
enum Py8ExecutionOutput
py8_cpuEmulate(Py8CPU* cpu, Py8Instruction* instruction);

/**
* @brief The instruction representation of any instruction given by an invalid opcode.
*
* Code: NOP.
*
* @param `cpu`
* @param `opcode`
* @param `code`
* @return Always returns the error code `INVALID_CHIP8_OPCODE`.
*/
enum Py8ExecutionOutput
py8_cpuExecutionError(Py8CPU* cpu, Py8Opcode opcode, char* code);

/**
* @brief Clear the Chip8's screen.
* 
* Opcode: 0x00E0.
* Code: CLS.
*
* @param `cpu`
* @param `opcode`
* @param `code`
* @return Always returns `CHIP8_EXECUTION_SUCCESSEFUL`.
*/
enum Py8ExecutionOutput
py8_cpuCLS(Py8CPU* cpu, Py8Opcode opcode, char* code);

/**
* @brief Return from a subroutine.
* 
* Opcode: 0x00EE.
* Code: RET.
* 
* The interpreter decrements the stack pointer and sets the PC to the top of the
* stack.
*
* @param `cpu`
* @param `opcode`
* @param `code`
* @return A code representation on whether the execution was sucesseful. It will return
* the code `CHIP8_STACK_EMPTY` if it is called when the CHIP8's stack is empty.
*/
enum Py8ExecutionOutput
py8_cpuRET(Py8CPU* cpu, Py8Opcode opcode, char* code);

/**
* @brief Call a subroutine.
*
* Opcode: 0x2NNN.
* Code: CALL 0x0NNN
*
* The interpreter increments the stack pointer and sets the top of the stack to the address
* held in the PC; the PC is then set to the address 0x0NNN.
*
* @param `cpu`
* @param `opcode`
* @param `code`
* @return A code representation on whether the execution was sucesseful. It will return
* the code `CHIP8_STACK_OVERFLOW` if it is called when CHIP8's stack is full.
*/
enum Py8ExecutionOutput
py8_cpuCALL(Py8CPU* cpu, Py8Opcode opcode, char* code);

/**
* @brief Jump to the given address.
* 
* Opcode: 0x1NNN.
* Code: JP 0x0NNN.
*
* The interpreter sets the PC to 0x0NNN.
*
* @param `cpu`
* @param `opcode`
* @param `code`
* @return Always returns `CHIP8_EXECUTION_SUCCESSEFUL`.
*/
enum Py8ExecutionOutput
py8_cpuJMP_ADDR(Py8CPU* cpu, Py8Opcode opcode, char* code);

/**
* @brief Compares a register's value to a given byte.
*
* Opcode: 0x3XKK.
* Code: SE Vx, KK.
*
* The interpreter compares the value held in the register Vx to the byte 0xKK. If they
* are equal, the interpreter increments the PC.
*
* @param `cpu`
* @param `opcode`
* @param `code`
* @return Always returns `CHIP8_EXECUTION_SUCCESSEFUL`.
*/
enum Py8ExecutionOutput
py8_cpuSE_VX_BYTE(Py8CPU* cpu, Py8Opcode opcode, char* code);

/**
* @brief Compares a register's value to a given byte.
*
* Opcode: 0x4XKK.
* Code: SNE Vx, KK.
*
* The interpreter compares the value held in the register Vx to the byte KK. If they
* are not equal, the interpreter increments the PC.
*
* @param `cpu`
* @param `opcode`
* @param `code`
* @return Always returns `CHIP8_EXECUTION_SUCCESSEFUL`.
*/
enum Py8ExecutionOutput
py8_cpuSNE_VX_BYTE(Py8CPU* cpu, Py8Opcode opcode, char* code);

/**
* @brief Set the value of the register VX to the given byte.
*
* Opcode: 0x6XKK.
* Code: Code: LD Vx, KK.
*
* @param `cpu`
* @param `opcode`
* @param `code`
* @return Always returns `CHIP8_EXECUTION_SUCCESSEFUL`.
*/
enum Py8ExecutionOutput
py8_cpuLD_VX_BYTE(Py8CPU* cpu, Py8Opcode opcode, char* code);

/**
* @brief Adds the given byte to the value held in the given register.
*
* Opcode: 0x7XKK.
* Code: ADD Vx, KK.
*
* @note Overflows are not considered.
*
* @param `cpu`
* @param `opcode`
* @param `code`
* @return Always returns `CHIP8_EXECUTION_SUCCESSEFUL`.
*/
enum Py8ExecutionOutput
py8_cpuADD_VX_BYTE(Py8CPU* cpu, Py8Opcode opcode, char* code);

/**
* @brief Compares the values held in the two given registers for equality.
*
* Opcode: 0x4XKK.
* Code: SE VX, VY.
*
* The interpreter compares the values held in the registers VX and VY. If they are
* equal, the interpreter increments the PC.
*
* @param `cpu`
* @param `opcode`
* @param `code`
* @return Always returns `CHIP8_EXECUTION_SUCCESSEFUL`.
*/
enum Py8ExecutionOutput
py8_cpuSE_VX_VY(Py8CPU* cpu, Py8Opcode opcode, char* code);

/**
* @brief Compares the values held in the two given registers for diferent values.
*
* Opcode: 0x5XY0.
* Code: SE VX, VY.
*
* The interpreter compares the values held in the registers VX and VY. If they are not
* equal, the interpreter increments the PC.
*
* @param `cpu`
* @param `opcode`
* @param `code`
* @return Always returns `CHIP8_EXECUTION_SUCCESSEFUL`.
*/
enum Py8ExecutionOutput
py8_cpuSNE_VX_VY(Py8CPU* cpu, Py8Opcode opcode, char* code);

/**
* @brief Sets the value of register VX to the value held at the register VY.
*
* Opcode: 0x8XY0.
* Code: LD VX, VY.
*
* @param `cpu`
* @param `opcode`
* @param `code`
* @return Always returns `CHIP8_EXECUTION_SUCCESSEFUL`.
*/
enum Py8ExecutionOutput
py8_cpuLD_VX_VY(Py8CPU* cpu, Py8Opcode opcode, char* code);

/**
* @brief Sets the value of register VX to Vx OR Vy.
*
* Opcode: 0x8XY1.
* Code: OR VX, VY.
*
* @param `cpu`
* @param `opcode`
* @param `code`
* @return Always returns `CHIP8_EXECUTION_SUCCESSEFUL`.
*/
enum Py8ExecutionOutput
py8_cpuOR_VX_VY(Py8CPU* cpu, Py8Opcode opcode, char* code);

/**
* @brief Sets the value of register VX to Vx AND Vy.
*
* Opcode: 0x8XY2.
* Code: AND VX, VY.
*
* @param `cpu`
* @param `opcode`
* @param `code`
* @return Always returns `CHIP8_EXECUTION_SUCCESSEFUL`.
*/
enum Py8ExecutionOutput
py8_cpuAND_VX_VY(Py8CPU* cpu, Py8Opcode opcode, char* code);

/**
* @brief Sets the value of register VX to Vx XOR Vy.
*
* Opcode: 0x8XY3.
* Code: XOR VX, VY.
*
* @param `cpu`
* @param `opcode`
* @param `code`
* @return Always returns `CHIP8_EXECUTION_SUCCESSEFUL`.
*/
enum Py8ExecutionOutput
py8_cpuXOR_VX_VY(Py8CPU* cpu, Py8Opcode opcode, char* code);

/**
* @brief Sets the value of register VX to Vx + Vy, VF := carry.
*
* Opcode: 0x8XY4.
* Code: ADD VX, VY.
*
* @param `cpu`
* @param `opcode`
* @param `code`
* @return Always returns `CHIP8_EXECUTION_SUCCESSEFUL`.
*/
enum Py8ExecutionOutput
py8_cpuADD_VX_VY(Py8CPU* cpu, Py8Opcode opcode, char* code);

/**
* @brief Sets the value of register VX to Vx - Vy, VF := NOT borrow.
*
* Opcode: 0x8XY5.
* Code: SUB VX, VY.
*
* @param `cpu`
* @param `opcode`
* @param `code`
* @return Always returns `CHIP8_EXECUTION_SUCCESSEFUL`.
*/
enum Py8ExecutionOutput
py8_cpuSUB_VX_VY(Py8CPU* cpu, Py8Opcode opcode, char* code);

/**
* @brief Sets the value of register VX to Vy - Vx, VF := NOT borrow.
*
* Opcode: 0x8XY7.
* Code: SUBN VX, VY.
*
* @param `cpu`
* @param `opcode`
* @param `code`
* @return Always returns `CHIP8_EXECUTION_SUCCESSEFUL`.
*/
enum Py8ExecutionOutput
py8_cpuSUBN_VX_VY(Py8CPU* cpu, Py8Opcode opcode, char* code);

/**
* @brief This instruction is ambiguous. To solve it, the implementation mode
* decides which implementation reference to follow:
*  - COSMAC-VIP: Sets the value of register VX to VY >> 1, VF := overflow.
*  - Modern: Sets the value of register VX to VX >> 1, VF := overflow.
*
*
* Opcode: 0x8XY6.
* Code: SHR VX, { VY}.
*
* @param `cpu`
* @param `opcode`
* @param `code`
* @return Always returns `CHIP8_EXECUTION_SUCCESSEFUL`.
*/
enum Py8ExecutionOutput
py8_cpuSHR_VX_VY(Py8CPU* cpu, Py8Opcode opcode, char* code);

/**
* @brief This instruction is ambiguous. To solve it, the implementation mode
* decides which implementation reference to follow:
*  - COSMAC-VIP: Sets the value of register VX to VY << 1, VF := overflow.
*  - Modern: Sets the value of register VX to VX << 1, VF := overflow.
*
* Opcode: 0x8XYE.
* Code: SHL VX, { VY}.
*
* @param `cpu`
* @param `opcode`
* @param `code`
* @return Always returns `CHIP8_EXECUTION_SUCCESSEFUL`.
*/
enum Py8ExecutionOutput
py8_cpuSHL_VX_VY(Py8CPU* cpu, Py8Opcode opcode, char* code);

/**
* @brief Sets the value of the index register to the given address.
*
* Opcode: 0xANNN.
* Code: LD I, 0x0NNN.
*
* @param `cpu`
* @param `opcode`
* @param `code`
* @return Always returns `CHIP8_EXECUTION_SUCCESSEFUL`.
*/
enum Py8ExecutionOutput
py8_cpuLD_I_ADDR(Py8CPU* cpu, Py8Opcode opcode, char* code);

/**
* @brief This instructions is ambiguous. To solve it, the implementation mode
* decides which reference to follow:
* - COSMAC-VIP: The original COSMAC-VIP interpreter jumps to the adress 0x0NNN
*   plus the value held at the register V0, i.e. PC := 0x0NNN + V0.
* - Modern: Jumps to the location 0x0XNN + VX, i.e. PC := 0x0XNN + VX.
*
* Opcode: (COSMAC-VIP) 0xBNNN or (Modern) 0xBXNN.
* Code: (COSMAC-VIP) JP V0x0, 0x0NNN or (Modern) JP V0xX, 0x0NNN.
*
* @param `cpu`
* @param `opcode`
* @param `code`
* @return Always returns `CHIP8_EXECUTION_SUCCESSEFUL`.
*/
enum Py8ExecutionOutput
py8_cpuJP_V0_ADDR(Py8CPU* cpu, Py8Opcode opcode, char* code);

/**
* @brief Generate a random 8-bit integer and performs a bitwise and operation with the
* given byte; the result is stored into the register VX.
*
* Opcode: 0xCXKK.
* Code: RND VX, 0xKK
*
* @param `cpu`
* @param `opcode`
* @param `code`
* @return Always returns `CHIP8_EXECUTION_SUCCESSEFUL`.
*/
enum Py8ExecutionOutput
py8_cpuRND_VX_BYTE(Py8CPU* cpu, Py8Opcode opcode, char* code);

/**
* @brief Draw a sprite of size N at position VX, VY.
*
* Opcode: 0xDXYN.
* DRW V0xX, V0xY, 0xN
*
* @param `cpu`
* @param `opcode`
* @param `code`
* @return Always returns `CHIP8_EXECUTION_SUCCESSEFUL`.
*
* @note The Chip8's oroginal screen has 32x64 pixels. In our implementation, we represent
* the screen as a contiguous block of 8-bit integers that can either be activated (1)
* or deactivated (0), representing thus the black-white color of the original screen.
*
* A sprite is an array of 8-bit integers whose length ranges from 1 to 16. The 
* begining of the sprite is determined by the index register while its length is
* determined by the niblle 0xN.
*
* Once one of the bytes composing the sprite has been retrieved from memory,
* the Draw Function will check each bit of the byte. If the bit is activated,
* the corresponded pixel is then checked. If such pixel is also activated,
* the collision is registered in the register V0xF and the pixel is deactivated;
* if not, the corresponding pixel is activated.
*
* For example, the character `F` can be represented as a Chip8 sprite as follows:
*
*                   Mem Addr | bit  0 1 2 3 4 5 6 7  | Hex
*                   ---------+-----------------------+--------
*                    0x0E01  |      1 1 1 1 0 0 0 0  | 0xF0
*                    0x0E02  |      1 0 0 0 0 0 0 0  | 0x80
*                    0x0E03  |      1 1 1 1 0 0 0 0  | 0xF0
*                    0x0E04  |      1 0 0 0 0 0 0 0  | 0x80
*                    0x0E05  |      1 0 0 0 0 0 0 0  | 0x80
*
* The sprite starts at memory address 0x0E01 and ranges for 5 bytes.
*/
enum Py8ExecutionOutput
py8_cpuDRW_VX_VY_N(Py8CPU* cpu, Py8Opcode opcode, char* code);

/**
* @brief Skip the next instruction if the key VX is pressed.
*
* Opcode: 0xEX9E.
* Code: SKP V0xX.
*
* @param `cpu`
* @param `opcode`
* @param `code`
* @return Always returns `CHIP8_EXECUTION_SUCCESSEFUL`.
*/
enum Py8ExecutionOutput
py8_cpuSKP_VX(Py8CPU* cpu, Py8Opcode opcode, char* code);

/**
* @brief Skip the next instruction if the key VX is not pressed.
*
* Opcode: 0xEXA1.
* Code: SKNP V0xX.
*
* @param `cpu`
* @param `opcode`
* @param `code`
* @return Always returns `CHIP8_EXECUTION_SUCCESSEFUL`.
*/
enum Py8ExecutionOutput
py8_cpuSKNP_VX(Py8CPU* cpu, Py8Opcode opcode, char* code);

/**
* @brief Set VX to the value held at the delay time register.
*
* Opcode: 0xFX07.
* Code: LD V0xX, DT.
*
* @param `cpu`
* @param `opcode`
* @param `code`
* @return Always returns `CHIP8_EXECUTION_SUCCESSEFUL`.
*/
enum Py8ExecutionOutput
py8_cpuLD_VX_DT(Py8CPU* cpu, Py8Opcode opcode, char* code);

/**
* @brief Listen for a key press and store the value in the register VX.
*
* Opcode: 0xFX0A.
* Code: LD V0x0, K.
*
* @param `cpu`
* @param `opcode`
* @param `code`
* @return Always returns `CHIP8_EXECUTION_SUCCESSEFUL`.
*/
enum Py8ExecutionOutput
py8_cpuLD_VX_K(Py8CPU* cpu, Py8Opcode opcode, char* code);

/**
* @brief Set the delay timer register to the value held in VX.
* Opcode: 0xFX15.
* Code: LD DT, V0xX.
*
* @param `cpu`
* @param `opcode`
* @param `code`
* @return Always returns `CHIP8_EXECUTION_SUCCESSEFUL`.
*/
enum Py8ExecutionOutput
py8_cpuLD_DT_VX(Py8CPU* cpu, Py8Opcode opcode, char* code);

/**
* @brief Set the sound timer register to the value held in VX.
*
* Opcode: 0xFX18.
* Code: LD ST, V0xX.
*
* @param `cpu`
* @param `opcode`
* @param `code`
* @return Always returns `CHIP8_EXECUTION_SUCCESSEFUL`.
*/
enum Py8ExecutionOutput
py8_cpuLD_ST_VX(Py8CPU* cpu, Py8Opcode opcode, char* code);

/**
* @brief Set I := I + VX.
*
* Opcode: 0xFX1E.
* Code: ADD I, V0xX.
*
* @param `cpu`
* @param `opcode`
* @param `code`
* @return Always returns `CHIP8_EXECUTION_SUCCESSEFUL`.
*/
enum Py8ExecutionOutput
py8_cpuADD_I_VX(Py8CPU* cpu, Py8Opcode opcode, char* code);

/**
* @brief The value of I is set to the location for the hexadecimal
* sprite corresponding to the value of Vx.
*
* Opcode: 0xFX29.
* Code: LD F, V0xX.
*
* @param `cpu`
* @param `opcode`
* @param `code`
* @return Always returns `CHIP8_EXECUTION_SUCCESSEFUL`.
*/
enum Py8ExecutionOutput
py8_cpuLD_F_VX(Py8CPU* cpu, Py8Opcode opcode, char* code);

/**
* @brief  Store BCD representation of Vx in memory locations I, I+1, and I+2.
* LD B, V0xX.
*
* @param `cpu`
* @param `opcode`
* @param `code`
* @return Always returns `CHIP8_EXECUTION_SUCCESSEFUL`.
*
* @note The Binary Coded Representation of a number (assumed written in the
* decimal base) represent it by representing each of its decimal digit to binary.
*
* Example
* -------
* Consider the number 1732. Then the BCD representation of such number is
                      8  4  2  1
                ---+-------------
*                1 |  0  0  0  1
*                7 |  0  1  1  1
*                3 |  0  0  1  1
*                2 |  0  0  1  0
*/
enum Py8ExecutionOutput
py8_cpuLD_B_VX(Py8CPU* cpu, Py8Opcode opcode, char* code);

/**
* @brief This instruction is ambiguous. To solve the proble, the CPU mode decides
* which implementation to follow:
*  - COSMAV-VIP: for each of the values i from 0 to X (inclusive), the interpreter
*    sets the value held at memory at the adrress I+i to the value held at VX and, in
*    the same step of the loop, it increments I by 1.
*  - Modern: for each of the values i from 0 to X (inclusive), the interpreter
*    sets the values held at the memory at I+i to the value held at VX.
*
* Opcode: 0xFX55.
* Code: LD [I], V0xX.
*
* @param `cpu`
* @param `opcode`
* @param `code`
* @return Always returns `CHIP8_EXECUTION_SUCCESSEFUL`.
*/
enum Py8ExecutionOutput
py8_cpuLD_I_V0_VX(Py8CPU* cpu, Py8Opcode opcode, char* code);

/**
* @brief This instruction is ambiguous. To solve the proble, the CPU mode decides
* which implementation to follow:
*  - COSMAV-VIP: for each of the values i from 0 to X (inclusive), the interpreter
*    sets the value of the register VX to the value held at memory address I+i, in
*    the same step of the loop, it increments I by 1.
*  - Modern: for each of the values i from 0 to X (inclusive), the interpreter
*    sets the value of register VX to the value held at the memory address I+i.
*
* Opcode: 0xFX65.
* Code: LD V0xX, [I].
*
* @param `cpu`
* @param `opcode`
* @param `code`
* @return Always returns `CHIP8_EXECUTION_SUCCESSEFUL`.
*/
enum Py8ExecutionOutput
py8_cpuLD_VX_V0_I(Py8CPU* cpu, Py8Opcode opcode, char* code);

#ifdef __cplusplus
    }
#endif
#endif // PY8_CPU_H
