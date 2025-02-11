/**
* @file cpu.c
* @author Paulo Arruda
* @license GPL-3
* @brief Implementation of the Chip8's CPU and related emulation routines.
*/
#ifndef PY8_CPU_C
    #define PY8_CPU_C
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
_py8_its(uint8_t nibble){
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
_py8_writeU8(char* code, uint8_t byte){
    code[2] = _py8_its((byte >> 4) & 0xF);
    code[3] = _py8_its(byte & 0xF);
}

/**
* @brief Converts a u16 to its hexadecimal form.
*
* @param[in, out] `code`.
* @param[in] `hex`.
*/
static inline void
_py8_writeU16(char* code, uint16_t hex){
    code[2] = _py8_its((hex >> 12) & 0xF);
    code[3] = _py8_its((hex >> 8) & 0xF);
    code[4] = _py8_its((hex >> 4) & 0xF);
    code[5] = _py8_its(hex & 0xF);
}

Py8Opcode
py8_opcodeInit(uint16_t code){
    return (Py8Opcode){
        .lsq = code & 0x000F,
        .iq1 = (code & 0x00F0) >> 4,
        .iq2 = (code & 0x0F00) >> 8,
        .msq = (code & 0xF000) >> 12,
    };
}

uint8_t
py8_opcodeGetByte(Py8Opcode opcode){
    return (opcode.iq1 << 4) | (opcode.lsq);
}

uint16_t
py8_opcodeGetAddr(Py8Opcode opcode){
    return (((uint16_t) opcode.iq2) << 8) |
           (((uint16_t) opcode.iq1) << 4) |
           ((uint16_t) opcode.lsq);
}

enum Py8ExecutionOutput
py8_cpuInit(Py8CPU* cpu, uint8_t implm_flags){
    if (!cpu){
        return PY8_EXECOUT_EMPTY_STRUCT;
    }
    cpu->implm_flags = implm_flags;
    uint8_t fontset[PY8_SIZE_FONTSET_PIXELS] = {
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
    cpu->pc = PY8_MEM_ADDR_PROG_START;
    cpu->ir = 0;
    cpu->sp = 0;
    cpu->dt = 0;
    cpu->sp = 0;
    memset(&cpu->memory, 0, PY8_SIZE_RAM * SIZE_U8);
    memcpy(cpu->memory + PY8_MEM_ADDR_FONTSET_START, fontset, PY8_SIZE_FONTSET_PIXELS * SIZE_U8);
    memset(&cpu->graphics, 0, PY8_SIZE_GRAPHICS * SIZE_U8);
    memset(&cpu->registers, 0, PY8_SIZE_REGISTERS * SIZE_U8);
    memset(&cpu->stack, 0, PY8_SIZE_STACK * SIZE_U16);
    return PY8_EXECOUT_SUCCESS;
}

enum Py8ExecutionOutput
py8_cpuLoadRom(Py8CPU* cpu, const char* rom_file_path){
    if (!cpu){
        return PY8_EXECOUT_EMPTY_STRUCT;
    }else if (!rom_file_path){
        return PY8_EXECOUT_ROM_FILE_INVALID;
    }
    FILE* rom_file = fopen(rom_file_path, "rb");
    if (!rom_file_path){
        return PY8_EXECOUT_ROM_FILE_FAILED_TO_OPEN;
    }
    (void) fseek(rom_file, 0, SEEK_END);
    size_t rom_size = ftell(rom_file);
    if (rom_size > PY8_SIZE_MAX_ROM_FILE){
        fclose(rom_file);
        return PY8_EXECOUT_ROM_FILE_EXCEEDS_MAX_MEM;
    }
    rewind(rom_file);
    size_t nread = fread(cpu->memory + PY8_MEM_ADDR_PROG_START, 1, rom_size, rom_file);
    if (nread != rom_size){
        fclose(rom_file);
        return PY8_EXECOUT_ROM_FILE_FAILED_TO_READ;
    }
    fclose(rom_file);
    // for (size_t i=PY8_MEM_ADDR_PROG_START; i<PY8_SIZE_RAM; i++){
    //     printf("0x%02x ", cpu->memory[i]);
    // }
    return PY8_EXECOUT_SUCCESS;
}
/**
* @brief Manages the CPU timers.
*
* @param `cpu`.
*/
static inline void
_py8_cpuTickTimers(Py8CPU* cpu){
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
_py8_cpuIncrementPC(Py8CPU* cpu){
    cpu->pc += 2;
}

static inline void
_py8_cpuDecrementPC(Py8CPU* cpu){
    cpu->pc -= 2;
}

/**
* @brief Retrieves the opcode from memory.
*
* @param `cpu`.
* @return The opcode specified by the emulation process.
*/
static Py8Opcode
_py8_cpuGetOpcode(Py8CPU cpu){
    uint16_t code = cpu.memory[cpu.pc];
    code <<= 8;
    code |= cpu.memory[cpu.pc + 1];
    printf("0x%04x\n", code);
    return py8_opcodeInit(code);
}

enum Py8ExecutionOutput
py8_cpuSetKey(Py8CPU* cpu, size_t key, bool value){
    if (!cpu){
        return PY8_EXECOUT_EMPTY_STRUCT;
    }
    if (value){
        cpu->keys |= (1 << key);
    }else{
        cpu->keys ^= (1 << key);
    }
    return PY8_EXECOUT_SUCCESS;
}

/*
* NOP
*/
enum Py8ExecutionOutput
py8_cpuExecutionError(Py8CPU* cpu, Py8Opcode opcode, char* code){
    UNUSED(cpu);
    UNUSED(opcode);
    UNUSED(code);
    return PY8_EXECOUT_INVALID_OPCODE;
}

/*
* CLS
*/
enum Py8ExecutionOutput
py8_cpuCLS(Py8CPU* cpu, Py8Opcode opcode, char* code){
    if (!cpu){
        return PY8_EXECOUT_EMPTY_STRUCT;
    }
    UNUSED(opcode);
    UNUSED(code);
    memset(&cpu->graphics, 0, PY8_SIZE_GRAPHICS * SIZE_U8);
    return PY8_EXECOUT_SUCCESS;
}

/*
* RET
*/
enum Py8ExecutionOutput
py8_cpuRET(Py8CPU* cpu, Py8Opcode opcode, char* code){
    UNUSED(opcode);
    UNUSED(code);
    if (!cpu){
        return PY8_EXECOUT_EMPTY_STRUCT;
    }
    if (cpu->sp == 0){
        return PY8_EXECOUT_STACK_EMPTY;
    }
    cpu->sp--;
    cpu->pc = cpu->stack[cpu->sp];
    return PY8_EXECOUT_SUCCESS;
}

/*
* CALL 0x0NNN
*/
enum Py8ExecutionOutput
py8_cpuCALL(Py8CPU* cpu, Py8Opcode opcode, char* code){
    UNUSED(code);
    if (!cpu){
        return PY8_EXECOUT_EMPTY_STRUCT;
    }
    if (cpu->sp == 16){
        return PY8_EXECOUT_STACK_OVERFLOW;
    }
    cpu->stack[cpu->sp] = cpu->pc;
    cpu->sp++;
    cpu->pc = py8_opcodeGetAddr(opcode);
    _py8_writeU16(code + 5, cpu->pc);
    return PY8_EXECOUT_SUCCESS;
}

/*
* JP 0x0NNN.
*/
enum Py8ExecutionOutput
py8_cpuJMP_ADDR(Py8CPU* cpu, Py8Opcode opcode, char* code){
    if (!cpu || !code){
        return PY8_EXECOUT_EMPTY_STRUCT;
    }
    cpu->pc = py8_opcodeGetAddr(opcode);
    _py8_writeU16(code + 3, cpu->pc);
    return PY8_EXECOUT_SUCCESS;
}

/*
* SE V{0xX}, 0xKK.
*/
enum Py8ExecutionOutput
py8_cpuSE_VX_BYTE(Py8CPU* cpu, Py8Opcode opcode, char* code){
    if (!cpu || !code){
        return PY8_EXECOUT_EMPTY_STRUCT;
    }
    uint8_t kk = py8_opcodeGetByte(opcode);
    size_t x = opcode.iq2;
    if (cpu->registers[x] == kk){
        _py8_cpuIncrementPC(cpu);
    }
    code[7] = _py8_its((uint8_t) x);
    _py8_writeU8(code + 11, kk);
    return PY8_EXECOUT_SUCCESS;
}

/*
* SNE V{0xX}, 0xKK.
*/
enum Py8ExecutionOutput
py8_cpuSNE_VX_BYTE(Py8CPU* cpu, Py8Opcode opcode, char* code){
    if (!cpu || !code){
        return PY8_EXECOUT_EMPTY_STRUCT;
    }
    uint8_t kk = py8_opcodeGetByte(opcode);
    size_t x = opcode.iq2;
    if (cpu->registers[x] != kk){
        _py8_cpuIncrementPC(cpu);
    }
    code[8] = _py8_its((uint8_t) x);
    _py8_writeU8(code + 12, kk);
    return PY8_EXECOUT_SUCCESS;
}

/*
* LD V{0xX}, 0xKK.
*/
enum Py8ExecutionOutput
py8_cpuLD_VX_BYTE(Py8CPU* cpu, Py8Opcode opcode, char* code){
    if (!cpu || !code){
        return PY8_EXECOUT_EMPTY_STRUCT;
    }
    size_t x = opcode.iq2;
    uint8_t kk = py8_opcodeGetByte(opcode);
    cpu->registers[x] = kk;
    code[7] = _py8_its((uint8_t) x);
    _py8_writeU8(code + 11, kk);
    return PY8_EXECOUT_SUCCESS;
}

/*
* ADD V{0xX}, 0xKK.
*/
enum Py8ExecutionOutput
py8_cpuADD_VX_BYTE(Py8CPU* cpu, Py8Opcode opcode, char* code){
    if (!cpu || !code){
        return PY8_EXECOUT_EMPTY_STRUCT;
    }
    uint8_t kk = py8_opcodeGetByte(opcode);
    size_t x = opcode.iq2;
    cpu->registers[x] += kk;
    code[8] = _py8_its((uint8_t) x);
    _py8_writeU8(code + 12, kk);
    return PY8_EXECOUT_SUCCESS;
}

/*
* SE V{0xX}, V{0xY}.
*/
enum Py8ExecutionOutput
py8_cpuSE_VX_VY(Py8CPU* cpu, Py8Opcode opcode, char* code){
    if (!cpu || !code){
        return PY8_EXECOUT_EMPTY_STRUCT;
    }
    size_t x = opcode.iq2;
    size_t y = opcode.iq1;
    if (cpu->registers[x] == cpu->registers[y]){
        _py8_cpuIncrementPC(cpu);
    }
    code[7] = _py8_its((uint8_t) x);
    code[15] = _py8_its((uint8_t) y);
    return PY8_EXECOUT_SUCCESS;
}

/*
* SNE V{0xX}, V{0xY}.
*/
enum Py8ExecutionOutput
py8_cpuSNE_VX_VY(Py8CPU* cpu, Py8Opcode opcode, char* code){
    if (!cpu || !code){
        return PY8_EXECOUT_EMPTY_STRUCT;
    }
    size_t x = opcode.iq2;
    size_t y = opcode.iq1;
    if (cpu->registers[x] != cpu->registers[y]){
        _py8_cpuIncrementPC(cpu);
    }
    code[8] = _py8_its((uint8_t) x);
    code[16] = _py8_its((uint8_t) y);
    return PY8_EXECOUT_SUCCESS;
}

/*
* LD V{0xX}, V{0xY}.
*/
enum Py8ExecutionOutput
py8_cpuLD_VX_VY(Py8CPU* cpu, Py8Opcode opcode, char* code){
    if (!cpu || !code){
        return PY8_EXECOUT_EMPTY_STRUCT;
    }
    size_t x = opcode.iq2;
    size_t y = opcode.iq1;
    cpu->registers[x] = cpu->registers[y];
    code[7] = _py8_its((uint8_t) x);
    code[15] = _py8_its((uint8_t) y);
    return PY8_EXECOUT_SUCCESS;
}

/*
* OR V{0xX}, V{0xY}.
*/
enum Py8ExecutionOutput
py8_cpuOR_VX_VY(Py8CPU* cpu, Py8Opcode opcode, char* code){
    if (!cpu || !code){
        return PY8_EXECOUT_EMPTY_STRUCT;
    }
    size_t x = opcode.iq2;
    size_t y = opcode.iq1;
    cpu->registers[x] |= cpu->registers[y];
    code[7] = _py8_its((uint8_t) x);
    code[15] = _py8_its((uint8_t) y);
    return PY8_EXECOUT_SUCCESS;
}

/*
* AND V{0xX}, V{0xY}.
*/
enum Py8ExecutionOutput
py8_cpuAND_VX_VY(Py8CPU* cpu, Py8Opcode opcode, char* code){
    if (!cpu || !code){
        return PY8_EXECOUT_EMPTY_STRUCT;
    }
    size_t x = opcode.iq2;
    size_t y = opcode.iq1;
    cpu->registers[x] &= cpu->registers[y];
    code[8] = _py8_its((uint8_t) x);
    code[16] = _py8_its((uint8_t) y);
    return PY8_EXECOUT_SUCCESS;
}

/*
* XOR V{0xX}, V{0xY}.
*/
enum Py8ExecutionOutput
py8_cpuXOR_VX_VY(Py8CPU* cpu, Py8Opcode opcode, char* code){
    if (!cpu || !code){
        return PY8_EXECOUT_EMPTY_STRUCT;
    }
    size_t x = opcode.iq2;
    size_t y = opcode.iq1;
    cpu->registers[x] ^= cpu->registers[y];
    code[8] = _py8_its((uint8_t) x);
    code[16] = _py8_its((uint8_t) y);
    return PY8_EXECOUT_SUCCESS;
}

/*
* ADD V{0xX}, V{0xY}.
*/
enum Py8ExecutionOutput
py8_cpuADD_VX_VY(Py8CPU* cpu, Py8Opcode opcode, char* code){
    if (!cpu || !code){
        return PY8_EXECOUT_EMPTY_STRUCT;
    }
    size_t x = opcode.iq2;
    size_t y = opcode.iq1;
    cpu->registers[0xF] = UINT8_MAX - cpu->registers[x] < cpu->registers[y]? 1:0;
    cpu->registers[x] += cpu->registers[y];
    code[8] = _py8_its((uint8_t) x);
    code[16] = _py8_its((uint8_t) y);
    return PY8_EXECOUT_SUCCESS;
}

/*
* SUB V{0xX}, V{0xY}.
*/
enum Py8ExecutionOutput
py8_cpuSUB_VX_VY(Py8CPU* cpu, Py8Opcode opcode, char* code){
    if (!cpu || !code){
        return PY8_EXECOUT_EMPTY_STRUCT;
    }
    size_t x = opcode.iq2;
    size_t y = opcode.iq1;
    cpu->registers[0xF] = cpu->registers[x] < cpu->registers[y]? 1:0;
    cpu->registers[x] -= cpu->registers[y];
    code[8] = _py8_its((uint8_t) x);
    code[16] = _py8_its((uint8_t) y);
    return PY8_EXECOUT_SUCCESS;
}

/*
* SUBN V{0xX}, V{0xY}.
*/
enum Py8ExecutionOutput
py8_cpuSUBN_VX_VY(Py8CPU* cpu, Py8Opcode opcode, char* code){
    if (!cpu || !code){
        return PY8_EXECOUT_EMPTY_STRUCT;
    }
    size_t x = opcode.iq2;
    size_t y = opcode.iq1;
    cpu->registers[0xF] = cpu->registers[x] > cpu->registers[y]? 1:0;
    cpu->registers[x] = cpu->registers[y] - cpu->registers[x];
    code[9] = _py8_its((uint8_t) x);
    code[17] = _py8_its((uint8_t) y);
    return PY8_EXECOUT_SUCCESS;
}

/*
* SHR V{0xX}, V{0xY}.
*/
enum Py8ExecutionOutput
py8_cpuSHR_VX_VY(Py8CPU* cpu, Py8Opcode opcode, char* code){
    if (!cpu || !code){
        return PY8_EXECOUT_EMPTY_STRUCT;
    }
    size_t x = opcode.iq2;
    size_t y = opcode.iq1;
    if ((cpu->implm_flags & PY8_IMPLM_MODE_SHIFTS_USE_VY) > 0){
            cpu->registers[x] = cpu->registers[y];
    }
    cpu->registers[0xF] = (cpu->registers[x] & 0x1) != 0? 1:0;
    cpu->registers[x] >>= 1;
    code[8] = _py8_its((uint8_t) x);
    code[16] = _py8_its((uint8_t) y);
    return PY8_EXECOUT_SUCCESS;
}

/*
* SHL V{0xX}, V{0xY}.
*/
enum Py8ExecutionOutput
py8_cpuSHL_VX_VY(Py8CPU* cpu, Py8Opcode opcode, char* code){
    if (!cpu || !code){
        return PY8_EXECOUT_EMPTY_STRUCT;
    }
    size_t x = opcode.iq2;
    size_t y = opcode.iq1;
    if ((cpu->implm_flags & PY8_IMPLM_MODE_SHIFTS_USE_VY) > 0){
            cpu->registers[x] = cpu->registers[y];
    }
    cpu->registers[0xF] = (cpu->registers[x] & 0x80) != 0? 1:0;
    cpu->registers[x] <<= 1;
    code[8] = _py8_its((uint8_t) x);
    code[16] = _py8_its((uint8_t) y);
    return PY8_EXECOUT_SUCCESS;
}

/*
* LD I, 0x0NNN.
*/
enum Py8ExecutionOutput
py8_cpuLD_I_ADDR(Py8CPU* cpu, Py8Opcode opcode, char* code){
    if (!cpu || !code){
        return PY8_EXECOUT_EMPTY_STRUCT;
    }
    cpu->ir = py8_opcodeGetAddr(opcode);
    _py8_writeU16(code + 6, cpu->ir);
    return PY8_EXECOUT_SUCCESS;
}

/*
* JP V{0x0}, 0x0NNN.
*/
enum Py8ExecutionOutput
py8_cpuJP_V0_ADDR(Py8CPU* cpu, Py8Opcode opcode, char* code){
    if (!cpu || !code){
        return PY8_EXECOUT_EMPTY_STRUCT;
    }
    uint16_t addr = py8_opcodeGetAddr(opcode);
    size_t x = opcode.iq2;
    if ((cpu->implm_flags & PY8_IMPLM_MODE_BNNN_USE_VX) > 0){
            cpu->pc = addr + cpu->registers[x];
            code[7] = _py8_its((uint8_t) x);
    }else {
            cpu->pc = addr + cpu->registers[0];
    }
    _py8_writeU16(code + 11, addr);
    return PY8_EXECOUT_SUCCESS;
}

/*
* RND V{0xX}, 0xKK.
*/
enum Py8ExecutionOutput
py8_cpuRND_VX_BYTE(Py8CPU* cpu, Py8Opcode opcode, char* code){
    if (!cpu || !code){
        return PY8_EXECOUT_EMPTY_STRUCT;
    }
    srand(time(NULL));
    size_t x = opcode.iq2;
    uint8_t kk = py8_opcodeGetByte(opcode);
    cpu->registers[x] = rand() & kk;
    code[8] = _py8_its((uint8_t) x);
    _py8_writeU8(code + 12, kk);
    return PY8_EXECOUT_SUCCESS;
}

static inline uint8_t*
_py8_cpuGetPixel(Py8CPU* cpu, size_t x, size_t y){
    size_t idx_x = x & 63;
    size_t idx_y = y & 31;
    return &cpu->graphics[idx_y * PY8_GRAPHICS_WIDTH + idx_x];
}
/*
* DRW V{0xX}, V{0xY}, 0xN.
*/
enum Py8ExecutionOutput
py8_cpuDRW_VX_VY_N(Py8CPU* cpu, Py8Opcode opcode, char* code){
    if (!cpu || !code){
        return PY8_EXECOUT_EMPTY_STRUCT;
    }
    cpu->registers[0xF] = 0;
    uint8_t px = cpu->registers[opcode.iq2];
    uint8_t py = cpu->registers[opcode.iq1];
    for (uint8_t col = 0; col < opcode.lsq; col++){
        uint8_t byte = cpu->memory[cpu->ir + col];
        for (uint8_t row = 0; row < 8; row++){
            uint8_t bit = byte & (0x80u >> row);
            uint8_t* pixel_ptr = _py8_cpuGetPixel(cpu, px + row, py + col);
            *pixel_ptr ^= bit;
            if (bit && *pixel_ptr == 0){
                cpu->registers[0xF] = 1;
            }
        }
    }
    code[8] = _py8_its(opcode.iq2);
    code[16] = _py8_its(opcode.iq1);
    code[22] = _py8_its(opcode.lsq);
    return PY8_EXECOUT_SUCCESS;
}

/*
* SKP V{0xX}.
*/
enum Py8ExecutionOutput
py8_cpuSKP_VX(Py8CPU* cpu, Py8Opcode opcode, char* code){
    if (!cpu || !code){
        return PY8_EXECOUT_EMPTY_STRUCT;
    }
    size_t x = opcode.iq2;
    size_t key = cpu->registers[x];
    if (py8_cpuGetKeyVal(*cpu, key)){
        _py8_cpuIncrementPC(cpu);
    }
    code[8] = _py8_its((uint8_t) x);
    return PY8_EXECOUT_SUCCESS;
}

/*
* SKNP V{0xX}.
*/
enum Py8ExecutionOutput
py8_cpuSKNP_VX(Py8CPU* cpu, Py8Opcode opcode, char* code){
    if (!cpu || !code){
        return PY8_EXECOUT_EMPTY_STRUCT;
    }
    size_t x = opcode.iq2;
    size_t key = cpu->registers[x];
    if (!py8_cpuGetKeyVal(*cpu, key)){
        _py8_cpuIncrementPC(cpu);
    }
    code[9] = _py8_its((uint8_t) x);
    return PY8_EXECOUT_SUCCESS;
}

/*
* LD V{0xX}, DT.
*/
enum Py8ExecutionOutput
py8_cpuLD_VX_DT(Py8CPU* cpu, Py8Opcode opcode, char* code){
    if (!cpu || !code){
        return PY8_EXECOUT_EMPTY_STRUCT;
    }
    size_t x = opcode.iq2;
    cpu->registers[x] = cpu->dt;
    code[7] = _py8_its((uint8_t) x);
    return PY8_EXECOUT_SUCCESS;
}

/*
* LD V{0xX}, K{0xK}.
*/
enum Py8ExecutionOutput
py8_cpuLD_VX_K(Py8CPU* cpu, Py8Opcode opcode, char* code){
    if (!cpu || !code){
        return PY8_EXECOUT_EMPTY_STRUCT;
    }
    bool has_ld = false;
    size_t x = opcode.iq2;
    size_t key_index = 0;
    while (key_index < 16){
        if (py8_cpuGetKeyVal(*cpu, key_index)){
            cpu->registers[x] = (uint8_t) key_index;
            has_ld = true;
            break;
        }
        key_index++;
    }
    if (!has_ld){
        _py8_cpuDecrementPC(cpu);
        code[7] = _py8_its((uint8_t) x);
        code[15] = '-';
        return PY8_EXECOUT_SUCCESS;
    }
    code[7] = _py8_its((uint8_t) x);
    code[15] = _py8_its((uint8_t) key_index);
    return PY8_EXECOUT_SUCCESS;
}

/*
* LD DT, V{0xX}.
*/
enum Py8ExecutionOutput
py8_cpuLD_DT_VX(Py8CPU* cpu, Py8Opcode opcode, char* code){
    if (!cpu || !code){
        return PY8_EXECOUT_EMPTY_STRUCT;
    }
    size_t x = opcode.iq2;
    cpu->dt = cpu->registers[x];
    code[11] = _py8_its((uint8_t) x);
    return PY8_EXECOUT_SUCCESS;
}

/*
* LD ST, V{0xX}.
*/
enum Py8ExecutionOutput
py8_cpuLD_ST_VX(Py8CPU* cpu, Py8Opcode opcode, char* code){
    if (!cpu || !code){
        return PY8_EXECOUT_EMPTY_STRUCT;
    }
    size_t x = opcode.iq2;
    cpu->st = cpu->registers[x];
    code[11] = _py8_its((uint8_t) x);
    return PY8_EXECOUT_SUCCESS;
}

/*
* ADD I, V{0xX}.
*/
enum Py8ExecutionOutput
py8_cpuADD_I_VX(Py8CPU* cpu, Py8Opcode opcode, char* code){
    if (!cpu || !code){
        return PY8_EXECOUT_EMPTY_STRUCT;
    }
    size_t x = opcode.iq2;
    cpu->ir += cpu->registers[x];
    code[11] = _py8_its((uint8_t) x);
    return PY8_EXECOUT_SUCCESS;
}

/*
* LD F, V{0xX}.
*/
enum Py8ExecutionOutput
py8_cpuLD_F_VX(Py8CPU* cpu, Py8Opcode opcode, char* code){
    if (!cpu || !code){
        return PY8_EXECOUT_EMPTY_STRUCT;
    }
    size_t x = opcode.iq2;
    cpu->ir = PY8_MEM_ADDR_FONTSET_START + (PY8_SIZE_FONTSET_PIXEL_PER_SPRITE * cpu->registers[x]);
    code[10] = _py8_its((uint8_t) x);
    return PY8_EXECOUT_SUCCESS;
}

/*
* LD B, V{0xX}.
*/
enum Py8ExecutionOutput
py8_cpuLD_B_VX(Py8CPU* cpu, Py8Opcode opcode, char* code){
    if (!cpu || !code){
        return PY8_EXECOUT_EMPTY_STRUCT;
    }
    size_t x = opcode.iq2;
    uint8_t value = cpu->registers[x];
    cpu->memory[cpu->ir + 2] = value % 10;
    value /= 10;
    cpu->memory[cpu->ir + 1] = value % 10;
    value /= 10;
    cpu->memory[cpu->ir] = value % 10;
    code[10] = _py8_its((uint8_t) x);
    return PY8_EXECOUT_SUCCESS;
}

/*
* LD [I], V{0xX}.
*/
enum Py8ExecutionOutput
py8_cpuLD_I_V0_VX(Py8CPU* cpu, Py8Opcode opcode, char* code){
    if (!cpu || !code){
        return PY8_EXECOUT_EMPTY_STRUCT;
    }
    size_t x = opcode.iq2;
    if ((cpu->implm_flags & PY8_IMPLM_MODE_FX_CHANGE_I)){
        for (size_t i=0; i<=x; i++){
            cpu->memory[cpu->ir + i] = cpu->registers[i];
            cpu->ir++;
        }
    }else {
        for (size_t i=0; i<=x; i++){
            cpu->memory[cpu->ir + i] = cpu->registers[i];
        }
    }
    code[12] = _py8_its((uint8_t) x);
    return PY8_EXECOUT_SUCCESS;
}

/*
* LD V{0xX}, [I].
*/
enum Py8ExecutionOutput
py8_cpuLD_VX_V0_I(Py8CPU* cpu, Py8Opcode opcode, char* code){
    if (!cpu || !code){
        return PY8_EXECOUT_EMPTY_STRUCT;
    }
    size_t x = opcode.iq2;
    if ((cpu->implm_flags & PY8_IMPLM_MODE_FX_CHANGE_I)){
        for (size_t i=0; i<=x; i++){
            cpu->registers[i] = cpu->registers[cpu->ir + i];
            cpu->ir++;
        }
    }else {
        for (size_t i=0; i<=x; i++){
            cpu->registers[i] = cpu->registers[cpu->ir + i];
        }
    }
    code[7] = _py8_its((uint8_t) x);
    return PY8_EXECOUT_SUCCESS;
}

Py8Instruction
py8_opcodeDecode(Py8Opcode opcode){
    Py8Instruction instruction;
    switch (opcode.msq) {
        case 0x0:
            switch (opcode.lsq) {
                case 0x0:
                    instruction = (Py8Instruction) {
                        .code = "CLS",
                        .exec = py8_cpuCLS};
                    break;
                case 0xE:
                    instruction = (Py8Instruction) {
                        .code="RET",
                        .exec = py8_cpuRET};
                    break;
                default:
                    instruction = (Py8Instruction) {
                        .code = "NOP", 
                        .exec = py8_cpuExecutionError
                    };
                    break;
            }
            break;
        case 0x1:
            instruction = (Py8Instruction) {
                .code = "JP 0x0NNN",
                .exec = py8_cpuJMP_ADDR,
            };
            break;
        case 0x2:
            instruction = (Py8Instruction) {
                .code = "CALL 0x0NNN",
                .exec = py8_cpuCALL,
            };
            break;
        case 0x3:
            instruction = (Py8Instruction) {
                .code = "SE V{0xX}, 0xKK",
                .exec = py8_cpuSE_VX_BYTE,
            };
            break;
        case 0x4:
            instruction = (Py8Instruction) {
                .code = "SNE V{0xX}, 0xKK",
                .exec = py8_cpuSNE_VX_BYTE,
            };
            break;
        case 0x5:
            instruction = (Py8Instruction) {
                .code = "SE V{0xX}, V{0xY}",
                .exec = py8_cpuSE_VX_VY,
            };
            break;
        case 0x6:
            instruction = (Py8Instruction) {
                .code = "LD V{0xX}, 0xKK",
                .exec = py8_cpuLD_VX_BYTE,
            };
            break;
        case 0x7:
            instruction = (Py8Instruction) {
                .code = "ADD V{0xX}, 0xKK",
                .exec = py8_cpuADD_VX_BYTE,
            };
            break;
        case 0x8:
            switch (opcode.lsq) {
                case 0x0:
                    instruction = (Py8Instruction) {
                        .code = "LD V{0xX}, V{0xY}",
                        .exec = py8_cpuLD_VX_VY,
                    };
                    break;
                case 0x1:
                    instruction = (Py8Instruction) {
                        .code = "OR V{0xX}, V{0xY}",
                        .exec = py8_cpuOR_VX_VY,
                    };
                    break;
                case 0x2:
                    instruction = (Py8Instruction) {
                        .code = "AND V{0xX}, V{0xY}",
                        .exec = py8_cpuAND_VX_VY,
                    };
                    break;
                case 0x3:
                    instruction = (Py8Instruction) {
                        .code = "XOR V{0xX}, V{0xY}",
                        .exec = py8_cpuXOR_VX_VY,
                    };
                    break;
                case 0x4:
                    instruction = (Py8Instruction) {
                        .code = "ADD V{0xX}, V{0xY}",
                        .exec = py8_cpuADD_VX_VY,
                    };
                    break;
                case 0x5:
                    instruction = (Py8Instruction) {
                        .code = "SUB V{0xX}, V{0xY}",
                        .exec = py8_cpuSUB_VX_VY,
                    };
                    break;
                case 0x6:
                    instruction = (Py8Instruction) {
                        .code = "SHR V{0xX}, V{0xY}",
                        .exec = py8_cpuSHR_VX_VY,
                    };
                    break;
                case 0x7:
                    instruction = (Py8Instruction) {
                        .code = "SUBN V{0xX}, V{0xY}",
                        .exec = py8_cpuSUBN_VX_VY,
                    };
                    break;
                case 0xE:
                    instruction = (Py8Instruction) {
                        .code = "SHL V{0xX}, V{0xY}",
                        .exec = py8_cpuSHL_VX_VY,
                    };
                    break;
                default:
                    instruction = (Py8Instruction) {
                        .code = "NOP",
                        .exec = py8_cpuExecutionError,
                    };
                    break;
            }
            break;
        case 0x9:
                instruction = (Py8Instruction) {
                    .code = "SNE V{0xX}, V{0xY}",
                    .exec = py8_cpuSNE_VX_VY,
                };
            break;
        case 0xA:
                instruction = (Py8Instruction) {
                    .code = "LD I, 0x0NNN",
                    .exec = py8_cpuLD_I_ADDR,
                };
            break;
        case 0xB:
                instruction = (Py8Instruction) {
                    .code = "JP V{0x0}, 0x0NNN",
                    .exec = py8_cpuJP_V0_ADDR,
                };
            break;
        case 0xC:
                instruction = (Py8Instruction) {
                    .code = "RND V{0xX}, 0xKK",
                    .exec = py8_cpuRND_VX_BYTE,
                };
            break;
        case 0xD:
                instruction = (Py8Instruction) {
                    .code = "DRW V{0xX}, V{0xY}, 0xN",
                    .exec = py8_cpuDRW_VX_VY_N,
                };
            break;
        case 0xE:
            switch (opcode.lsq) {
                case 0xE:
                    instruction = (Py8Instruction) {
                        .code = "SKP V{0xX}",
                        .exec = py8_cpuSKP_VX,
                    };
                    break;
                case 0x1:
                    instruction = (Py8Instruction) {
                        .code = "SKNP V{0xX}",
                        .exec = py8_cpuSKNP_VX,
                    };
                    break;
                default:
                    instruction = (Py8Instruction) {
                        .code = "NOP",
                        .exec = py8_cpuExecutionError, };
                    break;
            }
            break;
        case 0xF:
            switch (opcode.lsq) {
                case 0x7:
                    instruction = (Py8Instruction) {
                        .code = "LD V{0xX}, DT",
                        .exec = py8_cpuLD_VX_DT,
                    };
                    break;
                case 0xA:
                    instruction = (Py8Instruction) {
                        .code = "LD V{0xX}, K{0xK}",
                        .exec = py8_cpuLD_VX_K,
                    };
                    break;
                case 0x5:
                    switch (opcode.iq1) {
                        case 0x1:
                            instruction = (Py8Instruction) {
                                .code = "LD DT, V{0xX}",
                                .exec = py8_cpuLD_DT_VX,
                            };
                            break;
                        case 0x5:
                            instruction = (Py8Instruction) {
                                .code = "LD [I], V{0xX}",
                                .exec = py8_cpuLD_I_V0_VX,
                            };
                            break;
                        case 0x6:
                            instruction = (Py8Instruction) {
                                .code = "LD V{0xX}, [I]",
                                .exec = py8_cpuLD_VX_V0_I,
                            };
                            break;
                        default:
                            instruction = (Py8Instruction) {
                                .code = "NOP",
                                .exec = py8_cpuExecutionError,
                            };
                            break;
                    }
                    break;
                case 0x8:
                    instruction = (Py8Instruction) {
                        .code = "LD ST, V{0xX}",
                        .exec = py8_cpuLD_ST_VX,
                    };
                    break;
                case 0xE:
                    instruction = (Py8Instruction) {
                        .code = "ADD I, V{0xX}",
                        .exec = py8_cpuADD_I_VX,
                    };
                    break;
                case 0x9:
                    instruction = (Py8Instruction) {
                        .code = "LD F, V{0xX}",
                        .exec = py8_cpuLD_F_VX,
                    };
                    break;
                case 0x3:
                    instruction = (Py8Instruction) {
                        .code = "LD B, V{0xX}",
                        .exec = py8_cpuLD_B_VX,
                    };
                    break;
                default:
                    instruction = (Py8Instruction) {
                        .code = "NOP",
                        .exec = py8_cpuExecutionError,
                    };
                    break;
            }
            break;
    }
    return instruction;
}

enum Py8ExecutionOutput
py8_cpuSetp(Py8CPU* cpu, Py8Instruction* instruction){
    Py8Opcode opcode = _py8_cpuGetOpcode(*cpu);
    _py8_cpuIncrementPC(cpu);
    *instruction = py8_opcodeDecode(opcode);
    _py8_cpuTickTimers(cpu);
    return instruction->exec(cpu, opcode, instruction->code);
}

#ifdef __cplusplus
    }
#endif
#endif // PY8_CPU_C
