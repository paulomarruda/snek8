/**
* @file cpu.c
* @author Paulo Arruda
* @license GPL-3
* @brief Implementation of the Chip8's CPU and related emulation routines.
*/
#ifndef SNEK8_CPU_C
    #define SNEK8_CPU_C
#include <stdint.h>
#include <stdio.h>
#ifdef __cplusplus
    extern "C"{
#endif

#include <time.h>
#include <stdlib.h>
#include "cpu.h"

#define SIZE_U8 sizeof(uint8_t)
#define SIZE_U16 sizeof(uint16_t)
#define SIZE_U64 sizeof(uint64_t)

enum Snek8ExecutionOutput
snek8_stackInit(Snek8Stack *stack){
    if (!stack){
        return SNEK8_EXECOUT_EMPTY_STRUCT;
    }
    stack->sp = 0;
    (void) memset(stack->buffer, 0, SNEK8_SIZE_STACK * SIZE_U16);
    return SNEK8_EXECOUT_SUCCESS;
}

enum Snek8ExecutionOutput
snek8_stackPush(Snek8Stack *stack, uint16_t* pc, uint16_t opcode){
    if (SNEK8_SIZE_STACK == stack->sp){
        return SNEK8_EXECOUT_STACK_OVERFLOW;
    }
    stack->buffer[stack->sp] = *pc;
    stack->sp++;
    *pc = snek8_opcodeGetAddr(opcode);
    return SNEK8_EXECOUT_SUCCESS;
}

enum Snek8ExecutionOutput
snek8_stackPop(Snek8Stack *stack, uint16_t* pc){
    if (0 == stack->sp){
        return SNEK8_EXECOUT_STACK_EMPTY;
    }
    stack->sp--;
    *pc = stack->buffer[stack->sp];
    return SNEK8_EXECOUT_SUCCESS;
}

enum Snek8ExecutionOutput
snek8_cpuInit(Snek8CPU* cpu, uint8_t implm_flags){
    if (!cpu){
        return SNEK8_EXECOUT_EMPTY_STRUCT;
    }
    srand(time(NULL));
    cpu->implm_flags = implm_flags;
    uint8_t fontset[SNEK8_SIZE_FONTSET_PIXELS] = {
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
        0xF0, 0x80, 0xF0, 0x80, 0x80, // F
    };
    cpu->keys = 0;
    cpu->pc = SNEK8_MEM_ADDR_PROG_START;
    cpu->ir = 0;
    cpu->sp = 0;
    cpu->dt = 0;
    cpu->sp = 0;
    (void) snek8_stackInit(&cpu->stack);
    (void) memset(&cpu->registers, 0, SNEK8_SIZE_REGISTERS * SIZE_U8);
    (void) memset(&cpu->memory, 0, SNEK8_SIZE_RAM * SIZE_U8);
    (void) memset(&cpu->graphics, 0, SNEK8_SIZE_GRAPHICS * SIZE_U8);
    (void) memcpy(cpu->memory + SNEK8_MEM_ADDR_FONTSET_START, fontset, SNEK8_SIZE_FONTSET_PIXELS * SIZE_U8);
    return SNEK8_EXECOUT_SUCCESS;
}

enum Snek8ExecutionOutput
snek8_cpuLoadRom(Snek8CPU* cpu, const char* rom_file_path){
    if (!cpu){
        return SNEK8_EXECOUT_EMPTY_STRUCT;
    }else if (!rom_file_path){
        return SNEK8_EXECOUT_ROM_FILE_INVALID;
    }
    FILE* rom_file = fopen(rom_file_path, "rb");
    if (!rom_file_path){
        return SNEK8_EXECOUT_ROM_FILE_FAILED_TO_OPEN;
    }
    (void) fseek(rom_file, 0, SEEK_END);
    size_t rom_size = ftell(rom_file);
    if (rom_size > SNEK8_SIZE_MAX_ROM_FILE){
        fclose(rom_file);
        return SNEK8_EXECOUT_ROM_FILE_EXCEEDS_MAX_MEM;
    }
    rewind(rom_file);
    size_t nread = fread(cpu->memory + SNEK8_MEM_ADDR_PROG_START, 1, rom_size, rom_file);
    if (nread != rom_size){
        fclose(rom_file);
        return SNEK8_EXECOUT_ROM_FILE_FAILED_TO_READ;
    }
    (void) fclose(rom_file);
    return SNEK8_EXECOUT_SUCCESS;
}

/**
* @brief Manages the CPU timers.
*
* @param `cpu`.
*/
static inline void
_snek8_cpuTickTimers(Snek8CPU* cpu){
    if (cpu->dt){
        cpu->dt--;
    }
    if (cpu->st){
        cpu->st--;
    }
}

/**
* @brief Increments the program counter.
*
* @param `cpu`.
*/
static inline void
_snek8_cpuIncrementPC(Snek8CPU* cpu){
    cpu->pc += 2;
}

static inline void
_snek8_cpuDecrementPC(Snek8CPU* cpu){
    cpu->pc -= 2;
}

/**
* @brief Retrieves the opcode from memory.
*
* @param `cpu`.
* @return The opcode specified by the emulation process.
*/
static inline uint16_t
_snek8_cpuGetOpcode(Snek8CPU cpu){
    uint16_t opcode = cpu.memory[cpu.pc];
    opcode <<= 8;
    opcode |= cpu.memory[cpu.pc + 1];
    return opcode;
}

enum Snek8ExecutionOutput
snek8_cpuSetKey(Snek8CPU* cpu, size_t key, bool value){
    if (!cpu){
        return SNEK8_EXECOUT_EMPTY_STRUCT;
    }
    if (value){
        cpu->keys |= (1 << key);
    }else{
        cpu->keys &= (~(1 << key));
    }
    return SNEK8_EXECOUT_SUCCESS;
}

/*
* NOP
*/
enum Snek8ExecutionOutput
snek8_cpuExecutionError(Snek8CPU* cpu, uint16_t opcode){
    UNUSED cpu;
    UNUSED opcode;
    return SNEK8_EXECOUT_INVALID_OPCODE;
}

/*
* CLS
* 0x00E0
*/
enum Snek8ExecutionOutput
snek8_cpuCLS(Snek8CPU* cpu, uint16_t opcode){
    UNUSED opcode;
    UNUSED memset(&cpu->graphics, 0, SNEK8_SIZE_GRAPHICS * SIZE_U8);
    return SNEK8_EXECOUT_SUCCESS;
}

/*
* RET
* 0x00EE
*/
enum Snek8ExecutionOutput
snek8_cpuRET(Snek8CPU* cpu, uint16_t opcode){
    UNUSED opcode;
    return snek8_stackPop(&cpu->stack, &cpu->pc);
}

/*
* CALL 0x0NNN
* 0x2NNN
*/
enum Snek8ExecutionOutput
snek8_cpuCALL(Snek8CPU* cpu, uint16_t opcode){
   return snek8_stackPush(&cpu->stack, &cpu->pc, opcode);
}

/*
* JP 0x0NNN
* 0x1NNN
*/
enum Snek8ExecutionOutput
snek8_cpuJMP_ADDR(Snek8CPU* cpu, uint16_t opcode){
    cpu->pc = snek8_opcodeGetAddr(opcode);
    return SNEK8_EXECOUT_SUCCESS;
}

/*
* SE V{0xX}, 0xKK
* 0x3XKK
*/
enum Snek8ExecutionOutput
snek8_cpuSE_VX_BYTE(Snek8CPU* cpu, uint16_t opcode){
    uint8_t kk = snek8_opcodeGetByte(opcode);
    uint8_t x = snek8_opcodeGetNibble(opcode, 2);
    if (cpu->registers[x] == kk){
        _snek8_cpuIncrementPC(cpu);
    }
    return SNEK8_EXECOUT_SUCCESS;
}

/*
* SNE V{0xX}, 0xKK
* 0x4KK
*/
enum Snek8ExecutionOutput
snek8_cpuSNE_VX_BYTE(Snek8CPU* cpu, uint16_t opcode){
    uint8_t kk = snek8_opcodeGetByte(opcode);
    uint8_t x = snek8_opcodeGetNibble(opcode, 2);
    if (cpu->registers[x] != kk){
        _snek8_cpuIncrementPC(cpu);
    }
    return SNEK8_EXECOUT_SUCCESS;
}

/*
* LD V{0xX}, 0xKK
* 0x6XKK
*/
enum Snek8ExecutionOutput
snek8_cpuLD_VX_BYTE(Snek8CPU* cpu, uint16_t opcode){
    uint8_t kk = snek8_opcodeGetByte(opcode);
    uint8_t x = snek8_opcodeGetNibble(opcode, 2);
    cpu->registers[x] = kk;
    return SNEK8_EXECOUT_SUCCESS;
}

/*
* ADD V{0xX}, 0xKK
* 0x7XKK
*/
enum Snek8ExecutionOutput
snek8_cpuADD_VX_BYTE(Snek8CPU* cpu, uint16_t opcode){
    uint8_t kk = snek8_opcodeGetByte(opcode);
    uint8_t x = snek8_opcodeGetNibble(opcode, 2);
    cpu->registers[x] += kk;
    return SNEK8_EXECOUT_SUCCESS;
}

/*
* SE V{0xX}, V{0xY}
* 0x5XY0
*/
enum Snek8ExecutionOutput
snek8_cpuSE_VX_VY(Snek8CPU* cpu, uint16_t opcode){
    uint8_t x = snek8_opcodeGetNibble(opcode, 2);
    uint8_t y = snek8_opcodeGetNibble(opcode, 1);
    if (cpu->registers[x] == cpu->registers[y]){
        _snek8_cpuIncrementPC(cpu);
    }
    return SNEK8_EXECOUT_SUCCESS;
}

/*
* SNE V{0xX}, V{0xY}
* 0x9XY0
*/
enum Snek8ExecutionOutput
snek8_cpuSNE_VX_VY(Snek8CPU* cpu, uint16_t opcode){
    uint8_t x = snek8_opcodeGetNibble(opcode, 2);
    uint8_t y = snek8_opcodeGetNibble(opcode, 1);
    if (cpu->registers[x] != cpu->registers[y]){
        _snek8_cpuIncrementPC(cpu);
    }
    return SNEK8_EXECOUT_SUCCESS;
}

/*
* LD V{0xX}, V{0xY}
* 0x8XY0
*/
enum Snek8ExecutionOutput
snek8_cpuLD_VX_VY(Snek8CPU* cpu, uint16_t opcode){
    uint8_t x = snek8_opcodeGetNibble(opcode, 2);
    uint8_t y = snek8_opcodeGetNibble(opcode, 1);
    cpu->registers[x] = cpu->registers[y];
    return SNEK8_EXECOUT_SUCCESS;
}

/*
* OR V{0xX}, V{0xY}
* 0x8XY1
*/
enum Snek8ExecutionOutput
snek8_cpuOR_VX_VY(Snek8CPU* cpu, uint16_t opcode){
    uint8_t x = snek8_opcodeGetNibble(opcode, 2);
    uint8_t y = snek8_opcodeGetNibble(opcode, 1);
    cpu->registers[x] |= cpu->registers[y];
    return SNEK8_EXECOUT_SUCCESS;
}

/*
* AND V{0xX}, V{0xY}.
* 0x8XY2
*/
enum Snek8ExecutionOutput
snek8_cpuAND_VX_VY(Snek8CPU* cpu, uint16_t opcode){
    uint8_t x = snek8_opcodeGetNibble(opcode, 2);
    uint8_t y = snek8_opcodeGetNibble(opcode, 1);
    cpu->registers[x] &= cpu->registers[y];
    return SNEK8_EXECOUT_SUCCESS;
}

/*
* XOR V{0xX}, V{0xY}.
* 0x8XY3
*/
enum Snek8ExecutionOutput
snek8_cpuXOR_VX_VY(Snek8CPU* cpu, uint16_t opcode){
    uint8_t x = snek8_opcodeGetNibble(opcode, 2);
    uint8_t y = snek8_opcodeGetNibble(opcode, 1);
    cpu->registers[x] ^= cpu->registers[y];
    return SNEK8_EXECOUT_SUCCESS;
}

/*
* ADD V{0xX}, V{0xY}
* 0x8XY4
* note: The reason we use the variable `carry` below instead of assigning
* the flag direct to V{0xF} is that V{0xF} could also be used in the operation,
* and thus, if the flag assignment occurs before the ADD operation is done,
* the value stored in V{0xF} is lost and the operation's result is incorrect.
*/
enum Snek8ExecutionOutput
snek8_cpuADD_VX_VY(Snek8CPU* cpu, uint16_t opcode){
    uint8_t x = snek8_opcodeGetNibble(opcode, 2);
    uint8_t y = snek8_opcodeGetNibble(opcode, 1);
    uint8_t carry = (UINT8_MAX - cpu->registers[x]) < cpu->registers[y]? 1: 0;
    cpu->registers[x] += cpu->registers[y];
    cpu->registers[0xF] = carry;
    return SNEK8_EXECOUT_SUCCESS;
}

/*
* SUB V{0xX}, V{0xY}
* 0x8XY5
* note: The reason we use the variable `not_borrow` below instead of assigning
* the flag direct to V{0xF} is that V{0xF} could also be used in the operation,
* and thus, if the flag assignment occurs before the SUB operation is done,
* the value stored in V{0xF} is lost and the operation's result is incorrect.
*/
enum Snek8ExecutionOutput
snek8_cpuSUB_VX_VY(Snek8CPU* cpu, uint16_t opcode){
    uint8_t x = snek8_opcodeGetNibble(opcode, 2);
    uint8_t y = snek8_opcodeGetNibble(opcode, 1);
    uint8_t not_borrow = cpu->registers[x] >= cpu->registers[y]? 1: 0;
    cpu->registers[x] -= cpu->registers[y];
    cpu->registers[0xF] = not_borrow;
    return SNEK8_EXECOUT_SUCCESS;
}

/*
* SUBN V{0xX}, V{0xY}
* 0x8XY7
* note: The reason we use the variable `not_borrow` below instead of assigning
* the flag direct to V{0xF} is that V{0xF} could also be used in the operation,
* and thus, if the flag assignment occurs before the SUBN operation is done,
* the value stored in V{0xF} is lost and the operation's result is incorrect.
*/
enum Snek8ExecutionOutput
snek8_cpuSUBN_VX_VY(Snek8CPU* cpu, uint16_t opcode){
    uint8_t x = snek8_opcodeGetNibble(opcode, 2);
    uint8_t y = snek8_opcodeGetNibble(opcode, 1);
    uint8_t not_borrow = cpu->registers[y] >= cpu->registers[x]? 1: 0;
    cpu->registers[x] = cpu->registers[y] - cpu->registers[x];
    cpu->registers[0xF] = not_borrow;
    return SNEK8_EXECOUT_SUCCESS;
}

/*
* SHR V{0xX}, V{0xY}
* 0x8XY6
* note: The reason we use the variable `shr_underflow` below instead of assigning
* the flag direct to V{0xF} is that V{0xF} could also be used in the operation,
* and thus, if the flag assignment occurs before the SHR operation is done,
* the value stored in V{0xF} is lost and the operation's result is incorrect.
*/
enum Snek8ExecutionOutput
snek8_cpuSHR_VX_VY(Snek8CPU* cpu, uint16_t opcode){
    uint8_t x = snek8_opcodeGetNibble(opcode, 2);
    uint8_t y = snek8_opcodeGetNibble(opcode, 1);
    uint8_t shr_underflow = cpu->registers[x] & 0x1;
    if (cpu->implm_flags & SNEK8_IMPLM_MODE_SHIFTS_USE_VY){
        cpu->registers[x] = cpu->registers[y];
    }
    cpu->registers[x] >>= 1;
    cpu->registers[0xF] = shr_underflow;
    return SNEK8_EXECOUT_SUCCESS;
}

/*
* SHL V{0xX}, V{0xY}.
* 0x8XYE
* note: The reason we use the variable `shl_overflow` below instead of assigning
* the flag direct to V{0xF} is that V{0xF} could also be used in the operation,
* and thus, if the flag assignment occurs before the SHR operation is done,
* the value stored in V{0xF} is lost and the operation's result is incorrect.
*/
enum Snek8ExecutionOutput
snek8_cpuSHL_VX_VY(Snek8CPU* cpu, uint16_t opcode){
    uint8_t x = snek8_opcodeGetNibble(opcode, 2);
    uint8_t y = snek8_opcodeGetNibble(opcode, 1);
    uint8_t shl_overflow = (cpu->registers[x] & 0x80) >> 7u;
    if (cpu->implm_flags & SNEK8_IMPLM_MODE_SHIFTS_USE_VY){
            cpu->registers[x] = cpu->registers[y];
    }
    cpu->registers[x] <<= 1;
    cpu->registers[0xF] = shl_overflow;
    return SNEK8_EXECOUT_SUCCESS;
}

/*
* LD I, 0x0NNN
* 0xANNN
*/
enum Snek8ExecutionOutput
snek8_cpuLD_I_ADDR(Snek8CPU* cpu, uint16_t opcode){
    cpu->ir = snek8_opcodeGetAddr(opcode);
    return SNEK8_EXECOUT_SUCCESS;
}

/*
* JP V{0x0}, 0x0NNN.
* 0xBNNN
*
* JP V{0xX}, 0x0XNN.
* 0xBXNN
*/
enum Snek8ExecutionOutput
snek8_cpuJP_V0_ADDR(Snek8CPU* cpu, uint16_t opcode){
    uint16_t addr = snek8_opcodeGetAddr(opcode);
    uint8_t x = 0;
    if (cpu->implm_flags & SNEK8_IMPLM_MODE_BNNN_USES_VX){
        x = snek8_opcodeGetNibble(opcode, x);
    }
    cpu->pc = addr + cpu->registers[x];
    return SNEK8_EXECOUT_SUCCESS;
}

/*
* RND V{0xX}, 0xKK.
*/
enum Snek8ExecutionOutput
snek8_cpuRND_VX_BYTE(Snek8CPU* cpu, uint16_t opcode){
    uint8_t kk = snek8_opcodeGetByte(opcode);
    uint8_t x = snek8_opcodeGetNibble(opcode, 2);
    cpu->registers[x] = rand() & kk;
    return SNEK8_EXECOUT_SUCCESS;
}


/*
* DRW V{0xX}, V{0xY}, 0xN
* 0xDXYN
*/
enum Snek8ExecutionOutput
snek8_cpuDRW_VX_VY_N(Snek8CPU* cpu, uint16_t opcode){
    cpu->registers[0xF] = 0;
    uint8_t x = snek8_opcodeGetNibble(opcode, 2);
    uint8_t y = snek8_opcodeGetNibble(opcode, 1);
    uint8_t n = snek8_opcodeGetNibble(opcode, 0);
    if (cpu->ir + n > SNEK8_MEM_ADDR_RAM_END){
        return SNEK8_EXECOUT_MEM_ADDR_OUT_OF_BOUNDS;
    }
    uint8_t px = cpu->registers[x];
    uint8_t py = cpu->registers[y];
    for (uint8_t col = 0; col < n; col++){
        uint8_t byte = cpu->memory[cpu->ir + col];
        for (uint8_t row = 0; row < 8; row++){
            uint8_t bit = byte & (0x80u >> row);
            uint8_t* pixel_ptr = snek8_cpuGetPixel(cpu, px + row, py + col);
            *pixel_ptr ^= bit;
            if (bit && *pixel_ptr == 0){
                cpu->registers[0xF] = 1;
            }
        }
    }
    return SNEK8_EXECOUT_SUCCESS;
}

/*
* SKP V{0xX}
* 0xEX9E
*/
enum Snek8ExecutionOutput
snek8_cpuSKP_VX(Snek8CPU* cpu, uint16_t opcode){
    uint8_t x = snek8_opcodeGetNibble(opcode, 2);
    uint8_t key = cpu->registers[x];
    if (snek8_cpuGetKeyVal(*cpu, key)){
        _snek8_cpuIncrementPC(cpu);
    }
    return SNEK8_EXECOUT_SUCCESS;
}

/*
* SKNP V{0xX}
* 0xEXA1
*/
enum Snek8ExecutionOutput
snek8_cpuSKNP_VX(Snek8CPU* cpu, uint16_t opcode){
    uint8_t x = snek8_opcodeGetNibble(opcode, 2);
    uint8_t key = cpu->registers[x];
    if (!snek8_cpuGetKeyVal(*cpu, key)){
        _snek8_cpuIncrementPC(cpu);
    }
    return SNEK8_EXECOUT_SUCCESS;
}

/*
* LD V{0xX}, DT
* 0xFX07
*/
enum Snek8ExecutionOutput
snek8_cpuLD_VX_DT(Snek8CPU* cpu, uint16_t opcode){
    uint8_t x = snek8_opcodeGetNibble(opcode, 2);
    cpu->registers[x] = cpu->dt;
    return SNEK8_EXECOUT_SUCCESS;
}

/*
* LD V{0xX}, K{0xK}
* 0xFX0A
*/
enum Snek8ExecutionOutput
snek8_cpuLD_VX_K(Snek8CPU* cpu, uint16_t opcode){
    uint8_t x = snek8_opcodeGetNibble(opcode, 2);
    if (!cpu->keys){
        _snek8_cpuDecrementPC(cpu);
        return SNEK8_EXECOUT_SUCCESS;
    }
    for (size_t key = 0; key < SNEK8_SIZE_KEYSET; key++){
        if (snek8_cpuGetKeyVal(*cpu, key)){
            cpu->registers[x] = key;
            break;
        }
    }
    return SNEK8_EXECOUT_SUCCESS;
}

/*
* LD DT, V{0xX}.
* 0xFX15
*/
enum Snek8ExecutionOutput
snek8_cpuLD_DT_VX(Snek8CPU* cpu, uint16_t opcode){
    uint8_t x = snek8_opcodeGetNibble(opcode, 2);
    cpu->dt = cpu->registers[x];
    return SNEK8_EXECOUT_SUCCESS;
}

/*
* LD ST, V{0xX}
* 0xFX18
*/
enum Snek8ExecutionOutput
snek8_cpuLD_ST_VX(Snek8CPU* cpu, uint16_t opcode){
    uint8_t x = snek8_opcodeGetNibble(opcode, 2);
    cpu->st = cpu->registers[x];
    return SNEK8_EXECOUT_SUCCESS;
}

/*
* ADD I, V{0xX}
* 0xFX1E
*/
enum Snek8ExecutionOutput
snek8_cpuADD_I_VX(Snek8CPU* cpu, uint16_t opcode){
    uint8_t x = snek8_opcodeGetNibble(opcode, 2);
    cpu->ir = (cpu->ir + cpu->registers[x]) & 0x0FFF;
    return SNEK8_EXECOUT_SUCCESS;
}

/*
* LD F, V{0xX}
* 0xFX29
*/
enum Snek8ExecutionOutput
snek8_cpuLD_F_VX(Snek8CPU* cpu, uint16_t opcode){
    uint8_t x = snek8_opcodeGetNibble(opcode, 2);
    cpu->ir = SNEK8_MEM_ADDR_FONTSET_START + (SNEK8_SIZE_FONTSET_PIXEL_PER_SPRITE * cpu->registers[x]);
    return SNEK8_EXECOUT_SUCCESS;
}

/*
* LD B, V{0xX}
* 0xFX29
*/
enum Snek8ExecutionOutput
snek8_cpuLD_B_VX(Snek8CPU* cpu, uint16_t opcode){
    if (cpu->ir + 2 > SNEK8_MEM_ADDR_RAM_END){
        return SNEK8_EXECOUT_MEM_ADDR_OUT_OF_BOUNDS;
    }
    uint8_t x = snek8_opcodeGetNibble(opcode, 2);
    uint8_t value = cpu->registers[x];
    cpu->memory[cpu->ir + 2] = value % 10;
    value /= 10;
    cpu->memory[cpu->ir + 1] = value % 10;
    value /= 10;
    cpu->memory[cpu->ir] = value % 10;
    return SNEK8_EXECOUT_SUCCESS;
}

/*
* LD [I], V{0xX}
* 0xFX55
*/
enum Snek8ExecutionOutput
snek8_cpuLD_I_V0_VX(Snek8CPU* cpu, uint16_t opcode){
    uint8_t x = snek8_opcodeGetNibble(opcode, 2);
    if (cpu->ir + x > SNEK8_MEM_ADDR_RAM_END){
        return SNEK8_EXECOUT_MEM_ADDR_OUT_OF_BOUNDS;
    }
    if (cpu->implm_flags & SNEK8_IMPLM_MODE_FX_CHANGES_I){
        for (uint8_t i = 0; i <= x; i++){
            cpu->memory[cpu->ir + i] = cpu->registers[i];
            cpu->ir++;
        }
    }else {
        for (uint8_t i = 0; i <= x; i++){
            cpu->memory[cpu->ir + i] = cpu->registers[i];
        }
    }
    return SNEK8_EXECOUT_SUCCESS;
}

/*
* LD V{0xX}, [I]
* 0xFX65
*/
enum Snek8ExecutionOutput
snek8_cpuLD_VX_V0_I(Snek8CPU* cpu, uint16_t opcode){
    uint8_t x = snek8_opcodeGetNibble(opcode, 2);
    if (cpu->ir + x > SNEK8_MEM_ADDR_RAM_END){
        return SNEK8_EXECOUT_MEM_ADDR_OUT_OF_BOUNDS;
    }
    if (cpu->implm_flags & SNEK8_IMPLM_MODE_FX_CHANGES_I){
        for (uint8_t i=0; i<=x; i++){
            cpu->registers[i] = cpu->memory[cpu->ir + i];
            cpu->ir++;
        }
    }else {
        for (uint8_t i=0; i<=x; i++){
            cpu->registers[i] = cpu->memory[cpu->ir + i];
        }
    }
    return SNEK8_EXECOUT_SUCCESS;
}

Snek8Instruction
snek8_opcodeDecode(uint16_t opcode){
    Snek8Instruction instruction;
    uint8_t msq = snek8_opcodeGetNibble(opcode, 3);
    uint8_t lsq = snek8_opcodeGetNibble(opcode, 0);
    uint8_t iq1 = snek8_opcodeGetNibble(opcode, 1);
    uint8_t iq22 = snek8_opcodeGetNibble(opcode, 2);
    switch (msq) {
        case 0x0:
            switch (lsq) {
                case 0x0:
                    instruction = (Snek8Instruction) {
                        .code = "CLS",
                        .exec = snek8_cpuCLS};
                    break;
                case 0xE:
                    instruction = (Snek8Instruction) {
                        .code="RET",
                        .exec = snek8_cpuRET};
                    break;
                default:
                    instruction = (Snek8Instruction) {
                        .code = "NOP", 
                        .exec = snek8_cpuExecutionError
                    };
                    break;
            }
            break;
        case 0x1:
            instruction = (Snek8Instruction) {
                .code = "JP 0x0NNN",
                .exec = snek8_cpuJMP_ADDR,
            };
            break;
        case 0x2:
            instruction = (Snek8Instruction) {
                .code = "CALL 0x0NNN",
                .exec = snek8_cpuCALL,
            };
            break;
        case 0x3:
            instruction = (Snek8Instruction) {
                .code = "SE V{0xX}, 0xKK",
                .exec = snek8_cpuSE_VX_BYTE,
            };
            break;
        case 0x4:
            instruction = (Snek8Instruction) {
                .code = "SNE V{0xX}, 0xKK",
                .exec = snek8_cpuSNE_VX_BYTE,
            };
            break;
        case 0x5:
            instruction = (Snek8Instruction) {
                .code = "SE V{0xX}, V{0xY}",
                .exec = snek8_cpuSE_VX_VY,
            };
            break;
        case 0x6:
            instruction = (Snek8Instruction) {
                .code = "LD V{0xX}, 0xKK",
                .exec = snek8_cpuLD_VX_BYTE,
            };
            break;
        case 0x7:
            instruction = (Snek8Instruction) {
                .code = "ADD V{0xX}, 0xKK",
                .exec = snek8_cpuADD_VX_BYTE,
            };
            break;
        case 0x8:
            switch (lsq) {
                case 0x0:
                    instruction = (Snek8Instruction) {
                        .code = "LD V{0xX}, V{0xY}",
                        .exec = snek8_cpuLD_VX_VY,
                    };
                    break;
                case 0x1:
                    instruction = (Snek8Instruction) {
                        .code = "OR V{0xX}, V{0xY}",
                        .exec = snek8_cpuOR_VX_VY,
                    };
                    break;
                case 0x2:
                    instruction = (Snek8Instruction) {
                        .code = "AND V{0xX}, V{0xY}",
                        .exec = snek8_cpuAND_VX_VY,
                    };
                    break;
                case 0x3:
                    instruction = (Snek8Instruction) {
                        .code = "XOR V{0xX}, V{0xY}",
                        .exec = snek8_cpuXOR_VX_VY,
                    };
                    break;
                case 0x4:
                    instruction = (Snek8Instruction) {
                        .code = "ADD V{0xX}, V{0xY}",
                        .exec = snek8_cpuADD_VX_VY,
                    };
                    break;
                case 0x5:
                    instruction = (Snek8Instruction) {
                        .code = "SUB V{0xX}, V{0xY}",
                        .exec = snek8_cpuSUB_VX_VY,
                    };
                    break;
                case 0x6:
                    instruction = (Snek8Instruction) {
                        .code = "SHR V{0xX}, V{0xY}",
                        .exec = snek8_cpuSHR_VX_VY,
                    };
                    break;
                case 0x7:
                    instruction = (Snek8Instruction) {
                        .code = "SUBN V{0xX}, V{0xY}",
                        .exec = snek8_cpuSUBN_VX_VY,
                    };
                    break;
                case 0xE:
                    instruction = (Snek8Instruction) {
                        .code = "SHL V{0xX}, V{0xY}",
                        .exec = snek8_cpuSHL_VX_VY,
                    };
                    break;
                default:
                    instruction = (Snek8Instruction) {
                        .code = "NOP",
                        .exec = snek8_cpuExecutionError,
                    };
                    break;
            }
            break;
        case 0x9:
                instruction = (Snek8Instruction) {
                    .code = "SNE V{0xX}, V{0xY}",
                    .exec = snek8_cpuSNE_VX_VY,
                };
            break;
        case 0xA:
                instruction = (Snek8Instruction) {
                    .code = "LD I, 0x0NNN",
                    .exec = snek8_cpuLD_I_ADDR,
                };
            break;
        case 0xB:
                instruction = (Snek8Instruction) {
                    .code = "JP V{0x0}, 0x0NNN",
                    .exec = snek8_cpuJP_V0_ADDR,
                };
            break;
        case 0xC:
                instruction = (Snek8Instruction) {
                    .code = "RND V{0xX}, 0xKK",
                    .exec = snek8_cpuRND_VX_BYTE,
                };
            break;
        case 0xD:
                instruction = (Snek8Instruction) {
                    .code = "DRW V{0xX}, V{0xY}, 0xN",
                    .exec = snek8_cpuDRW_VX_VY_N,
                };
            break;
        case 0xE:
            switch (lsq) {
                case 0xE:
                    instruction = (Snek8Instruction) {
                        .code = "SKP V{0xX}",
                        .exec = snek8_cpuSKP_VX,
                    };
                    break;
                case 0x1:
                    instruction = (Snek8Instruction) {
                        .code = "SKNP V{0xX}",
                        .exec = snek8_cpuSKNP_VX,
                    };
                    break;
                default:
                    instruction = (Snek8Instruction) {
                        .code = "NOP",
                        .exec = snek8_cpuExecutionError, };
                    break;
            }
            break;
        case 0xF:
            switch (lsq) {
                case 0x7:
                    instruction = (Snek8Instruction) {
                        .code = "LD V{0xX}, DT",
                        .exec = snek8_cpuLD_VX_DT,
                    };
                    break;
                case 0xA:
                    instruction = (Snek8Instruction) {
                        .code = "LD V{0xX}, K{0xK}",
                        .exec = snek8_cpuLD_VX_K,
                    };
                    break;
                case 0x5:
                    switch (iq1) {
                        case 0x1:
                            instruction = (Snek8Instruction) {
                                .code = "LD DT, V{0xX}",
                                .exec = snek8_cpuLD_DT_VX,
                            };
                            break;
                        case 0x5:
                            instruction = (Snek8Instruction) {
                                .code = "LD [I], V{0xX}",
                                .exec = snek8_cpuLD_I_V0_VX,
                            };
                            break;
                        case 0x6:
                            instruction = (Snek8Instruction) {
                                .code = "LD V{0xX}, [I]",
                                .exec = snek8_cpuLD_VX_V0_I,
                            };
                            break;
                        default:
                            instruction = (Snek8Instruction) {
                                .code = "NOP",
                                .exec = snek8_cpuExecutionError,
                            };
                            break;
                    }
                    break;
                case 0x8:
                    instruction = (Snek8Instruction) {
                        .code = "LD ST, V{0xX}",
                        .exec = snek8_cpuLD_ST_VX,
                    };
                    break;
                case 0xE:
                    instruction = (Snek8Instruction) {
                        .code = "ADD I, V{0xX}",
                        .exec = snek8_cpuADD_I_VX,
                    };
                    break;
                case 0x9:
                    instruction = (Snek8Instruction) {
                        .code = "LD F, V{0xX}",
                        .exec = snek8_cpuLD_F_VX,
                    };
                    break;
                case 0x3:
                    instruction = (Snek8Instruction) {
                        .code = "LD B, V{0xX}",
                        .exec = snek8_cpuLD_B_VX,
                    };
                    break;
                default:
                    instruction = (Snek8Instruction) {
                        .code = "NOP",
                        .exec = snek8_cpuExecutionError,
                    };
                    break;
            }
            break;
    }
    return instruction;
}

enum Snek8ExecutionOutput
snek8_cpuStep(Snek8CPU* cpu, Snek8Instruction* instruction){
    uint16_t opcode = _snek8_cpuGetOpcode(*cpu);
    _snek8_cpuIncrementPC(cpu);
    *instruction = snek8_opcodeDecode(opcode);
    enum Snek8ExecutionOutput out = instruction->exec(cpu, opcode);
    _snek8_cpuTickTimers(cpu);
    return out;
}

#ifdef __cplusplus
    }
#endif
#endif // SNEK8_CPU_C
