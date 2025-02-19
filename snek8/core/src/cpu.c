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

/**
* @brief Converts a single hexadecimal-digit to its hexadecimal char representation.
*
* @param[in] `nibble`
*/
inline static char
_snek8_its(uint8_t nibble){
    char result = ' ';
    switch (nibble) {
        case 0x0:
            result = '0';
            break;
        case 0x1:
            result = '1';
            break;
        case 0x2:
            result = '2';
            break;
        case 0x3:
            result = '3';
            break;
        case 0x4:
            result = '4';
            break;
        case 0x5:
            result = '5';
            break;
        case 0x6:
            result = '6';
            break;
        case 0x7:
            result = '7';
            break;
        case 0x8:
            result = '8';
            break;
        case 0x9:
            result = '9';
            break;
        case 0xA:
            result = 'A';
            break;
        case 0xB:
            result = 'B';
            break;
        case 0xC:
            result = 'C';
            break;
        case 0xD:
            result = 'D';
            break;
        case 0xE:
            result = 'E';
            break;
        case 0xF:
            result = 'F';
            break;
        default:
            break;
    }
    return result;
}

/**
* @brief Converts an u8 to its hexadecimal form.
*
* @param[in, out] `code`.
* @param[in] `byte`.
*/
static inline void
_snek8_writeU8(char* code, uint8_t byte){
    code[2] = _snek8_its((byte >> 4) & 0xF);
    code[3] = _snek8_its(byte & 0xF);
}

/**
* @brief Converts a u16 to its hexadecimal form.
*
* @param[in, out] `code`.
* @param[in] `hex`.
*/
static inline void
_snek8_writeU16(char* code, uint16_t hex){
    code[2] = _snek8_its((hex >> 12) & 0xF);
    code[3] = _snek8_its((hex >> 8) & 0xF);
    code[4] = _snek8_its((hex >> 4) & 0xF);
    code[5] = _snek8_its(hex & 0xF);
}

Snek8Opcode
snek8_opcodeInit(uint16_t code){
    return (Snek8Opcode){
        .lsq = code & 0x000F,
        .iq1 = (code & 0x00F0) >> 4,
        .iq2 = (code & 0x0F00) >> 8,
        .msq = (code & 0xF000) >> 12,
    };
}

uint8_t
snek8_opcodeGetByte(Snek8Opcode opcode){
    return (opcode.iq1 << 4) | (opcode.lsq);
}

uint16_t
snek8_opcodeGetAddr(Snek8Opcode opcode){
    return (((uint16_t) opcode.iq2) << 8) |
           (((uint16_t) opcode.iq1) << 4) |
           ((uint16_t) opcode.lsq);
}

