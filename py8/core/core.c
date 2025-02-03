/**
* @file core.c
* @author Paulo Arruda
* @license MIT
* @brief Implementation of the Python Py8 Emulator extension module.
* @note Built with Python 3.12.
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
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|i", kwlist, &mode)){
        return -1;
    }
    CAST_PTR(Py8Emulator, self)->ob_is_emulating = true;
    switch (mode) {
        case 0:
            py8_cpuInit(&CAST_PTR(Py8Emulator, self)->ob_cpu, PY8_IMPLM_MODE_COSMAC_VIP);
            break;
        case 1:
            py8_cpuInit(&CAST_PTR(Py8Emulator, self)->ob_cpu, PY8_IMPLM_MODE_MODERN);
            break;
        default:
            PyErr_Format(PyExc_ValueError, "Value %d is invalid for implementation.", mode);
            break;
    }
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
