/**
* @file cpu.h
* @author Paulo Arruda
* @license GPL-3
* @brief Declaration of the Chip8's CPU and related emulation routines.
*
* @note Diferent Chip8 implementations exist regarding the following instructions:
*
*     - (0x8XY6) SHR V{0xX}, V{0xY}
*     - (0x8XYE) SHL V{0xX}, V{0xY}
*     - (0xBXNN) JP V0x0, 0x0NNN or JP V0xX, 0x0NNN
*     - (0xFX55) LD [I], V{0xX}
*     - (0xFX65) LD V{0xX}, [I]
*
* In the original COSMAC-VIP interpreter, these instructions did the following:
*
*     - (0x8XY6) SHR V{0xX}, V{0xY}: Right-shifts the value held in V{0xY} by 1 and
*                load the result in V{0xX}.
*     - (0x8XYE) SHL V{0xX}, V{0xY}: Left-shifts the value held in V{0xY} by 1 and
*                load the result in V{0xX}.
*     - (0xBNNN) JP V{0x0}, 0x0XNN: Jumps to the memory address 0x0NNN + V{0x0}.
*     - (0xFX55) LD [I], V{0xX}: For each index y in [0,X], the interpreter stores
*                the value of V{0xy} in the memory location I+y and increments the
*                index register by 1.
*     - (0xFX65) LD V{0xX}, [I]: For each index y in [0,X], the interpreter loads
*                the value of the memory location I+y into the register V{0xy} and
*                increments the index register by 1.
*
* Other interpreters, such as CHIP48, SUPER-CHIP, or the Amiga interpreter, had
* different interpretations for these instructions:
*
*     - (0x8XY6) SHR V{0xX}, V{0xY}: Right-shifts the value held in V{0xX} by 1 and
*                loads the result in V{0xX}. Hence, V{0xY} is ignored.
*     - (0x8XYE) SHL V{0xX}, V{0xY}: Left-shifts the value held in V{0xX} by 1 and
*                load the result in V{0xX}. Hence, V{0xY} is ignored.
*     - (0xBXNN) JP V{0xX}, 0x0XNN: Jumps to the memory address 0x0XNN + V{0xX}.
*     - (0xFX55) LD [I], V{0xX}: For each index y in [0,X], the interpreter stores
*                the value of V{0xy} in the memory location I+y. The index register
*                is left unchanged.
*     - (0xFX65) LD V{0xX}, [I]: For each index y in [0,X], the interpreter loads
*                the value of the memory location I+y into the register V{0xy}. The
*                index register is left unchanged.
*
* @note For our the implementation, we follow the following references:
*
*     - Cowgod's Chip-8 Technical Reference v1.0. Accessed at
*
*           `http://devernay.free.fr/hacks/chip8/C8TECH10.HTM`
*
*     - Guide to making a CHIP-8 emulator by Tobias Langhoff. Accessed at
*
*           `https://tobiasvl.github.io/blog/write-a-chip-8-emulator/`
*
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

/**
* @def PY8_SIZE_KEYSET
* @brief The number of keys that CHIP8's keypad has.
*/
#define PY8_SIZE_KEYSET                16

/**
* @def PY8_SIZE_STACK
* @brief The size of CHIP8's stack.
*/
#define PY8_SIZE_STACK                 16

/**
* @def PY8_SIZE_REGISTERS
* @brief The number of registers that CHIP8 has.
*/

#define PY8_SIZE_REGISTERS             16
/**
* @def PY8_SIZE_RAM
* @brief The size of CHIP8's RAM.
* @note In old interpreters, the sector 0x000 - 0x1FF were reserved for the
* interpreter itself; the sector reserved for the programs were 0x200 - 0xFFF.
*
*               +---------------+= 0xFFF (4095) End of Chip-8 RAM
*               |               |
*               |               |
*               |               |
*               |               |
*               |               |
*               | 0x200 to 0xFFF|
*               |     Chip-8    |
*               | Program / Data|
*               |     Space     |
*               |               |
*               |               |
*               |               |
*               +- - - - - - - -+= 0x600 (1536) Start of ETI 660 Chip-8 programs
*               |               |
*               |               |
*               |               |
*               +---------------+= 0x200 (512) Start of most Chip-8 programs
*               | 0x000 to 0x1FF|
*               | Reserved for  |
*               |  interpreter  |
*               +---------------+= 0x000 (0) Start of Chip-8 RAM
*
*/
#define PY8_SIZE_RAM                   4096