enum Snek8ExecutionOutput
snek8_cpuInit(Snek8CPU* cpu, uint8_t implm_flags){
    if (!cpu){
        return SNEK8_EXECOUT_EMPTY_STRUCT;
    }
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
    memset(&cpu->memory, 0, SNEK8_SIZE_RAM * SIZE_U8);
    memcpy(cpu->memory + SNEK8_MEM_ADDR_FONTSET_START, fontset, SNEK8_SIZE_FONTSET_PIXELS * SIZE_U8);
    memset(&cpu->graphics, 0, SNEK8_SIZE_GRAPHICS * SIZE_U8);
    memset(&cpu->registers, 0, SNEK8_SIZE_REGISTERS * SIZE_U8);
    memset(&cpu->stack, 0, SNEK8_SIZE_STACK * SIZE_U16);
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
    fclose(rom_file);
    // for (size_t i=SNEK8_MEM_ADDR_PROG_START; i<SNEK8_SIZE_RAM; i++){
    //     printf("0x%02x ", cpu->memory[i]);
    // }
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
static Snek8Opcode
_snek8_cpuGetOpcode(Snek8CPU cpu){
    uint16_t code = cpu.memory[cpu.pc];
    code <<= 8;
    code |= cpu.memory[cpu.pc + 1];
    printf("0x%04x\n", code);
    return snek8_opcodeInit(code);
}

enum Snek8ExecutionOutput
snek8_cpuSetKey(Snek8CPU* cpu, size_t key, bool value){
    if (!cpu){
        return SNEK8_EXECOUT_EMPTY_STRUCT;
    }
    if (value){
        cpu->keys |= (1 << key);
    }else{
        cpu->keys ^= (1 << key);
    }
    return SNEK8_EXECOUT_SUCCESS;
}

/*
* NOP
*/
enum Snek8ExecutionOutput
snek8_cpuExecutionError(Snek8CPU* cpu, Snek8Opcode opcode, char* code){
    UNUSED(cpu);
    UNUSED(opcode);
    UNUSED(code);
    return SNEK8_EXECOUT_INVALID_OPCODE;
}

/*
* CLS
*/
enum Snek8ExecutionOutput
snek8_cpuCLS(Snek8CPU* cpu, Snek8Opcode opcode, char* code){
    if (!cpu){
        return SNEK8_EXECOUT_EMPTY_STRUCT;
    }
    UNUSED(opcode);
    UNUSED(code);
    memset(&cpu->graphics, 0, SNEK8_SIZE_GRAPHICS * SIZE_U8);
    return SNEK8_EXECOUT_SUCCESS;
}

/*
* RET
*/
enum Snek8ExecutionOutput
snek8_cpuRET(Snek8CPU* cpu, Snek8Opcode opcode, char* code){
    UNUSED(opcode);
    UNUSED(code);
    if (!cpu){
        return SNEK8_EXECOUT_EMPTY_STRUCT;
    }
    if (cpu->sp == 0){
        return SNEK8_EXECOUT_STACK_EMPTY;
    }
    cpu->sp--;
    cpu->pc = cpu->stack[cpu->sp];
    return SNEK8_EXECOUT_SUCCESS;
}

/*
* CALL 0x0NNN
*/
enum Snek8ExecutionOutput
snek8_cpuCALL(Snek8CPU* cpu, Snek8Opcode opcode, char* code){
    UNUSED(code);
    if (!cpu){
        return SNEK8_EXECOUT_EMPTY_STRUCT;
    }
    if (cpu->sp == 16){
        return SNEK8_EXECOUT_STACK_OVERFLOW;
    }
    cpu->stack[cpu->sp] = cpu->pc;
    cpu->sp++;
    cpu->pc = snek8_opcodeGetAddr(opcode);
    _snek8_writeU16(code + 5, cpu->pc);
    return SNEK8_EXECOUT_SUCCESS;
}

/*
* JP 0x0NNN.
*/
enum Snek8ExecutionOutput
snek8_cpuJMP_ADDR(Snek8CPU* cpu, Snek8Opcode opcode, char* code){
    if (!cpu || !code){
        return SNEK8_EXECOUT_EMPTY_STRUCT;
    }
    cpu->pc = snek8_opcodeGetAddr(opcode);
    _snek8_writeU16(code + 3, cpu->pc);
    return SNEK8_EXECOUT_SUCCESS;
}

/*
* SE V{0xX}, 0xKK.
*/
enum Snek8ExecutionOutput
snek8_cpuSE_VX_BYTE(Snek8CPU* cpu, Snek8Opcode opcode, char* code){
    if (!cpu || !code){
        return SNEK8_EXECOUT_EMPTY_STRUCT;
    }
    uint8_t kk = snek8_opcodeGetByte(opcode);
    size_t x = opcode.iq2;
    if (cpu->registers[x] == kk){
        _snek8_cpuIncrementPC(cpu);
    }
    code[7] = _snek8_its((uint8_t) x);
    _snek8_writeU8(code + 11, kk);
    return SNEK8_EXECOUT_SUCCESS;
}

/*
* SNE V{0xX}, 0xKK.
*/
enum Snek8ExecutionOutput
snek8_cpuSNE_VX_BYTE(Snek8CPU* cpu, Snek8Opcode opcode, char* code){
    if (!cpu || !code){
        return SNEK8_EXECOUT_EMPTY_STRUCT;
    }
    uint8_t kk = snek8_opcodeGetByte(opcode);
    size_t x = opcode.iq2;
    if (cpu->registers[x] != kk){
        _snek8_cpuIncrementPC(cpu);
    }
    code[8] = _snek8_its((uint8_t) x);
    _snek8_writeU8(code + 12, kk);
    return SNEK8_EXECOUT_SUCCESS;
}

/*
* LD V{0xX}, 0xKK.
*/
enum Snek8ExecutionOutput
snek8_cpuLD_VX_BYTE(Snek8CPU* cpu, Snek8Opcode opcode, char* code){
    if (!cpu || !code){
        return SNEK8_EXECOUT_EMPTY_STRUCT;
    }
    size_t x = opcode.iq2;
    uint8_t kk = snek8_opcodeGetByte(opcode);
    cpu->registers[x] = kk;
    code[7] = _snek8_its((uint8_t) x);
    _snek8_writeU8(code + 11, kk);
    return SNEK8_EXECOUT_SUCCESS;
}

/*
* ADD V{0xX}, 0xKK.
*/
enum Snek8ExecutionOutput
snek8_cpuADD_VX_BYTE(Snek8CPU* cpu, Snek8Opcode opcode, char* code){
    if (!cpu || !code){
        return SNEK8_EXECOUT_EMPTY_STRUCT;
    }
    uint8_t kk = snek8_opcodeGetByte(opcode);
    size_t x = opcode.iq2;
    cpu->registers[x] += kk;
    code[8] = _snek8_its((uint8_t) x);
    _snek8_writeU8(code + 12, kk);
    return SNEK8_EXECOUT_SUCCESS;
}

/*
* SE V{0xX}, V{0xY}.
*/
enum Snek8ExecutionOutput
snek8_cpuSE_VX_VY(Snek8CPU* cpu, Snek8Opcode opcode, char* code){
    if (!cpu || !code){
        return SNEK8_EXECOUT_EMPTY_STRUCT;
    }
    size_t x = opcode.iq2;
    size_t y = opcode.iq1;
    if (cpu->registers[x] == cpu->registers[y]){
        _snek8_cpuIncrementPC(cpu);
    }
    code[7] = _snek8_its((uint8_t) x);
    code[15] = _snek8_its((uint8_t) y);
    return SNEK8_EXECOUT_SUCCESS;
}

/*
* SNE V{0xX}, V{0xY}.
*/
enum Snek8ExecutionOutput
snek8_cpuSNE_VX_VY(Snek8CPU* cpu, Snek8Opcode opcode, char* code){
    if (!cpu || !code){
        return SNEK8_EXECOUT_EMPTY_STRUCT;
    }
    size_t x = opcode.iq2;
    size_t y = opcode.iq1;
    if (cpu->registers[x] != cpu->registers[y]){
        _snek8_cpuIncrementPC(cpu);
    }
    code[8] = _snek8_its((uint8_t) x);
    code[16] = _snek8_its((uint8_t) y);
    return SNEK8_EXECOUT_SUCCESS;
}

/*
* LD V{0xX}, V{0xY}.
*/
enum Snek8ExecutionOutput
snek8_cpuLD_VX_VY(Snek8CPU* cpu, Snek8Opcode opcode, char* code){
    if (!cpu || !code){
        return SNEK8_EXECOUT_EMPTY_STRUCT;
    }
    size_t x = opcode.iq2;
    size_t y = opcode.iq1;
    cpu->registers[x] = cpu->registers[y];
    code[7] = _snek8_its((uint8_t) x);
    code[15] = _snek8_its((uint8_t) y);
    return SNEK8_EXECOUT_SUCCESS;
}

/*
* OR V{0xX}, V{0xY}.
*/
enum Snek8ExecutionOutput
snek8_cpuOR_VX_VY(Snek8CPU* cpu, Snek8Opcode opcode, char* code){
    if (!cpu || !code){
        return SNEK8_EXECOUT_EMPTY_STRUCT;
    }
    size_t x = opcode.iq2;
    size_t y = opcode.iq1;
    cpu->registers[x] |= cpu->registers[y];
    code[7] = _snek8_its((uint8_t) x);
    code[15] = _snek8_its((uint8_t) y);
    return SNEK8_EXECOUT_SUCCESS;
}

/*
* AND V{0xX}, V{0xY}.
*/
enum Snek8ExecutionOutput
snek8_cpuAND_VX_VY(Snek8CPU* cpu, Snek8Opcode opcode, char* code){
    if (!cpu || !code){
        return SNEK8_EXECOUT_EMPTY_STRUCT;
    }
    size_t x = opcode.iq2;
    size_t y = opcode.iq1;
    cpu->registers[x] &= cpu->registers[y];
    code[8] = _snek8_its((uint8_t) x);
    code[16] = _snek8_its((uint8_t) y);
    return SNEK8_EXECOUT_SUCCESS;
}

/*
* XOR V{0xX}, V{0xY}.
*/
enum Snek8ExecutionOutput
snek8_cpuXOR_VX_VY(Snek8CPU* cpu, Snek8Opcode opcode, char* code){
    if (!cpu || !code){
        return SNEK8_EXECOUT_EMPTY_STRUCT;
    }
    size_t x = opcode.iq2;
    size_t y = opcode.iq1;
    cpu->registers[x] ^= cpu->registers[y];
    code[8] = _snek8_its((uint8_t) x);
    code[16] = _snek8_its((uint8_t) y);
    return SNEK8_EXECOUT_SUCCESS;
}

/*
* ADD V{0xX}, V{0xY}.
*/
enum Snek8ExecutionOutput
snek8_cpuADD_VX_VY(Snek8CPU* cpu, Snek8Opcode opcode, char* code){
    if (!cpu || !code){
        return SNEK8_EXECOUT_EMPTY_STRUCT;
    }
    size_t x = opcode.iq2;
    size_t y = opcode.iq1;
    cpu->registers[0xF] = UINT8_MAX - cpu->registers[x] < cpu->registers[y]? 1:0;
    cpu->registers[x] += cpu->registers[y];
    code[8] = _snek8_its((uint8_t) x);
    code[16] = _snek8_its((uint8_t) y);
    return SNEK8_EXECOUT_SUCCESS;
}

/*
* SUB V{0xX}, V{0xY}.
*/
enum Snek8ExecutionOutput
snek8_cpuSUB_VX_VY(Snek8CPU* cpu, Snek8Opcode opcode, char* code){
    if (!cpu || !code){
        return SNEK8_EXECOUT_EMPTY_STRUCT;
    }
    size_t x = opcode.iq2;
    size_t y = opcode.iq1;
    cpu->registers[0xF] = cpu->registers[x] < cpu->registers[y]? 1:0;
    cpu->registers[x] -= cpu->registers[y];
    code[8] = _snek8_its((uint8_t) x);
    code[16] = _snek8_its((uint8_t) y);
    return SNEK8_EXECOUT_SUCCESS;
}

/*
* SUBN V{0xX}, V{0xY}.
*/
enum Snek8ExecutionOutput
snek8_cpuSUBN_VX_VY(Snek8CPU* cpu, Snek8Opcode opcode, char* code){
    if (!cpu || !code){
        return SNEK8_EXECOUT_EMPTY_STRUCT;
    }
    size_t x = opcode.iq2;
    size_t y = opcode.iq1;
    cpu->registers[0xF] = cpu->registers[x] > cpu->registers[y]? 1:0;
    cpu->registers[x] = cpu->registers[y] - cpu->registers[x];
    code[9] = _snek8_its((uint8_t) x);
    code[17] = _snek8_its((uint8_t) y);
    return SNEK8_EXECOUT_SUCCESS;
}

/*
* SHR V{0xX}, V{0xY}.
*/
enum Snek8ExecutionOutput
snek8_cpuSHR_VX_VY(Snek8CPU* cpu, Snek8Opcode opcode, char* code){
    if (!cpu || !code){
        return SNEK8_EXECOUT_EMPTY_STRUCT;
    }
    size_t x = opcode.iq2;
    size_t y = opcode.iq1;
    if ((cpu->implm_flags & SNEK8_IMPLM_MODE_SHIFTS_USE_VY) > 0){
            cpu->registers[x] = cpu->registers[y];
    }
    cpu->registers[0xF] = (cpu->registers[x] & 0x1) != 0? 1:0;
    cpu->registers[x] >>= 1;
    code[8] = _snek8_its((uint8_t) x);
    code[16] = _snek8_its((uint8_t) y);
    return SNEK8_EXECOUT_SUCCESS;
}

/*
* SHL V{0xX}, V{0xY}.
*/
enum Snek8ExecutionOutput
snek8_cpuSHL_VX_VY(Snek8CPU* cpu, Snek8Opcode opcode, char* code){
    if (!cpu || !code){
        return SNEK8_EXECOUT_EMPTY_STRUCT;
    }
    size_t x = opcode.iq2;
    size_t y = opcode.iq1;
    if ((cpu->implm_flags & SNEK8_IMPLM_MODE_SHIFTS_USE_VY) > 0){
            cpu->registers[x] = cpu->registers[y];
    }
    cpu->registers[0xF] = (cpu->registers[x] & 0x80) != 0? 1:0;
    cpu->registers[x] <<= 1;
    code[8] = _snek8_its((uint8_t) x);
    code[16] = _snek8_its((uint8_t) y);
    return SNEK8_EXECOUT_SUCCESS;
}

/*
* LD I, 0x0NNN.
*/
enum Snek8ExecutionOutput
snek8_cpuLD_I_ADDR(Snek8CPU* cpu, Snek8Opcode opcode, char* code){
    if (!cpu || !code){
        return SNEK8_EXECOUT_EMPTY_STRUCT;
    }
    cpu->ir = snek8_opcodeGetAddr(opcode);
    _snek8_writeU16(code + 6, cpu->ir);
    return SNEK8_EXECOUT_SUCCESS;
}

/*
* JP V{0x0}, 0x0NNN.
*/
enum Snek8ExecutionOutput
snek8_cpuJP_V0_ADDR(Snek8CPU* cpu, Snek8Opcode opcode, char* code){
    if (!cpu || !code){
        return SNEK8_EXECOUT_EMPTY_STRUCT;
    }
    uint16_t addr = snek8_opcodeGetAddr(opcode);
    size_t x = opcode.iq2;
    if ((cpu->implm_flags & SNEK8_IMPLM_MODE_BNNN_USE_VX) > 0){
            cpu->pc = addr + cpu->registers[x];
            code[7] = _snek8_its((uint8_t) x);
    }else {
            cpu->pc = addr + cpu->registers[0];
    }
    _snek8_writeU16(code + 11, addr);
    return SNEK8_EXECOUT_SUCCESS;
}

/*
* RND V{0xX}, 0xKK.
*/
enum Snek8ExecutionOutput
snek8_cpuRND_VX_BYTE(Snek8CPU* cpu, Snek8Opcode opcode, char* code){
    if (!cpu || !code){
        return SNEK8_EXECOUT_EMPTY_STRUCT;
    }
    srand(time(NULL));
    size_t x = opcode.iq2;
    uint8_t kk = snek8_opcodeGetByte(opcode);
    cpu->registers[x] = rand() & kk;
    code[8] = _snek8_its((uint8_t) x);
    _snek8_writeU8(code + 12, kk);
    return SNEK8_EXECOUT_SUCCESS;
}

static inline uint8_t*
_snek8_cpuGetPixel(Snek8CPU* cpu, size_t x, size_t y){
    size_t idx_x = x & 63;
    size_t idx_y = y & 31;
    return &cpu->graphics[idx_y * SNEK8_GRAPHICS_WIDTH + idx_x];
}
/*
* DRW V{0xX}, V{0xY}, 0xN.
*/
enum Snek8ExecutionOutput
snek8_cpuDRW_VX_VY_N(Snek8CPU* cpu, Snek8Opcode opcode, char* code){
    if (!cpu || !code){
        return SNEK8_EXECOUT_EMPTY_STRUCT;
    }
    cpu->registers[0xF] = 0;
    uint8_t px = cpu->registers[opcode.iq2];
    uint8_t py = cpu->registers[opcode.iq1];
    for (uint8_t col = 0; col < opcode.lsq; col++){
        uint8_t byte = cpu->memory[cpu->ir + col];
        for (uint8_t row = 0; row < 8; row++){
            uint8_t bit = byte & (0x80u >> row);
            uint8_t* pixel_ptr = _snek8_cpuGetPixel(cpu, px + row, py + col);
            *pixel_ptr ^= bit;
            if (bit && *pixel_ptr == 0){
                cpu->registers[0xF] = 1;
            }
        }
    }
    code[8] = _snek8_its(opcode.iq2);
    code[16] = _snek8_its(opcode.iq1);
    code[22] = _snek8_its(opcode.lsq);
    return SNEK8_EXECOUT_SUCCESS;
}

/*
* SKP V{0xX}.
*/
enum Snek8ExecutionOutput
snek8_cpuSKP_VX(Snek8CPU* cpu, Snek8Opcode opcode, char* code){
    if (!cpu || !code){
        return SNEK8_EXECOUT_EMPTY_STRUCT;
    }
    size_t x = opcode.iq2;
    size_t key = cpu->registers[x];
    if (snek8_cpuGetKeyVal(*cpu, key)){
        _snek8_cpuIncrementPC(cpu);
    }
    code[8] = _snek8_its((uint8_t) x);
    return SNEK8_EXECOUT_SUCCESS;
}

/*
* SKNP V{0xX}.
*/
enum Snek8ExecutionOutput
snek8_cpuSKNP_VX(Snek8CPU* cpu, Snek8Opcode opcode, char* code){
    if (!cpu || !code){
        return SNEK8_EXECOUT_EMPTY_STRUCT;
    }
    size_t x = opcode.iq2;
    size_t key = cpu->registers[x];
    if (!snek8_cpuGetKeyVal(*cpu, key)){
        _snek8_cpuIncrementPC(cpu);
    }
    code[9] = _snek8_its((uint8_t) x);
    return SNEK8_EXECOUT_SUCCESS;
}

/*
* LD V{0xX}, DT.
*/
enum Snek8ExecutionOutput
snek8_cpuLD_VX_DT(Snek8CPU* cpu, Snek8Opcode opcode, char* code){
    if (!cpu || !code){
        return SNEK8_EXECOUT_EMPTY_STRUCT;
    }
    size_t x = opcode.iq2;
    cpu->registers[x] = cpu->dt;
    code[7] = _snek8_its((uint8_t) x);
    return SNEK8_EXECOUT_SUCCESS;
}

/*
* LD V{0xX}, K{0xK}.
*/
enum Snek8ExecutionOutput
snek8_cpuLD_VX_K(Snek8CPU* cpu, Snek8Opcode opcode, char* code){
    if (!cpu || !code){
        return SNEK8_EXECOUT_EMPTY_STRUCT;
    }
    bool key_found = false;
    size_t x = opcode.iq2;
    size_t key_index = 0;
    while (key_index < 16){
        if (snek8_cpuGetKeyVal(*cpu, key_index)){
            cpu->registers[x] = (uint8_t) key_index;
            key_found = true;
            break;
        }
        key_index++;
    }
    if (!key_found){
        _snek8_cpuDecrementPC(cpu);
        code[7] = _snek8_its((uint8_t) x);
        code[15] = '-';
        return SNEK8_EXECOUT_SUCCESS;
    }
    code[7] = _snek8_its((uint8_t) x);
    code[15] = _snek8_its((uint8_t) key_index);
    return SNEK8_EXECOUT_SUCCESS;
}

/*
* LD DT, V{0xX}.
*/
enum Snek8ExecutionOutput
snek8_cpuLD_DT_VX(Snek8CPU* cpu, Snek8Opcode opcode, char* code){
    if (!cpu || !code){
        return SNEK8_EXECOUT_EMPTY_STRUCT;
    }
    size_t x = opcode.iq2;
    cpu->dt = cpu->registers[x];
    code[11] = _snek8_its((uint8_t) x);
    return SNEK8_EXECOUT_SUCCESS;
}

/*
* LD ST, V{0xX}.
*/
enum Snek8ExecutionOutput
snek8_cpuLD_ST_VX(Snek8CPU* cpu, Snek8Opcode opcode, char* code){
    if (!cpu || !code){
        return SNEK8_EXECOUT_EMPTY_STRUCT;
    }
    size_t x = opcode.iq2;
    cpu->st = cpu->registers[x];
    code[11] = _snek8_its((uint8_t) x);
    return SNEK8_EXECOUT_SUCCESS;
}

/*
* ADD I, V{0xX}.
*/
enum Snek8ExecutionOutput
snek8_cpuADD_I_VX(Snek8CPU* cpu, Snek8Opcode opcode, char* code){
    if (!cpu || !code){
        return SNEK8_EXECOUT_EMPTY_STRUCT;
    }
    size_t x = opcode.iq2;
    cpu->ir += cpu->registers[x];
    code[11] = _snek8_its((uint8_t) x);
    return SNEK8_EXECOUT_SUCCESS;
}

/*
* LD F, V{0xX}.
*/
enum Snek8ExecutionOutput
snek8_cpuLD_F_VX(Snek8CPU* cpu, Snek8Opcode opcode, char* code){
    if (!cpu || !code){
        return SNEK8_EXECOUT_EMPTY_STRUCT;
    }
    size_t x = opcode.iq2;
    cpu->ir = SNEK8_MEM_ADDR_FONTSET_START + (SNEK8_SIZE_FONTSET_PIXEL_PER_SPRITE * cpu->registers[x]);
    code[10] = _snek8_its((uint8_t) x);
    return SNEK8_EXECOUT_SUCCESS;
}

/*
* LD B, V{0xX}.
*/
enum Snek8ExecutionOutput
snek8_cpuLD_B_VX(Snek8CPU* cpu, Snek8Opcode opcode, char* code){
    if (!cpu || !code){
        return SNEK8_EXECOUT_EMPTY_STRUCT;
    }
    size_t x = opcode.iq2;
    uint8_t value = cpu->registers[x];
    cpu->memory[cpu->ir + 2] = value % 10;
    value /= 10;
    cpu->memory[cpu->ir + 1] = value % 10;
    value /= 10;
    cpu->memory[cpu->ir] = value % 10;
    code[10] = _snek8_its((uint8_t) x);
    return SNEK8_EXECOUT_SUCCESS;
}

/*
* LD [I], V{0xX}.
*/
enum Snek8ExecutionOutput
snek8_cpuLD_I_V0_VX(Snek8CPU* cpu, Snek8Opcode opcode, char* code){
    if (!cpu || !code){
        return SNEK8_EXECOUT_EMPTY_STRUCT;
    }
    size_t x = opcode.iq2;
    if ((cpu->implm_flags & SNEK8_IMPLM_MODE_FX_CHANGE_I)){
        for (size_t i=0; i<=x; i++){
            cpu->memory[cpu->ir + i] = cpu->registers[i];
            cpu->ir++;
        }
    }else {
        for (size_t i=0; i<=x; i++){
            cpu->memory[cpu->ir + i] = cpu->registers[i];
        }
    }
    code[12] = _snek8_its((uint8_t) x);
    return SNEK8_EXECOUT_SUCCESS;
}

/*
* LD V{0xX}, [I].
*/
enum Snek8ExecutionOutput
snek8_cpuLD_VX_V0_I(Snek8CPU* cpu, Snek8Opcode opcode, char* code){
    if (!cpu || !code){
        return SNEK8_EXECOUT_EMPTY_STRUCT;
    }
    size_t x = opcode.iq2;
    if ((cpu->implm_flags & SNEK8_IMPLM_MODE_FX_CHANGE_I)){
        for (size_t i=0; i<=x; i++){
            cpu->registers[i] = cpu->registers[cpu->ir + i];
            cpu->ir++;
        }
    }else {
        for (size_t i=0; i<=x; i++){
            cpu->registers[i] = cpu->registers[cpu->ir + i];
        }
    }
    code[7] = _snek8_its((uint8_t) x);
    return SNEK8_EXECOUT_SUCCESS;
}

Snek8Instruction
snek8_opcodeDecode(Snek8Opcode opcode){
    Snek8Instruction instruction;
    switch (opcode.msq) {
        case 0x0:
            switch (opcode.lsq) {
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
            switch (opcode.lsq) {
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
            switch (opcode.lsq) {
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
            switch (opcode.lsq) {
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
                    switch (opcode.iq1) {
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
    Snek8Opcode opcode = _snek8_cpuGetOpcode(*cpu);
    _snek8_cpuIncrementPC(cpu);
    *instruction = snek8_opcodeDecode(opcode);
    _snek8_cpuTickTimers(cpu);
    return instruction->exec(cpu, opcode, instruction->code);
}

#ifdef __cplusplus
    }
#endif
#endif // SNEK8_CPU_C
