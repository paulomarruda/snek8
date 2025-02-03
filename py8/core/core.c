/**
* @file core.c
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
#ifndef PY8_CORE_C
    #define PY8_CORE_C
#include "pytypedefs.h"
#include <time.h>
#ifdef __cplusplus
    extern "C"{
#endif
#include <Python.h>
#include "cpu.h"

typedef struct{
    PyObject_HEAD
    Py8CPU ob_cpu;
    bool ob_is_emulating;
} Py8Emulator;

PyDoc_STRVAR(PY8_STR_DOC_EMULATOR,
             "Chip8's CPU Emulator.\n\n"
             "Attributes\n"
             "----------\n"
             "is_emulating: bool\n"
             "\tControls whether the emulation process is running.\n"
             "\n\n"
             "Parameters\n"
             "----------\n"
             "implementation: int\n"
             "  Which implementation to use. The possible values are:\n"
             "\t-0: The original COSMAC-VIP implementation.\n"
             "\t-1: Modern implementations.\n"
);

PyDoc_STRVAR(PY8_STR_DOC_EMULATOR_IS_EMULATING,
    "Controls whether the emulation is running."
);

/**
* @brief Declaration of the emulator attributes.
*/
static PyMemberDef py8_emulator_members[] = {
    {
        .name = "is_emulating",
        .type = Py_T_BOOL,
        .offset = offsetof(Py8Emulator, ob_is_emulating),
        .flags = 0, 
        .doc = PY8_STR_DOC_EMULATOR_IS_EMULATING,
    },
    {NULL},
};

/*
* INIT and DENIT METHODS
* ----------------------
*/



/**
* @brief C interface for the __del__ method.
*/
static void
py8_emulatorDel(PyObject* self){
    Py_TYPE(self)->tp_free(self);
}

/**
* @brief C interface for the __new__ method.
*/
static PyObject*
py8_emulatorNew(PyTypeObject* subtype, PyObject* args, PyObject* kwargs){
    UNUSED(args);
    UNUSED(kwargs);
    PyObject* self = subtype->tp_alloc(subtype, 0);
    if (!self){
        PyErr_SetString(PyExc_MemoryError, "Failed to allocate memory for a new emulator");
        return NULL;
    }
    return self;
}

/**
* @brief C interface for the __init__ method.
*/
static int
py8_emulatorInit(PyObject* self, PyObject* args, PyObject* kwargs){
    int mode = 0;
    char* kwlist[] = {
        "implementation",
        NULL,
    };
    if (!PyArg_VaParseTupleAndKeywords(args, kwargs, "|i", , ))
    return 0;
}

/*
* CONSTANT METHODS
* ------------------
*/

static PyObject*
py8_emulatorGetPC(PyObject* self, PyObject* args);


/*
* EMULATION METHODS
* ------------------
*/

#ifdef __cplusplus
    }
#endif
#endif // PY8_CORE_H