/**
* @def PY8_SIZE_MAX_ROM_FILE
* @brief The maximum amount of memory CHIP8's RAM can dedicate for a program.
*/
#define PY8_SIZE_MAX_ROM_FILE          3584

/**
* @def PY8_SIZE_FONTSET_PIXELS
* @brief The total number of pixels that the fontset has.
*/
#define PY8_SIZE_FONTSET_PIXELS        80

/**
* @def PY8_SIZE_FONTSET_PIXEL_PER_SPRITE
* @brief The total number of pixels that each fonteset character has.
*/
#define PY8_SIZE_FONTSET_PIXEL_PER_SPRITE        5

/**
* @def PY8_SIZE_GRAPHICS
* @brief The total number of pixels in the original CHIP8's screen.
*/
#define PY8_SIZE_GRAPHICS              2048


/**
* @def PY8_GRAPHICS_WIDTH
* @brief The width of CHIP8's original screen.
* @note Our graphical implementation will use a multiple of this value as width (e.g.)
*       640
*/
#define PY8_GRAPHICS_WIDTH             64

/**
* @def PY8_GRAPHICS_HEIGHT
* @brief The height of CHIP8's original screen.
* @note Our graphical implementation will use a multiple of this value as height (e.g.)
*       320
*/
#define PY8_GRAPHICS_HEIGTH            32

/**
* @def PY8_MEM_ADDR_PROG_START
* @brief The memory address where the sector dedicated to store the program starts.
*/
#define PY8_MEM_ADDR_PROG_START        0x0200

/**
* @def PY8_MEM_ADDR_FONTSET_START
* @brief The memory address where CHIP8's fontset starts.
*/
#define PY8_MEM_ADDR_FONTSET_START     0x50

/**
* @def PY8_IMPLM_MODE_SHIFTS_USE_VY
* @brief A flag that will tell the interpreter to use V{0xY} in the bitwise shifts.
*/
#define PY8_IMPLM_MODE_SHIFTS_USE_VY   (1 << 0)

/**
* @def PY8_IMPLM_MODE_BNNN_USE_VX
* @brief A flag that will tell the interpreter to use V{0xX} in 0xB--- instructions.
*/
#define PY8_IMPLM_MODE_BNNN_USE_VX     (1 << 1)

/**
* @def PY8_IMPLM_MODE_FX_CHANGE_I
* @brief A flag that will tell the interpreter to increment I in the 0xFX55 and 0xFX65
*        instructions.
*/
#define PY8_IMPLM_MODE_FX_CHANGE_I     (1 << 2)

/**
* @def UNUSED(x)
* @brief Ignore an object.
* @param `x` The object to be ignored.
*/
#define UNUSED(x) ((void) x)

/**
* @def CAST_PTR(type, ptr)
* @brief Cast a pointer to a type.
* @param `type`
* @param `ptr`
*/
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
    PY8_EXECOUT_ROM_FILE_INVALID,
    PY8_EXECOUT_ROM_FILE_NOT_FOUND,
    PY8_EXECOUT_ROM_FILE_FAILED_TO_OPEN,
    PY8_EXECOUT_ROM_FILE_FAILED_TO_READ,
    PY8_EXECOUT_ROM_FILE_EXCEEDS_MAX_MEM,
    PY8_EXECOUT_EMPTY_STRUCT,
    PY8_EXECOUT_INDEX_OUT_RANGE,
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
*                        |___________ the most significant quarter.
*/
typedef struct{
    uint8_t lsq: 4;
    uint8_t iq1: 4;
    uint8_t iq2: 4;
    uint8_t msq: 4;
} Py8Opcode;

