/**
* @file cpu.h
* @author Paulo Arruda
* @license MIT
* @brief Implementation of the Chip8's CPU and related emulation routines.
*/
#ifndef PY8_CPU_C
    #define PY8_CPU_C
#ifdef __cplusplus
    extern "C"{
#endif

#include <time.h>
#include <stdlib.h>
#include "cpu.h"

#define SIZE_U8 sizeof(uint8_t)
#define SIZE_U16 sizeof(uint16_t)

/**
* @brief Converts a digit to its hexadecimal representation.
*
* @param[in] `nibble`
*/
inline static char
_py8_its(uint8_t nibble){
    char result = ' ';
    switch (nibble) {
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
py8_cpuInit(Py8CPU* cpu, enum Py8ImplmMode mode){
    if (!cpu){
        return PY8_EXECOUT_EMPTY_STRUCT;
    }
    cpu->mode = mode;
    uint8_t fontset[PY8_SIZE_FONTSET_PIXEL] = {
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
    for (size_t i = 0; i < PY8_SIZE_FONTSET_PIXEL; i++){
        cpu->memory[PY8_MEM_ADDR_FONTSET_START + i] = fontset[i];
    }
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
        return PY8_EXECOUT_ROM_FILE_NOT_FOUND;
    }
    FILE* rom_file = fopen(rom_file_path, "rb");
    if (!rom_file_path){
        return PY8_EXECOUT_ROM_FILE_FAILED_TO_OPEN;
    }
    size_t size = 0;
    if (fseek(rom_file, 0, SEEK_END) < 0 || (size = ftell(rom_file) < 0)){
        fclose(rom_file);
        return PY8_EXECOUT_ROM_FILE_FAILED_TO_READ;
    }
    rewind(rom_file);
    size_t nread = fread(cpu->memory + PY8_MEM_ADDR_PROG_START, 1, size, rom_file);
    if (nread != size){
        fclose(rom_file);
        return PY8_EXECOUT_ROM_FILE_FAILED_TO_READ;
    }
    fclose(rom_file);
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
static void
_py8_cpuIncrementPC(Py8CPU* cpu){
    cpu->pc += 2;
}

/**
* @brief Retrieves the opcode from memory.
*
* @param `cpu`.
* @return The opcode specified by the emulation process.
*/
static Py8Opcode
_py8_cpuGetOpcode(Py8CPU cpu){
    uint16_t code = ((uint16_t) cpu.memory[cpu.pc]) << 8;
    code |= cpu.memory[cpu.pc + 2];
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


enum Py8ExecutionOutput
py8_cpuExecutionError(Py8CPU* cpu, Py8Opcode opcode, char* code){
    UNUSED(cpu);
    UNUSED(opcode);
    UNUSED(code);
    return PY8_EXECOUT_INVALID_OPCODE;
}

enum Py8ExecutionOutput
py8_cpuCLS(Py8CPU* cpu, Py8Opcode opcode, char* code){
    UNUSED(opcode);
    UNUSED(code);
    memset(&cpu->graphics, 0, PY8_SIZE_GRAPHICS * SIZE_U8);
    return PY8_EXECOUT_SUCCESS;
}

enum Py8ExecutionOutput
py8_cpuRET(Py8CPU* cpu, Py8Opcode opcode, char* code){
    UNUSED(opcode);
    UNUSED(code);
    if (cpu->sp == 0){
        return PY8_EXECOUT_STACK_EMPTY;
    }
    cpu->sp--;
    cpu->pc = cpu->stack[cpu->sp];
    return PY8_EXECOUT_SUCCESS;
}

enum Py8ExecutionOutput
py8_cpuCALL(Py8CPU* cpu, Py8Opcode opcode, char* code){
    if (cpu->sp == 16){
        return PY8_EXECOUT_STACK_OVERFLOW;
    }
    cpu->stack[cpu->sp] = cpu->pc;
    cpu->sp++;
    cpu->pc = py8_opcodeGetAddr(opcode);
    return PY8_EXECOUT_SUCCESS;
}

/*
* JP, 0x0NNN.
*/
enum Py8ExecutionOutput
py8_cpuJMP_ADDR(Py8CPU* cpu, Py8Opcode opcode, char* code){
    cpu->pc = py8_opcodeGetAddr(opcode);
    _py8_writeU16(code + 4, cpu->pc);
    return PY8_EXECOUT_SUCCESS;
}

/*
* SE V0xX, 0xKK.
*/
enum Py8ExecutionOutput
py8_cpuSE_VX_BYTE(Py8CPU* cpu, Py8Opcode opcode, char* code){
    uint8_t kk = py8_opcodeGetByte(opcode);
    size_t x = opcode.iq2;
    if (cpu->registers[x] == kk){
        _py8_cpuIncrementPC(cpu);
    }
    code[6] = _py8_its((uint8_t) x);
    _py8_writeU8(code + 9, kk);
    return PY8_EXECOUT_SUCCESS;
}

/*
* SNE V0xX, 0xKK.
*/
enum Py8ExecutionOutput
py8_cpuSNE_VX_BYTE(Py8CPU* cpu, Py8Opcode opcode, char* code){
    uint8_t kk = py8_opcodeGetByte(opcode);
    size_t x = opcode.iq2;
    if (cpu->registers[x] != kk){
        _py8_cpuIncrementPC(cpu);
    }
    code[7] = _py8_its((uint8_t) x);
    _py8_writeU8(code + 10, kk);
    return PY8_EXECOUT_SUCCESS;
}

/*
* LD V0xX, 0xKK.
*/
enum Py8ExecutionOutput
py8_cpuLD_VX_BYTE(Py8CPU* cpu, Py8Opcode opcode, char* code){
    size_t x = opcode.iq2;
    uint8_t kk = py8_opcodeGetByte(opcode);
    cpu->registers[x] = kk;
    code[6] = _py8_its((uint8_t) x);
    _py8_writeU8(code + 9, kk);
    return PY8_EXECOUT_SUCCESS;
}

/*
* ADD V0xX, 0xKK.
*/
enum Py8ExecutionOutput
py8_cpuADD_VX_BYTE(Py8CPU* cpu, Py8Opcode opcode, char* code){
    uint8_t kk = py8_opcodeGetByte(opcode);
    size_t x = opcode.iq2;
    cpu->registers[x] += kk;
    code[7] = _py8_its((uint8_t) x);
    _py8_writeU8(code + 10, kk);
    return PY8_EXECOUT_SUCCESS;
}

/*
* SE V0xX, V0xY.
*/
enum Py8ExecutionOutput
py8_cpuSE_VX_VY(Py8CPU* cpu, Py8Opcode opcode, char* code){
    size_t x = opcode.iq2;
    size_t y = opcode.iq1;
    if (cpu->registers[x] == cpu->registers[y]){
        _py8_cpuIncrementPC(cpu);
    }
    code[6] = _py8_its((uint8_t) x);
    code[12] = _py8_its((uint8_t) y);
    return PY8_EXECOUT_SUCCESS;
}

/*
* SNE V0xX, V0xY.
*/
enum Py8ExecutionOutput
py8_cpuSNE_VX_VY(Py8CPU* cpu, Py8Opcode opcode, char* code){
    size_t x = opcode.iq2;
    size_t y = opcode.iq1;
    if (cpu->registers[x] != cpu->registers[y]){
        _py8_cpuIncrementPC(cpu);
    }
    code[7] = _py8_its((uint8_t) x);
    code[13] = _py8_its((uint8_t) y);
    return PY8_EXECOUT_SUCCESS;
}

/*
* LD V0xX, V0xY.
*/
enum Py8ExecutionOutput
py8_cpuLD_VX_VY(Py8CPU* cpu, Py8Opcode opcode, char* code){
    size_t x = opcode.iq2;
    size_t y = opcode.iq1;
    cpu->registers[x] = cpu->registers[y];
    code[6] = _py8_its((uint8_t) x);
    code[12] = _py8_its((uint8_t) y);
    return PY8_EXECOUT_SUCCESS;
}

/*
* OR V0xX, V0xY.
*/
enum Py8ExecutionOutput
py8_cpuOR_VX_VY(Py8CPU* cpu, Py8Opcode opcode, char* code){
    size_t x = opcode.iq2;
    size_t y = opcode.iq1;
    cpu->registers[x] |= cpu->registers[y];
    code[6] = _py8_its((uint8_t) x);
    code[12] = _py8_its((uint8_t) y);
    return PY8_EXECOUT_SUCCESS;
}

/*
* AND V0xX, V0xY.
*/
enum Py8ExecutionOutput
py8_cpuAND_VX_VY(Py8CPU* cpu, Py8Opcode opcode, char* code){
    size_t x = opcode.iq2;
    size_t y = opcode.iq1;
    cpu->registers[x] &= cpu->registers[y];
    code[7] = _py8_its((uint8_t) x);
    code[13] = _py8_its((uint8_t) y);
    return PY8_EXECOUT_SUCCESS;
}

/*
* XOR V0xX, V0xY.
*/
enum Py8ExecutionOutput
py8_cpuXOR_VX_VY(Py8CPU* cpu, Py8Opcode opcode, char* code){
    size_t x = opcode.iq2;
    size_t y = opcode.iq1;
    cpu->registers[x] ^= cpu->registers[y];
    code[7] = _py8_its((uint8_t) x);
    code[13] = _py8_its((uint8_t) y);
    return PY8_EXECOUT_SUCCESS;
}

/*
* ADD V0xX, V0xY.
*/
enum Py8ExecutionOutput
py8_cpuADD_VX_VY(Py8CPU* cpu, Py8Opcode opcode, char* code){
    size_t x = opcode.iq2;
    size_t y = opcode.iq1;
    cpu->registers[0xF] = UINT8_MAX - cpu->registers[x] < cpu->registers[y]? 1:0;
    cpu->registers[x] += cpu->registers[y];
    code[7] = _py8_its((uint8_t) x);
    code[13] = _py8_its((uint8_t) y);
    return PY8_EXECOUT_SUCCESS;
}

/*
* SUB V0xX, V0xY.
*/
enum Py8ExecutionOutput
py8_cpuSUB_VX_VY(Py8CPU* cpu, Py8Opcode opcode, char* code){
    size_t x = opcode.iq2;
    size_t y = opcode.iq1;
    cpu->registers[0xF] = cpu->registers[x] < cpu->registers[y]? 1:0;
    cpu->registers[x] -= cpu->registers[y];
    code[7] = _py8_its((uint8_t) x);
    code[13] = _py8_its((uint8_t) y);
    return PY8_EXECOUT_SUCCESS;
}

/*
* SUBN V0xX, V0xY.
*/
enum Py8ExecutionOutput
py8_cpuSUBN_VX_VY(Py8CPU* cpu, Py8Opcode opcode, char* code){
    size_t x = opcode.iq2;
    size_t y = opcode.iq1;
    cpu->registers[0xF] = cpu->registers[x] > cpu->registers[y]? 1:0;
    cpu->registers[x] = cpu->registers[y] - cpu->registers[x];
    code[8] = _py8_its((uint8_t) x);
    code[14] = _py8_its((uint8_t) y);
    return PY8_EXECOUT_SUCCESS;
}

/*
* SHR V0xX.
*/
enum Py8ExecutionOutput
py8_cpuSHR_VX_VY(Py8CPU* cpu, Py8Opcode opcode, char* code){
    size_t x = opcode.iq2;
    size_t y = opcode.iq1;
    switch (cpu->mode) {
        case PY8_IMPLM_MODE_COSMAC_VIP:
            cpu->registers[x] = cpu->registers[y];
            break;
        case PY8_IMPLM_MODE_MODERN:
            break;
    }
    cpu->registers[0xF] = (cpu->registers[x] & 0x1) != 0? 1:0;
    cpu->registers[x] >>= 1;
    code[7] = _py8_its((uint8_t) x);
    return PY8_EXECOUT_SUCCESS;
}

/*
* SHL V0xX.
*/
enum Py8ExecutionOutput
py8_cpuSHL_VX_VY(Py8CPU* cpu, Py8Opcode opcode, char* code){
    size_t x = opcode.iq2;
    size_t y = opcode.iq1;
    switch (cpu->mode) {
        case PY8_IMPLM_MODE_COSMAC_VIP:
            cpu->registers[x] = cpu->registers[y];
            break;
        case PY8_IMPLM_MODE_MODERN:
            break;
    }
    cpu->registers[0xF] = (cpu->registers[x] & 0x80) != 0? 1:0;
    cpu->registers[x] <<= 1;
    code[7] = _py8_its((uint8_t) x);
    return PY8_EXECOUT_SUCCESS;
}

/*
* JP V0, 0x0NNN.
*/
enum Py8ExecutionOutput
py8_cpuLD_I_ADDR(Py8CPU* cpu, Py8Opcode opcode, char* code){
    uint16_t addr = py8_opcodeGetAddr(opcode);
    size_t x = opcode.iq1;
    switch (cpu->mode) {
        case PY8_IMPLM_MODE_COSMAC_VIP:
            cpu->pc = addr + cpu->registers[0];
            break;
        case PY8_IMPLM_MODE_MODERN:
            cpu->pc = addr + cpu->registers[x];
            break;
    }
    _py8_writeU16(code + 7, addr);
    return PY8_EXECOUT_SUCCESS;
}

/*
* JP V0, 0x0NNN.
*/
enum Py8ExecutionOutput
py8_cpuJP_V0_ADDR(Py8CPU* cpu, Py8Opcode opcode, char* code){
    uint16_t addr = py8_opcodeGetAddr(opcode);
    size_t x = opcode.iq1;
    switch (cpu->mode) {
        case PY8_IMPLM_MODE_COSMAC_VIP:
            cpu->pc = addr + cpu->registers[0];
            break;
        case PY8_IMPLM_MODE_MODERN:
            cpu->pc = addr + cpu->registers[x];
            break;
    }
    _py8_writeU16(code + 7, addr);
    return PY8_EXECOUT_SUCCESS;
}

/*
* RND V0xX, 0xKK.
*/
enum Py8ExecutionOutput
py8_cpuRND_VX_BYTE(Py8CPU* cpu, Py8Opcode opcode, char* code){
    srand(time(NULL));
    size_t x = opcode.iq2;
    uint8_t kk = py8_opcodeGetByte(opcode);
    cpu->registers[x] = rand() & kk;
    code[7] = _py8_its((uint8_t) x);
    _py8_writeU8(code + 10, kk);
    return PY8_EXECOUT_SUCCESS;
}

/*
* DRW V0xX, V0xY, 0xN.
*/
enum Py8ExecutionOutput
py8_cpuDRW_VX_VY_N(Py8CPU* cpu, Py8Opcode opcode, char* code){
    size_t x = opcode.iq2;
    size_t y = opcode.iq1;
    size_t n = opcode.lsq;
    size_t pos_x = cpu->registers[x];
    size_t pos_y = cpu->registers[y];
    size_t byte_index = 0;
    while (byte_index < n){
        size_t bit_index = 0;
        uint8_t byte = cpu->memory[cpu->ir + byte_index];
        while (bit_index < 8){
            uint8_t bit = (byte >> byte_index) & 0x1;
            size_t index = ((pos_y + byte_index) & 31) * PY8_GRAPHICS_WIDTH +
                           ((pos_x + bit_index) & 63);
            uint8_t* pixel_ptr = &cpu->registers[index];
            *pixel_ptr ^= bit;
            if (bit && *pixel_ptr){
                cpu->registers[0xF] = 1;
            }
            bit_index++;
        }
        byte_index++;
    }
    code[7] = _py8_its((uint8_t) x);
    code[13] = _py8_its((uint8_t) y);
    code[18] = _py8_its((uint8_t) n);
    return PY8_EXECOUT_SUCCESS;
}

/*
* SKP V0xX.
*/
enum Py8ExecutionOutput
py8_cpuSKP_VX(Py8CPU* cpu, Py8Opcode opcode, char* code){
    size_t x = opcode.iq2;
    size_t key = cpu->registers[x];
    if (py8_cpuGetKeyVal(*cpu, key)){
        _py8_cpuIncrementPC(cpu);
    }
    code[7] = _py8_its((uint8_t) x);
    return PY8_EXECOUT_SUCCESS;
}

/*
* SKNP V0xX.
*/
enum Py8ExecutionOutput
py8_cpuSKNP_VX(Py8CPU* cpu, Py8Opcode opcode, char* code){
    size_t x = opcode.iq2;
    size_t key = cpu->registers[x];
    if (!py8_cpuGetKeyVal(*cpu, key)){
        _py8_cpuIncrementPC(cpu);
    }
    code[7] = _py8_its((uint8_t) x);
    return PY8_EXECOUT_SUCCESS;
}

/*
* LD V0xX, DT.
*/
enum Py8ExecutionOutput
py8_cpuLD_VX_DT(Py8CPU* cpu, Py8Opcode opcode, char* code){
    size_t x = opcode.iq2;
    cpu->registers[x] = cpu->dt;
    code[6] = _py8_its((uint8_t) x);
    return PY8_EXECOUT_SUCCESS;
}

/*
* LD V0xX, K.
*/
enum Py8ExecutionOutput
py8_cpuLD_VX_K(Py8CPU* cpu, Py8Opcode opcode, char* code){
    bool has_ld = false;
    size_t x = opcode.iq2;
    for (size_t i = 0; i < PY8_SIZE_KEYSET; i++){
        if (py8_cpuGetKeyVal(*cpu, i)){
            cpu->registers[x] = (uint8_t) i;
            has_ld = true;
        }
    }
    if (!has_ld){
        _py8_cpuIncrementPC(cpu);
    }
    code[6] = _py8_its((uint8_t) x);
    return PY8_EXECOUT_SUCCESS;
}

/*
* LD DT, V0xX.
*/
enum Py8ExecutionOutput
py8_cpuLD_DT_VX(Py8CPU* cpu, Py8Opcode opcode, char* code){
    size_t x = opcode.iq2;
    cpu->dt = cpu->registers[x];
    code[10] = _py8_its((uint8_t) x);
    return PY8_EXECOUT_SUCCESS;
}

/*
* LD ST, V0xX.
*/
enum Py8ExecutionOutput
py8_cpuLD_ST_VX(Py8CPU* cpu, Py8Opcode opcode, char* code){
    size_t x = opcode.iq2;
    cpu->st = cpu->registers[x];
    code[10] = _py8_its((uint8_t) x);
    return PY8_EXECOUT_SUCCESS;
}

/*
* ADD I, V0xX.
*/
enum Py8ExecutionOutput
py8_cpuADD_I_VX(Py8CPU* cpu, Py8Opcode opcode, char* code){
    size_t x = opcode.iq2;
    cpu->ir += cpu->registers[x];
    code[10] = _py8_its((uint8_t) x);
    return PY8_EXECOUT_SUCCESS;
}

/*
* LD F, V0xX.
*/
enum Py8ExecutionOutput
py8_cpuLD_F_VX(Py8CPU* cpu, Py8Opcode opcode, char* code){
    size_t x = opcode.iq2;
    cpu->ir = PY8_MEM_ADDR_FONTSET_START + PY8_SIZE_FONTSET_SPRITE * cpu->registers[x];
    code[9] = _py8_its((uint8_t) x);
    return PY8_EXECOUT_SUCCESS;
}

/*
* LD B, V0xX.
*/
enum Py8ExecutionOutput
py8_cpuLD_B_VX(Py8CPU* cpu, Py8Opcode opcode, char* code){
    size_t x = opcode.iq2;
    uint8_t value = cpu->registers[x];
    cpu->memory[cpu->ir + 2] = value % 10;
    value /= 10;
    cpu->memory[cpu->ir + 1] = value % 10;
    value /= 10;
    cpu->memory[cpu->ir] = value % 10;
    code[9] = _py8_its((uint8_t) x);
    return PY8_EXECOUT_SUCCESS;
}

/*
* LD [I], V0xX.
*/
enum Py8ExecutionOutput
py8_cpuLD_I_V0_VX(Py8CPU* cpu, Py8Opcode opcode, char* code){
    size_t x = opcode.iq2;
    switch (cpu->mode) {
        case PY8_IMPLM_MODE_COSMAC_VIP:
            for (size_t i=0; i<=x; i++){
                cpu->memory[cpu->ir + i] = cpu->registers[i];
                cpu->ir++;
            }
            break;
        case PY8_IMPLM_MODE_MODERN:
            for (size_t i=0; i<=x; i++){
                cpu->memory[cpu->ir + i] = cpu->registers[i];
            }
            break;
    }
    code[11] = _py8_its((uint8_t) x);
    return PY8_EXECOUT_SUCCESS;
}

/*
* LD V0xX, [I].
*/
enum Py8ExecutionOutput
py8_cpuLD_VX_V0_I(Py8CPU* cpu, Py8Opcode opcode, char* code){
    size_t x = opcode.iq2;
    switch (cpu->mode) {
        case PY8_IMPLM_MODE_COSMAC_VIP:
            for (size_t i=0; i<=x; i++){
                cpu->registers[i] = cpu->registers[cpu->ir + i];
                cpu->ir++;
            }
            break;
        case PY8_IMPLM_MODE_MODERN:
            for (size_t i=0; i<=x; i++){
                cpu->registers[i] = cpu->registers[cpu->ir + i];
            }
            break;
    }
    code[6] = _py8_its((uint8_t) x);
    return PY8_EXECOUT_SUCCESS;
}

Py8Instruction
py8_getInstruction(Py8Opcode opcode){
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
                .code = "CALL",
                .exec = py8_cpuCALL,
            };
            break;
        case 0x3:
            instruction = (Py8Instruction) {
                .code = "SE V0xX, 0xKK",
                .exec = py8_cpuSE_VX_BYTE,
            };
            break;
        case 0x4:
            instruction = (Py8Instruction) {
                .code = "SNE V0xX, 0xKK",
                .exec = py8_cpuSNE_VX_BYTE,
            };
            break;
        case 0x5:
            instruction = (Py8Instruction) {
                .code = "SE V0xX, V0xY",
                .exec = py8_cpuSE_VX_VY,
            };
            break;
        case 0x6:
            instruction = (Py8Instruction) {
                .code = "LD V0xX, 0xKK",
                .exec = py8_cpuSE_VX_VY,
            };
            break;
        case 0x7:
            instruction = (Py8Instruction) {
                .code = "ADD V0xX, 0xKK",
                .exec = py8_cpuADD_VX_BYTE,
            };
            break;
        case 0x8:
            switch (opcode.lsq) {
                case 0x0:
                    instruction = (Py8Instruction) {
                        .code = "LD V0xX, V0xY",
                        .exec = py8_cpuLD_VX_VY,
                    };
                    break;
                case 0x1:
                    instruction = (Py8Instruction) {
                        .code = "OR V0xX, V0xY",
                        .exec = py8_cpuOR_VX_VY,
                    };
                    break;
                case 0x2:
                    instruction = (Py8Instruction) {
                        .code = "AND V0xX, V0xY",
                        .exec = py8_cpuAND_VX_VY,
                    };
                    break;
                case 0x3:
                    instruction = (Py8Instruction) {
                        .code = "XOR V0xX, V0xY",
                        .exec = py8_cpuXOR_VX_VY,
                    };
                    break;
                case 0x4:
                    instruction = (Py8Instruction) {
                        .code = "ADD V0xX, V0xY",
                        .exec = py8_cpuADD_VX_VY,
                    };
                    break;
                case 0x5:
                    instruction = (Py8Instruction) {
                        .code = "SUB V0xX, V0xY",
                        .exec = py8_cpuSUB_VX_VY,
                    };
                    break;
                case 0x6:
                    instruction = (Py8Instruction) {
                        .code = "SHR V0xX",
                        .exec = py8_cpuSHR_VX_VY,
                    };
                    break;
                case 0x7:
                    instruction = (Py8Instruction) {
                        .code = "SUBN V0xX, V0xY",
                        .exec = py8_cpuSUBN_VX_VY,
                    };
                    break;
                case 0xE:
                    instruction = (Py8Instruction) {
                        .code = "SHL V0xX",
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
                    .code = "SNE V0xX, V0xY",
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
                    .code = "JP V0, 0x0NNN",
                    .exec = py8_cpuJP_V0_ADDR,
                };
            break;
        case 0xC:
                instruction = (Py8Instruction) {
                    .code = "RND V0xX, 0xKK",
                    .exec = py8_cpuRND_VX_BYTE,
                };
            break;
        case 0xD:
                instruction = (Py8Instruction) {
                    .code = "DRW V0xX, V0xY, 0xN",
                    .exec = py8_cpuDRW_VX_VY_N,
                };
            break;
        case 0xE:
            switch (opcode.lsq) {
                case 0xE:
                    instruction = (Py8Instruction) {
                        .code = "SKP V0xX",
                        .exec = py8_cpuSKP_VX,
                    };
                    break;
                case 0x1:
                    instruction = (Py8Instruction) {
                        .code = "SKNP V0xX",
                        .exec = py8_cpuSKNP_VX,
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
        case 0xF:
            switch (opcode.lsq) {
                case 0x7:
                    instruction = (Py8Instruction) {
                        .code = "LD V0xX, DT",
                        .exec = py8_cpuLD_VX_DT,
                    };
                    break;
                case 0xA:
                    instruction = (Py8Instruction) {
                        .code = "LD V0xX, K",
                        .exec = py8_cpuLD_VX_K,
                    };
                    break;
                case 0x5:
                    switch (opcode.iq1) {
                        case 0x1:
                            instruction = (Py8Instruction) {
                                .code = "LD DT, V0xX",
                                .exec = py8_cpuLD_DT_VX,
                            };
                            break;
                        case 0x5:
                            instruction = (Py8Instruction) {
                                .code = "LD [I], V0xX",
                                .exec = py8_cpuLD_I_V0_VX,
                            };
                            break;
                        case 0x6:
                            instruction = (Py8Instruction) {
                                .code = "LD V0xX, [I]",
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
                        .code = "LD ST, V0xX",
                        .exec = py8_cpuLD_ST_VX,
                    };
                    break;
                case 0xE:
                    instruction = (Py8Instruction) {
                        .code = "ADD I, V0xX",
                        .exec = py8_cpuADD_I_VX,
                    };
                    break;
                case 0x9:
                    instruction = (Py8Instruction) {
                        .code = "LD F, V0xX",
                        .exec = py8_cpuLD_F_VX,
                    };
                    break;
                case 0x3:
                    instruction = (Py8Instruction) {
                        .code = "LD F, V0xX",
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
py8_cpuEmulate(Py8CPU* cpu, Py8Instruction* instruction){
    Py8Opcode opcode = _py8_cpuGetOpcode(*cpu);
    _py8_cpuIncrementPC(cpu);
    *instruction = py8_getInstruction(opcode);
    _py8_cpuTickTimers(cpu);
    return instruction->exec(cpu, opcode, instruction->code);
}

#ifdef __cplusplus
    }
#endif
#endif // PY8_CPU_C