/**
* @brief Create a new opcode based on a integer value.
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
* @return the 16-bit unsigned integer representation of the first 12 bits of the
          `opcode`.
*/
uint16_t
py8_opcodeGetAddr(Py8Opcode opcode);

/**
* @brief Retrieves the rightmost byte from the `opcode`.
*
*                     0x X Y K K
*                           |___|
*                             |____ the byte.
*
* @param[in] `opcode`.
* @return A 8-bit unsigned integer represention of the rightmost byte from the
*         `opcode`.
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
* @param `memory` Array representation of Chip8's memory.
* @param `keys` Chip8's 16 key set. Each bit represent a key that is either pressed#
*         or released.
* @param `graphics` Array representation of Chip8's screen.
* @param `implm_flags`. Controls which implementation to follow.
*/
typedef struct{
    uint8_t memory[PY8_SIZE_RAM];
    uint8_t graphics[PY8_SIZE_GRAPHICS];
    uint8_t registers[PY8_SIZE_REGISTERS];
    uint16_t stack[PY8_SIZE_STACK];
    uint16_t keys;
    uint16_t pc;
    uint16_t ir;
    uint8_t implm_flags;
    uint8_t sp;
    uint8_t st;
    uint8_t dt;
} Py8CPU;

/**
* @brief Set the CPU's field members to their initial values.
*
* @param[in, out] `cpu`.
* @param[in] `mode` Determines which CHIP8 implementation to use.
* @return PY8_EXECOUT_SUCCESS.
*/
enum Py8ExecutionOutput
py8_cpuInit(Py8CPU* cpu, uint8_t implm_flags);

/**
* @brief Loads a given program into CHIP8's memory.
*
* @param[in, out] `cpu`.
* @param[in] `rom_file_path`.
* @return A code representation on whether the execution was sucesseful.
* @note The following error codes can be returned by this function:
* - `PY8_EXECOUT_EMPTY_STRUCT`.
* - `PY8_EXECOUT_ROM_FILE_INVALID`.
* - `PY8_EXECOUT_ROM_FILE_FAILED_TO_OPEN`.
* - `PY8_EXECOUT_ROM_FILE_EXCEEDS_MAX_MEM`.
* - `PY8_EXECOUT_ROM_FILE_FAILED_TO_READ`.
*/
enum Py8ExecutionOutput
py8_cpuLoadRom(Py8CPU* cpu, const char* rom_file_path);

/**
* @brief Toggle on/off a particular key.
*
* @param[in, out] `cpu`.
* @param[in] `key`.
* @param[in] `value`.
* @return A code representation on whether the execution was sucesseful.
* @note The following error codes can be returned by this function:
* - `PY8_EXECOUT_EMPTY_STRUCT`.
*/
enum Py8ExecutionOutput
py8_cpuSetKey(Py8CPU* cpu, size_t key, bool value);

/**
* @brief Retrieves the current value of the given key.
*
* @param[in, out] `cpu`.
* @param[in] `key`.
* @return The value of the key.
*/
static inline bool
py8_cpuGetKeyVal(Py8CPU cpu, size_t key){
    return (cpu.keys & (1 << key)) == 0? false: true;
}

/*
* @brief Function pointer representation the action of a given instruction.
*/
typedef enum Py8ExecutionOutput (*Py8InstructionExec)(Py8CPU* cpu, Py8Opcode opcode, char* code);

/*
* @brief Representation of a Chip8's instruction.
*
* @param `code` The string representation of the instruction.
* @param `exec` The function pointer that executes the instruction on the cpu.
*/
typedef struct{
    char code[30];
    Py8InstructionExec exec;
} Py8Instruction;

/**
* @brief Retrieves the especified instruction from an opcode.
*
* @param[in] `opcode`.
* @return The instruction determined by the `opcode`.
*/
Py8Instruction
py8_opcodeDecode(Py8Opcode opcode);

/**
* @brief Execute a step in the emulation process.
* 
* A step is here defined as the retrieval and execution of a single opcode and
* the update of the respective registers.
*
* @param[in, out] cpu
* @param[out] instruction
* @return A code representation on whether the execution was sucesseful indicating,
* if not, the problem ocurred.
*/
enum Py8ExecutionOutput
py8_cpuStep(Py8CPU* cpu, Py8Instruction* instruction);

/**
* @brief The instruction representation of any instruction given by an invalid opcode.
*
* Code: NOP.
*
* @param `cpu`
* @param `opcode`
* @param `code`
* @return Always returns the error code `PY8_EXECOUT_INVALID_OPCODE`.
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
* @return A code representation on whether the execution was sucesseful.
* @note This function will return `PY8_EXECOUT_EMPTY_STRUCT` if the cpu pointer is
*       empty.
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
* @return A code representation on whether the execution was sucesseful.
* @note This function will return `PY8_EXECOUT_EMPTY_STRUCT` if the cpu pointer is
*       empty, or `PY8_EXECOUT_EMPTY_STACK` if the stack is empty.
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
* @return A code representation on whether the execution was sucesseful.
* @note This function will return `PY8_EXECOUT_EMPTY_STRUCT` if the cpu pointer is
*       empty, or `PY8_EXECOUT_STACK_OVERFLOW` if the stack is full.
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
* @return A code representation on whether the execution was sucesseful.
* @note This function will return `PY8_EXECOUT_EMPTY_STRUCT` if the cpu pointer is
*       empty.
*/
enum Py8ExecutionOutput
py8_cpuJMP_ADDR(Py8CPU* cpu, Py8Opcode opcode, char* code);

/**
* @brief Compares a register's value to a given byte for equality.
*
* Opcode: 0x3XKK.
* Code: SE Vx{0xX}, 0xKK.
*
* The interpreter compares the value held in the register Vx to the byte 0xKK. If they
* are equal, the interpreter increments the PC.
*
* @param `cpu`
* @param `opcode`
* @param `code`
* @return A code representation on whether the execution was sucesseful.
* @note This function will return `PY8_EXECOUT_EMPTY_STRUCT` if the cpu pointer is
*       empty.
*/
enum Py8ExecutionOutput
py8_cpuSE_VX_BYTE(Py8CPU* cpu, Py8Opcode opcode, char* code);

/**
* @brief Compares a register's value to a given byte for inequality.
*
* Opcode: 0x4XKK.
* Code: SNE V{0xX}, 0xKK.
*
* The interpreter compares the value held in the register Vx to the byte KK. If they
* are not equal, the interpreter increments the PC.
*
* @param `cpu`
* @param `opcode`
* @param `code`
* @return A code representation on whether the execution was sucesseful.
* @note This function will return `PY8_EXECOUT_EMPTY_STRUCT` if the cpu pointer is
*       empty.
*/
enum Py8ExecutionOutput
py8_cpuSNE_VX_BYTE(Py8CPU* cpu, Py8Opcode opcode, char* code);

/**
* @brief Set the value of the register VX to the given byte.
*
* Opcode: 0x6XKK.
* Code: Code: LD V{0xX}, 0xKK.
*
* @param `cpu`
* @param `opcode`
* @param `code`
* @return A code representation on whether the execution was sucesseful.
* @note This function will return `PY8_EXECOUT_EMPTY_STRUCT` if the cpu pointer is
*       empty.
*/
enum Py8ExecutionOutput
py8_cpuLD_VX_BYTE(Py8CPU* cpu, Py8Opcode opcode, char* code);

/**
* @brief Add the given byte to the value held in the given register.
*
* Opcode: 0x7XKK.
* Code: ADD V{0xX}, 0xKK.
*
* @note Overflows are not considered.
*
* @param `cpu`
* @param `opcode`
* @param `code`
* @return A code representation on whether the execution was sucesseful.
* @note This function will return `PY8_EXECOUT_EMPTY_STRUCT` if the cpu pointer is
*       empty.
*/
enum Py8ExecutionOutput
py8_cpuADD_VX_BYTE(Py8CPU* cpu, Py8Opcode opcode, char* code);

/**
* @brief Compares the values held in the two given registers for equality.
*
* Opcode: 0x4XKK.
* Code: SE V{0xX}, V{0xY}.
*
* The interpreter compares the values held in the registers VX and VY. If they are
* equal, the interpreter increments the PC.
*
* @param `cpu`
* @param `opcode`
* @param `code`
* @return A code representation on whether the execution was sucesseful.
* @note This function will return `PY8_EXECOUT_EMPTY_STRUCT` if the cpu pointer is
*       empty.
*/
enum Py8ExecutionOutput
py8_cpuSE_VX_VY(Py8CPU* cpu, Py8Opcode opcode, char* code);

/**
* @brief Compares the values held in the two given registers for inequality.
*
* Opcode: 0x5XY0.
* Code: SE V{0xX}, V{0xY}.
*
* The interpreter compares the values held in the registers VX and VY. If they are not
* equal, the interpreter increments the PC.
*
* @param `cpu`
* @param `opcode`
* @param `code`
* @return A code representation on whether the execution was sucesseful.
* @note This function will return `PY8_EXECOUT_EMPTY_STRUCT` if the cpu pointer is
*       empty.
*/
enum Py8ExecutionOutput
py8_cpuSNE_VX_VY(Py8CPU* cpu, Py8Opcode opcode, char* code);

/**
* @brief Sets the value of register V{0xY} to the value held at the register V{0xX}.
*
* Opcode: 0x8XY0.
* Code: LD V{0xX}, V{0xY}.
*
* @param `cpu`
* @param `opcode`
* @param `code`
* @return A code representation on whether the execution was sucesseful.
* @note This function will return `PY8_EXECOUT_EMPTY_STRUCT` if the cpu pointer is
*       empty.
*/
enum Py8ExecutionOutput
py8_cpuLD_VX_VY(Py8CPU* cpu, Py8Opcode opcode, char* code);

/**
* @brief Sets the value of register V{0xX} to V{0xX} OR V{0xY}.
*
* Opcode: 0x8XY1.
* Code: OR V{0xX}, V{0xY}.
*
* @param `cpu`
* @param `opcode`
* @param `code`
* @return A code representation on whether the execution was sucesseful.
* @note This function will return `PY8_EXECOUT_EMPTY_STRUCT` if the cpu pointer is
*       empty.
*/
enum Py8ExecutionOutput
py8_cpuOR_VX_VY(Py8CPU* cpu, Py8Opcode opcode, char* code);

/**
* @brief Sets the value of register V{0xX} to V{0xX} AND V{0xY}.
*
* Opcode: 0x8XY2.
* Code: AND V{0xX}, V{0xY}.
*
* @param `cpu`
* @param `opcode`
* @param `code`
* @return A code representation on whether the execution was sucesseful.
* @note This function will return `PY8_EXECOUT_EMPTY_STRUCT` if the cpu pointer is
*       empty.
*/
enum Py8ExecutionOutput
py8_cpuAND_VX_VY(Py8CPU* cpu, Py8Opcode opcode, char* code);

/**
* @brief Sets the value of register V{0xX} to V{0xX} XOR V{0xY}.
*
* Opcode: 0x8XY3.
* Code: XOR VX, VY.
*
* @param `cpu`
* @param `opcode`
* @param `code`
* @return A code representation on whether the execution was sucesseful.
* @note This function will return `PY8_EXECOUT_EMPTY_STRUCT` if the cpu pointer is
*       empty.
*/
enum Py8ExecutionOutput
py8_cpuXOR_VX_VY(Py8CPU* cpu, Py8Opcode opcode, char* code);

/**
* @brief Sets the value of register V{0xX} to V{0xX} + V{0xY}, V{0xF} := carry.
*
* Opcode: 0x8XY4.
* Code: ADD V{0xX}, V{0xY}.
*
* @param `cpu`
* @param `opcode`
* @param `code`
* @return A code representation on whether the execution was sucesseful.
* @note This function will return `PY8_EXECOUT_EMPTY_STRUCT` if the cpu pointer is
*       empty.
*/
enum Py8ExecutionOutput
py8_cpuADD_VX_VY(Py8CPU* cpu, Py8Opcode opcode, char* code);

/**
* @brief Sets the value of register V{0xX} to V{0xX} - V{0xY}, V{0xF} := NOT borrow.
*
* Opcode: 0x8XY5.
* Code: SUB V{0xX}, V{0xY}.
*
* @param `cpu`
* @param `opcode`
* @param `code`
* @return A code representation on whether the execution was sucesseful.
* @note This function will return `PY8_EXECOUT_EMPTY_STRUCT` if the cpu pointer is
*       empty.
*/
enum Py8ExecutionOutput
py8_cpuSUB_VX_VY(Py8CPU* cpu, Py8Opcode opcode, char* code);

/**
* @brief Sets the value of register V{0xX} to V{0xY} - V{0xX}, V{0xF} := NOT borrow.
*
* Opcode: 0x8XY7.
* Code: SUBN V{0xX}, V{0xY}.
*
* @param `cpu`
* @param `opcode`
* @param `code`
* @return A code representation on whether the execution was sucesseful.
* @note This function will return `PY8_EXECOUT_EMPTY_STRUCT` if the cpu pointer is
*       empty.
*/
enum Py8ExecutionOutput
py8_cpuSUBN_VX_VY(Py8CPU* cpu, Py8Opcode opcode, char* code);

/**
* @brief This instruction is ambiguous. It either
*  - Sets the value of register V{0xX} to V{0xY} >> 1, VF := underflow. Or
*  - Modern: Sets the value of register V{0xX} to V{0xX} >> 1, VF := undeflow.
*    V{0xY} is therefore ignored.
*
* @see PY8_IMPLM_MODE_SHIFTS_USE_VY
*
* Opcode: 0x8XY6.
* Code: SHR V{0xX}, V{0xY}.
*
* @param `cpu`
* @param `opcode`
* @param `code`
* @return A code representation on whether the execution was sucesseful.
* @note This function will return `PY8_EXECOUT_EMPTY_STRUCT` if the cpu pointer is
*       empty.
*/
enum Py8ExecutionOutput
py8_cpuSHR_VX_VY(Py8CPU* cpu, Py8Opcode opcode, char* code);

/**
* @brief This instruction is ambiguous. It either
*  - Sets the value of register V{0xX} to V{0xY} << 1, VF := overflow. Or
*  - Modern: Sets the value of register V{0xX} to V{0xX} << 1, VF := overflow.
*    V{0xY} is therefore ignored.
*
* Opcode: 0x8XYE.
* Code: SHL V{0xX}, V{0xY}.
*
* @param `cpu`
* @param `opcode`
* @param `code`
* @return A code representation on whether the execution was sucesseful.
* @note This function will return `PY8_EXECOUT_EMPTY_STRUCT` if the cpu pointer is
*       empty.
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
* @return A code representation on whether the execution was sucesseful.
* @note This function will return `PY8_EXECOUT_EMPTY_STRUCT` if the cpu pointer is
*       empty.
*/
enum Py8ExecutionOutput
py8_cpuLD_I_ADDR(Py8CPU* cpu, Py8Opcode opcode, char* code);

/**
* @brief This instruction is ambiguous. It either
*  - (0xBNNN) Sets the index register to V{0x0} + 0x0NNN. Or
*  - (0xBXNN) Sets the index register to V{0xX} + 0x0XNN
*
* Opcode: 0xBNNN or 0xBXNN.
* Code: JP V0x0, 0x0NNN or JP V{0xX}, 0x0NNN.
*
* @see PY8_IMPLM_MODE_BNNN_USES_VX.
*
* @param `cpu`
* @param `opcode`
* @param `code`
* @return A code representation on whether the execution was sucesseful.
* @note This function will return `PY8_EXECOUT_EMPTY_STRUCT` if the cpu pointer is
*       empty.
*/
enum Py8ExecutionOutput
py8_cpuJP_V0_ADDR(Py8CPU* cpu, Py8Opcode opcode, char* code);

/**
* @brief Generate a random 8-bit integer and performs a bitwise and operation with the
* given byte; the result is stored into the register V{0xX}.
*
* Opcode: 0xCXKK.
* Code: RND V{0xX}, 0xKK
*
* @param `cpu`
* @param `opcode`
* @param `code`
* @return A code representation on whether the execution was sucesseful.
* @note This function will return `PY8_EXECOUT_EMPTY_STRUCT` if the cpu pointer is
*       empty.
*/
enum Py8ExecutionOutput
py8_cpuRND_VX_BYTE(Py8CPU* cpu, Py8Opcode opcode, char* code);

/**
* @brief Draw a sprite of size N at screen position V{0xX}, V{0xY}.
*
* Opcode: 0xDXYN.
* DRW V{0xX}, V{0xY}, 0xN
*
* @param `cpu`
* @param `opcode`
* @param `code`
* @return A code representation on whether the execution was sucesseful.
* @note This function will return `PY8_EXECOUT_EMPTY_STRUCT` if the cpu pointer is
*       empty.
*
* @note The Chip8's oroginal screen has 32x64 pixels. In our implementation, we represent
* the screen as a contiguous block of 8-bit integers that can either be activated (1)
* or deactivated (0), representing thus the black-white color scheme dealt by CHIP8.
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
*
*/
enum Py8ExecutionOutput
py8_cpuDRW_VX_VY_N(Py8CPU* cpu, Py8Opcode opcode, char* code);

/**
* @brief Skip the next instruction if the key V{0xX} is pressed.
*
* Opcode: 0xEX9E.
* Code: SKP V{0xX}.
*
* @param `cpu`
* @param `opcode`
* @param `code`
* @return A code representation on whether the execution was sucesseful.
* @note This function will return `PY8_EXECOUT_EMPTY_STRUCT` if the cpu pointer is
*       empty.
*/
enum Py8ExecutionOutput
py8_cpuSKP_VX(Py8CPU* cpu, Py8Opcode opcode, char* code);

/**
* @brief Skip the next instruction if the key V{0xX} is not pressed.
*
* Opcode: 0xEXA1.
* Code: SKNP V{0xX}.
*
* @param `cpu`
* @param `opcode`
* @param `code`
* @return A code representation on whether the execution was sucesseful.
* @note This function will return `PY8_EXECOUT_EMPTY_STRUCT` if the cpu pointer is
*       empty.
*/
enum Py8ExecutionOutput
py8_cpuSKNP_VX(Py8CPU* cpu, Py8Opcode opcode, char* code);

/**
* @brief Set V{0xX} to the value held at the delay time register.
*
* Opcode: 0xFX07.
* Code: LD V{0xX}, DT.
*
* @param `cpu`
* @param `opcode`
* @param `code`
* @return A code representation on whether the execution was sucesseful.
* @note This function will return `PY8_EXECOUT_EMPTY_STRUCT` if the cpu pointer is
*       empty.
*/
enum Py8ExecutionOutput
py8_cpuLD_VX_DT(Py8CPU* cpu, Py8Opcode opcode, char* code);

/**
* @brief Listen for a key press and store its index in the register V{0xX}.
*
* Opcode: 0xFX0A.
* Code: LD V{0xX}, 0xK.
*
* @param `cpu`
* @param `opcode`
* @param `code`
* @return A code representation on whether the execution was sucesseful.
* @note This function will return `PY8_EXECOUT_EMPTY_STRUCT` if the cpu pointer is
*       empty.
*/
enum Py8ExecutionOutput
py8_cpuLD_VX_K(Py8CPU* cpu, Py8Opcode opcode, char* code);

/**
* @brief Set the delay timer register to V{0xX}.
*
* Opcode: 0xFX15.
* Code: LD DT, V{0xX}.
*
* @param `cpu`
* @param `opcode`
* @param `code`
* @return A code representation on whether the execution was sucesseful.
* @note This function will return `PY8_EXECOUT_EMPTY_STRUCT` if the cpu pointer is
*       empty.
*/
enum Py8ExecutionOutput
py8_cpuLD_DT_VX(Py8CPU* cpu, Py8Opcode opcode, char* code);

/**
* @brief Set the sound timer register to V{0xX}.
*
* Opcode: 0xFX18.
* Code: LD ST, V{0xX}.
*
* @param `cpu`
* @param `opcode`
* @param `code`
* @return A code representation on whether the execution was sucesseful.
* @note This function will return `PY8_EXECOUT_EMPTY_STRUCT` if the cpu pointer is
*       empty.
*/
enum Py8ExecutionOutput
py8_cpuLD_ST_VX(Py8CPU* cpu, Py8Opcode opcode, char* code);

/**
* @brief Set I := I + V{0xX}.
*
* Opcode: 0xFX1E.
* Code: ADD I, V{0xX}.
*
* @param `cpu`
* @param `opcode`
* @param `code`
* @return A code representation on whether the execution was sucesseful.
* @note This function will return `PY8_EXECOUT_EMPTY_STRUCT` if the cpu pointer is
*       empty.
*/
enum Py8ExecutionOutput
py8_cpuADD_I_VX(Py8CPU* cpu, Py8Opcode opcode, char* code);

/**
* @brief The value of I is set to the location for the sprite representing the character
* V{0xX}.
*
* Opcode: 0xFX29.
* Code: LD F, V{0xX}.
*
* @param `cpu`
* @param `opcode`
* @param `code`
* @return A code representation on whether the execution was sucesseful.
* @note This function will return `PY8_EXECOUT_EMPTY_STRUCT` if the cpu pointer is
*       empty.
*/
enum Py8ExecutionOutput
py8_cpuLD_F_VX(Py8CPU* cpu, Py8Opcode opcode, char* code);

/**
* @brief  Store BCD representation of V{0xX} in memory locations I, I+1, and I+2.
*
* Opcode: 0xFX33
* Code: LD B, V{0xX}.
*
* @param `cpu`
* @param `opcode`
* @param `code`
* @return A code representation on whether the execution was sucesseful.
* @note This function will return `PY8_EXECOUT_EMPTY_STRUCT` if the cpu pointer is
*       empty.
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
*
*/
enum Py8ExecutionOutput
py8_cpuLD_B_VX(Py8CPU* cpu, Py8Opcode opcode, char* code);

/**
* @brief This instruction is ambiguous. It either
* - For each index y in [0,X], the interpreter stores the value of V{0xy} in the
*   memory location I+y and increments the index register by 1. Or
* - For each index y in [0,X], the interpreter stores the value of V{0xy} in the
*   memory location I+y. The index register is left unchanged.
*
* Opcode: 0xFX55.
* Code: LD [I], V{0xX}.
*
* @see PY8_IMPLM_MODE_FX_CHANGES_I.
*
* @param `cpu`
* @param `opcode`
* @param `code`
* @return A code representation on whether the execution was sucesseful.
* @note This function will return `PY8_EXECOUT_EMPTY_STRUCT` if the cpu pointer is
*       empty.
*/
enum Py8ExecutionOutput
py8_cpuLD_I_V0_VX(Py8CPU* cpu, Py8Opcode opcode, char* code);

/**
* @brief This instruction is ambiguous. It either
* - For each index y in [0,X], the interpreter stores the value held at the memory
*   location I+y in the register V{0xX} and increments the index register by 1. Or
* - For each index y in [0,X], the interpreter stores the value held at the memory
*   location I+y into the register V{0xX}. The index register is left unchanged.
*
* Opcode: 0xFX65.
* Code: LD V0xX, [I].
*
* @see PY8_IMPLM_MODE_FX_CHANGES_I.
*
* @param `cpu`
* @param `opcode`
* @param `code`
* @return A code representation on whether the execution was sucesseful.
* @note This function will return `PY8_EXECOUT_EMPTY_STRUCT` if the cpu pointer is
*       empty.
*/
enum Py8ExecutionOutput
py8_cpuLD_VX_V0_I(Py8CPU* cpu, Py8Opcode opcode, char* code);

#ifdef __cplusplus
    }
#endif
#endif // PY8_CPU_H
