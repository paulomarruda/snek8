/**
* @file core.c
* @author Paulo Arruda
* @license GPL-3
* @brief Implementation of the Python Snek8 extension module. This module is
* centered at the new bilt-in type Snek8EMulator, that is responsible to bridge
* the information between the frontend and the CPU and its routines.
* @note Built with Python 3.13.
*/
#ifndef SNEK8_CORE_C
    #define SNEK8_CORE_C
#ifdef __cplusplus
    extern "C"{
#endif
#include <Python.h>
#include "cpu.h"

typedef struct{
    PyObject_HEAD
    Snek8CPU ob_cpu;
    bool ob_is_running;
    char ob_last_instruc[30];
} Snek8Emulator;

PyDoc_STRVAR(SNEK8_STR_DOC_SNEK8_EMULATOR,
             "Snek8Emulator(implm_flags: int = 0)\n\n"
             "Chip8's emulator.\n\n"
             "Attributes\n"
             "----------\n"
             "is_running: bool\n"
             "\tControls whether the emulation process is running.\n"
             "last_instruc: str\n"
             "\tThe last executed instruction\n"
             "\n\n"
             "Parameters\n"
             "----------\n"
             "implm_flags: int\n"
             "\tFlags that dictates how certain instructions (see below) are executed. "
             "The flags passed to the init method has to be a bitwise or combination of "
             "the following:\n"
             "\t\t-0: IMPLM_MODE_BNNN_USE_VX.\n"
             "\t\t-1: IMPLM_MODE_SHIFTS_USE_VY.\n"
             "\t\t-2: IMPLM_MODE_FX_CHANGE_I.\n"
);

PyDoc_STRVAR(SNEK8_STR_DOC_EMULATOR_SNEK8_EMULATOR_IS_RUNNING,
    "is_running: int\n"
    "\tControls whether the emulation is running.\n"
    "Note\n"
    "----\n"
    "This is a read-only attribute that can only be modified by the emulation process.\n"
);

PyDoc_STRVAR(SNEK8_STR_DOC_EMULATOR_SNEK8_EMULATOR_LAST_INSTRUC,
    "last_instrc: int\n"
    "\nThe last executed instruction."
    "Note\n"
    "----\n"
    "This is a read-only attribute that can only be modified by the emulation process.\n"
);

/**
* @brief Declaration of the emulator attributes.
*/
static PyMemberDef snek8_emulator_members[] = {
    {
        .name = "is_running",
        .type = Py_T_BOOL,
        .offset = offsetof(Snek8Emulator, ob_is_running),
        .flags = Py_READONLY, 
        .doc = SNEK8_STR_DOC_EMULATOR_SNEK8_EMULATOR_IS_RUNNING,
    },
    {
        .name = "last_instruc",
        .type = Py_T_STRING,
        .offset = offsetof(Snek8Emulator, ob_last_instruc),
        .flags = Py_READONLY, 
        .doc = SNEK8_STR_DOC_EMULATOR_SNEK8_EMULATOR_LAST_INSTRUC,
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
snek8_emulatorDel(PyObject* self){
    Py_TYPE(self)->tp_free(self);
}

/**
* @brief C interface for the __new__ method.
*/
static PyObject*
snek8_emulatorNew(PyTypeObject* subtype, PyObject* args, PyObject* kwargs){
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
snek8_emulatorInit(PyObject* self, PyObject* args, PyObject* kwargs){
    int implm_flags = 0;
    char* kwlist[] = {
        "implm_flags",
        NULL,
    };
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|i", kwlist, &implm_flags)){
        return -1;
    }
    CAST_PTR(Snek8Emulator, self)->ob_is_running = false;
    if (implm_flags < 0 || implm_flags >= 255){
        PyErr_Format(PyExc_ValueError, "Value %d is invalid for implementation.", implm_flags);
        return -1;
    }
    snek8_cpuInit(&CAST_PTR(Snek8Emulator, self)->ob_cpu, (uint8_t) implm_flags);
    return 0;
}

/*
* CONSTANT METHODS
* ------------------
*/

/**
* @brief Retrieve the implementation flags.
*/
static PyObject*
snek8_emulatorGetFlags(PyObject* self, PyObject* args){
    UNUSED(args);
    return PyLong_FromLong((long) CAST_PTR(Snek8Emulator, self)->ob_cpu.implm_flags);
}

PyDoc_STRVAR(SNEK8_DOC_STR_SNEK8_EMULATOR_GET_FLAGS,
             "getFlags() -> int\n\n"
             "Retrieve the implementation flags.\n"
             "Returns\n"
             "-------\n"
             "int\n"
             "\tThe current value of the implementation flags."
);

/**
* @brief Retrieve the program counter.
*/
static PyObject*
snek8_emulatorGetPC(PyObject* self, PyObject* args){
    UNUSED(args);
    return Py_BuildValue("i", CAST_PTR(Snek8Emulator, self)->ob_cpu.pc);
}

PyDoc_STRVAR(SNEK8_DOC_STR_SNEK8_EMULATOR_GET_PC,
             "getPC() -> int\n\n"
             "Retrieve the program counter.\n"
             "Returns\n"
             "-------\n"
             "int\n"
             "\tThe current value of the program counter."
);

/**
* @brief Retrieve the delay timer register.
*/
static PyObject*
snek8_emulatorGetDT(PyObject* self, PyObject* args){
    UNUSED(args);
    return Py_BuildValue("i", CAST_PTR(Snek8Emulator, self)->ob_cpu.dt);
}

PyDoc_STRVAR(SNEK8_DOC_STR_SNEK8_EMULATOR_GET_DT,
             "getDT() -> int\n\n"
             "Retrieve the delay timer register.\n"
             "Returns\n"
             "-------\n"
             "int\n"
             "\tThe current value of the delay timer."
);

/**
* @brief Retrieve the sound timer register.
*/
static PyObject*
snek8_emulatorGetST(PyObject* self, PyObject* args){
    UNUSED(args);
    return Py_BuildValue("i", CAST_PTR(Snek8Emulator, self)->ob_cpu.st);
}

PyDoc_STRVAR(SNEK8_DOC_STR_SNEK8_EMULATOR_GET_ST,
             "getST() -> int\n\n"
             "Retrieve the sound timer register.\n"
             "Returns\n"
             "-------\n"
             "int\n"
             "\tThe current value of the sount timer."
);

/**
* @brief Retrieve the index register.
*/
static PyObject*
snek8_emulatorGetIR(PyObject* self, PyObject* args){
    UNUSED(args);
    return Py_BuildValue("i", CAST_PTR(Snek8Emulator, self)->ob_cpu.ir);
}

PyDoc_STRVAR(SNEK8_DOC_STR_SNEK8_EMULATOR_GET_IR,
             "getIR() -> int\n\n"
             "Retrieve the index register.\n"
             "Returns\n"
             "-------\n"
             "int\n"
             "\tThe current value of the index register."
);

/**
* @brief Retrieve the stack pointer.
*/
static PyObject*
snek8_emulatorGetSP(PyObject* self, PyObject* args){
    UNUSED(args);
    return Py_BuildValue("i", CAST_PTR(Snek8Emulator, self)->ob_cpu.sp);
}

PyDoc_STRVAR(SNEK8_DOC_STR_SNEK8_EMULATOR_GET_SP,
             "getSP() -> int\n\n"
             "Retrieve the stack pointer.\n"
             "Returns\n"
             "-------\n"
             "int\n"
             "\tThe current value of the stack pointer."
);

/**
* @brief Retrieve the current value of a given V-register.
*/
static PyObject*
snek8_emulatorGetRegister(PyObject* self, PyObject* args, PyObject* kwargs){
    int index;
    char* kwlist[] = {
        "index",
        NULL,
    };
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "i", kwlist, &index)){
        return NULL;
    }
    if (index < 0 || index >= 16){
        PyErr_Format(PyExc_IndexError, "Chip8 register's index ranges from 0 to 15 (inclusive).");
        return NULL;
    }
    return Py_BuildValue("i", CAST_PTR(Snek8Emulator, self)->ob_cpu.registers[index]);
}

PyDoc_STRVAR(SNEK8_DOC_STR_SNEK8_EMULATOR_GET_REGISTER,
             "getRegister(index: int) -> int\n\n"
             "Retrieve the value of a particular all purpose register.\n\n"
             "Parameters\n"
             "----------\n"
             "index: int\n"
             "\tThe index of the register to retrieve. I must be 0 <= index <= 15.\n"
             "Returns\n"
             "-------\n"
             "int\n"
             "\tThe current value of the register.\n"
             "Raises\n"
             "------\n"
             "IndexError\n"
             "\tIf the register index is not valid, i.e. index < 0 or index => 16."
);

/**
* @brief Retrieve the current values of all V-registers.
*/
static PyObject*
snek8_emulatorGetRegisters(PyObject* self, PyObject* args){
    UNUSED(args);
    PyObject* register_list = PyList_New(SNEK8_SIZE_REGISTERS);
    if (!register_list){
        PyErr_SetString(PyExc_MemoryError, "Failed to create register list.");
        return NULL;
    }
    for (size_t i = 0; i < SNEK8_SIZE_REGISTERS; i++){
        PyObject* value = Py_BuildValue("i", CAST_PTR(Snek8Emulator, self)->ob_cpu.registers[i]);
        if (!value){
            PyErr_SetString(PyExc_MemoryError, "Failed to create register list element.");
            Py_DECREF(register_list);
            return NULL;
        }
        PyList_SET_ITEM(register_list, i, value);
    }
    return register_list;
}

PyDoc_STRVAR(SNEK8_DOC_STR_SNEK8_EMULATOR_GET_REGISTERS,
             "getRegisters() -> Annotated[List[int], 16]\n\n"
             "Retrieve all values of the all porpuse registers.\n"
             "Returns\n"
             "-------\n"
             "Annotated[List[int], 16]\n"
             "\tThe current value of the registers."
);

/**
* @brief Retrieve the current values array representation of the screen.
*/
static PyObject*
snek8_emulatorGetGraphics(PyObject* self, PyObject* args){
    UNUSED(args);
    PyObject* graphics_list = PyList_New(SNEK8_SIZE_GRAPHICS);
    if (!graphics_list){
        PyErr_SetString(PyExc_MemoryError, "Failed to create graphics list.");
        return NULL;
    }
    for (size_t i = 0; i < SNEK8_SIZE_GRAPHICS; i++){
        if (CAST_PTR(Snek8Emulator, self)->ob_cpu.graphics[i]){
            PyList_SET_ITEM(graphics_list, i, Py_True);
        }else{
            PyList_SET_ITEM(graphics_list, i, Py_False);
        }
    }
    return graphics_list;
}

PyDoc_STRVAR(SNEK8_DOC_STR_SNEK8_EMULATOR_GET_GRAPHICS,
             "getGraphics() -> Annotated[List[bool], 2048]\n\n"
             "Retrieve the graphics list representation.\n"
             "Returns\n"
             "-------\n"
             "Annotated[List[bool], 2048]\n"
             "\tThe current values of pixels."
);

static PyObject*
snek8_emulatorGetStack(PyObject* self, PyObject* args){
    UNUSED(args);
    PyObject* stack_list = PyList_New(SNEK8_SIZE_STACK);
    if (!stack_list){
        PyErr_SetString(PyExc_MemoryError, "Failed to create stack list.");
        return NULL;
    }
    for (size_t i = 0; i < SNEK8_SIZE_STACK; i++){
        PyObject* value = Py_BuildValue("i", CAST_PTR(Snek8Emulator, self)->ob_cpu.stack.buffer[i]);
        if (!value){
            PyErr_SetString(PyExc_MemoryError, "Failed to create register list element.");
            Py_DECREF(stack_list);
            return NULL;
        }
        PyList_SET_ITEM(stack_list, i, value);
    }
    return stack_list;
}

PyDoc_STRVAR(SNEK8_DOC_STR_SNEK8_EMULATOR_GET_STACK,
             "getStack() -> Annotated[List[int],16]\n\n"
             "Retrieve the stack list representation.\n"
             "Returns\n"
             "-------\n"
             "Annotated[List[int],16]\n"
             "\tThe current values of the stack."
);

static PyObject*
snek8_emulatorGetKeyValue(PyObject* self, PyObject* args, PyObject* kwargs){
    int key;
    char* kwlist[] = {
        "index",
        NULL,
    };
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "i", kwlist, &key)){
        return NULL;
    }
    if (key < 0 || key >= 16){
        PyErr_Format(PyExc_IndexError, "Index must be between 0 and 15 (incl.). Value recieved: %d.", index);
        return NULL;
    }
    bool value = snek8_cpuGetKeyVal(CAST_PTR(Snek8Emulator, self)->ob_cpu, key);
    return value? Py_True: Py_False;
}

PyDoc_STRVAR(SNEK8_DOC_STR_SNEK8_EMULATOR_GET_KEY_VALUE,
             "getKeyValue(key: int) -> bool\n\n"
             "Retrieve the graphics list representation.\n"
             "Attributes\n"
             "----------\n"
             "key: int\n"
             "\tThe key index to be retrieved. 0 <= key <= 15\n"
             "Returns\n"
             "-------\n"
             "bool\n"
             "\tThe current value of the requested key.\n"
             "Raises\n"
             "------\n"
             "IndexError\n"
             "\tIf key is not a valid index, i.e. key < 0 or key >= 16."
);

/*
* EMULATION METHODS
* ------------------
*/

static PyObject*
snek8_emulatorTurnFlagsOn(PyObject* self, PyObject* args, PyObject* kwargs){
    int flags;
    char* kwlist[] = {
        "flags",
        NULL,
    };
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "i", kwlist, &flags)){
        return NULL;
    }
    CAST_PTR(Snek8Emulator, self)->ob_cpu.implm_flags |= ((uint8_t) flags);
    Py_RETURN_NONE;
}

PyDoc_STRVAR(SNEK8_DOC_STR_SNEK8_EMULATOR_TURN_FLAGS_ON,
             "turnFlagsOn(flags: int) -> None\n\n"
             "Modify the emulator's implementation mode.\n\n"
             "Attributes\n"
             "----------\n"
             "flags: int\n"
             "\tWhich implementation to toggle on. Possible values has to be a "
             "bitwise or combination of the following:\n"
             "\t\t-0: IMPLM_MODE_BNNN_USE_VX.\n"
             "\t\t-1: IMPLM_MODE_SHIFTS_USE_VY.\n"
             "\t\t-2: IMPLM_MODE_FX_CHANGE_I.\n"
             "Raises\n"
             "------\n"
             "ValueError\n"
             "\tIf the implm is not a valid value."
);

static PyObject*
snek8_emulatorTurnFlagsOff(PyObject* self, PyObject* args, PyObject* kwargs){
    int flags;
    char* kwlist[] = {
        "flags",
        NULL,
    };
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "i", kwlist, &flags)){
        return NULL;
    }
    CAST_PTR(Snek8Emulator, self)->ob_cpu.implm_flags ^= ((uint8_t) flags);
    Py_RETURN_NONE;
}

PyDoc_STRVAR(SNEK8_DOC_STR_SNEK8_EMULATOR_TURN_FLAGS_OFF,
             "turnFlagsOff(flags: int) -> None\n\n"
             "Turn the selected implementation flags off.\n\n"
             "Attributes\n"
             "----------\n"
             "flags: int\n"
             "\tWhich implementation to toggle on. Possible values has to be a "
             "bitwise or combination of the following:\n"
             "\t\t-0: IMPLM_MODE_BNNN_USE_VX.\n"
             "\t\t-1: IMPLM_MODE_SHIFTS_USE_VY.\n"
             "\t\t-2: IMPLM_MODE_FX_CHANGE_I.\n"
             "Raises\n"
             "------\n"
             "ValueError\n"
             "\tIf the implm is not a valid value."
);

static PyObject*
snek8_emulatorSetRunning(PyObject* self, PyObject* args, PyObject* kwargs){
    bool is_running;
    char* kwlist[] = {
        "is_running",
        NULL,
    };
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "b", kwlist, &is_running)){
        return NULL;
    }
    CAST_PTR(Snek8Emulator, self)->ob_is_running = is_running;
    Py_RETURN_NONE;
}

PyDoc_STRVAR(SNEK8_DOC_STR_SNEK8_EMULATOR_SET_RUNNING,
             "setRunning(is_running: bool) -> None\n\n"
             "Determine wether the CPU is running.\n\n"
             "Attributes\n"
             "----------\n"
             "is_running: bool\n"
             "\tDetermine if the CPU should run."
);

static PyObject*
snek8_emulatorSetKeyValue(PyObject* self, PyObject* args, PyObject* kwargs){
    int index;
    bool value;
    char* kwlist[] = {
        "key",
        "value",
        NULL,
    };
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "ib", kwlist, &index, &value)){
        return NULL;
    }
            if (index < 0 || index >= 16){
        PyErr_Format(PyExc_IndexError, "Key index must be between 0 and 15 (incl.). Value recieved: %d.", index);
        return NULL;
    }
    snek8_cpuSetKey(&CAST_PTR(Snek8Emulator, self)->ob_cpu, index, value);
    Py_RETURN_NONE;
}

PyDoc_STRVAR(SNEK8_DOC_STR_SNEK8_EMULATOR_SET_KEY_VALUE,
             "setKeyValue(key: int, value: bool) -> None\n\n"
             "Modifies a given key.\n"
             "Attributes\n"
             "----------\n"
             "key: int\n"
             "\tThe index to be modified.\n"
             "value: bool\n"
             "\tThe new value of the key..\n\n"
             "Raises\n"
             "------\n"
             "IndexError\n"
             "\tIf the key index is not a valid index, i.e. key < 0 or key >= 16."
);

static PyObject*
_snek8_emulatorExecOpc(PyObject* self, PyObject* args, PyObject* kwargs){
    int code = 0;
    char* kwlist[] = {
        "opcode",
        NULL,
    };
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "i", kwlist, &code)){
        return NULL;
    }
    if (code < 0 || code >= UINT16_MAX){
        PyErr_Format(PyExc_ValueError, "The opcode must be a valid 16-bit unsigned integer.");
        return NULL;
    }
    Snek8Instruction instruction = snek8_opcodeDecode((uint16_t) code);
    enum Snek8ExecutionOutput out = instruction.exec(&CAST_PTR(Snek8Emulator, self)->ob_cpu, code, instruction.code);
    return PyLong_FromLong((long) out);
}

PyDoc_STRVAR(SNEK8_DOC_STR_SNEK8_EMULATOR_EXEC_OPT,
             "_execOpc(opcode: int) -> int\n\n"
             "Execute a step in the emulation process determined by the opcode.\n"
             "This function should only be used to test CPU functionallities.\n"
             "Attributes\n"
             "----------\n"
             "opc: int\n"
             "\tThe opcode to be executed.\n"
             "Returns\n"
             "-------\n"
             "int\n"
             "\tThe execution output code representing whether the execution was successeful.\n"
             "Raises\n"
             "------\n"
             "ValueError\n"
             "\tIf the opcode is not a valid 16-bit unsigned integer."
);

static PyObject*
snek8_emulatorLoadRom(PyObject* self, PyObject* args, PyObject* kwargs){
    const char* rom_filepath = NULL;
    char* kwlist[] = {
        "rom_filepath",
        NULL,
    };
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s", kwlist, &rom_filepath)){
        return NULL;
    }
    if (!rom_filepath){
        return NULL;
    }
    enum Snek8ExecutionOutput out = snek8_cpuLoadRom(&CAST_PTR(Snek8Emulator, self)->ob_cpu, rom_filepath);
    if (out == SNEK8_EXECOUT_SUCCESS){
        CAST_PTR(Snek8Emulator, self)->ob_is_running = true;
    }
    return PyBool_FromLong((long) out);
}

PyDoc_STRVAR(SNEK8_DOC_STR_SNEK8_EMULATOR_LOAD_ROM,
             "loadRom(rom_filepath: str) -> int\n\n"
             "Load a Chip8's ROM into memory\n"
             "Attributes\n"
             "----------\n"
             "rom_filepath: str\n"
             "\tThe filepath to the ROM file.\n"
             "Returns\n"
             "-------\n"
             "int\n"
             "\tThe execution output code representing whether the execution was successeful."
);

static PyObject*
snek8_emulatorEmulationStep(PyObject* self, PyObject* args){
    UNUSED(args);
    Snek8Instruction instruc;
    enum Snek8ExecutionOutput out = snek8_cpuStep(&CAST_PTR(Snek8Emulator, self)->ob_cpu,
                                                 &instruc);
    printf("%s\n", instruc.code);
    if (out != SNEK8_EXECOUT_SUCCESS){
        CAST_PTR(Snek8Emulator, self)->ob_is_running = false;
    }
    return Py_BuildValue("i", out);
}

PyDoc_STRVAR(SNEK8_DOC_STR_SNEK8_EMULATOR_EMU_STEP,
             "emulationStep() -> int\n\n"
             "Execute one step in the emulation process\n"
             "Returns\n"
             "-------\n"
             "int\n"
             "\tThe execution output code representing whether the execution was successeful."
);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"

static struct PyMethodDef snek8_emulator_methods[] = {
    {
        .ml_name = "getMode",
        .ml_meth = snek8_emulatorGetFlags,
        .ml_flags = METH_NOARGS,
        .ml_doc = SNEK8_DOC_STR_SNEK8_EMULATOR_GET_FLAGS,
    },
    {
        .ml_name = "getPC",
        .ml_meth = snek8_emulatorGetPC,
        .ml_flags = METH_NOARGS,
        .ml_doc = SNEK8_DOC_STR_SNEK8_EMULATOR_GET_PC,
    },
    {
        .ml_name = "getIR",
        .ml_meth = snek8_emulatorGetIR,
        .ml_flags = METH_NOARGS,
        .ml_doc = SNEK8_DOC_STR_SNEK8_EMULATOR_GET_IR,
    },
    {
        .ml_name = "getDT",
        .ml_meth = snek8_emulatorGetDT,
        .ml_flags = METH_NOARGS,
        .ml_doc = SNEK8_DOC_STR_SNEK8_EMULATOR_GET_DT,
    },
    {
        .ml_name = "getST",
        .ml_meth = snek8_emulatorGetST,
        .ml_flags = METH_NOARGS,
        .ml_doc = SNEK8_DOC_STR_SNEK8_EMULATOR_GET_ST,
    },
    {
        .ml_name = "getSP",
        .ml_meth = snek8_emulatorGetSP,
        .ml_flags = METH_NOARGS,
        .ml_doc = SNEK8_DOC_STR_SNEK8_EMULATOR_GET_SP,
    },
    {
        .ml_name = "getGraphics",
        .ml_meth = snek8_emulatorGetGraphics,
        .ml_flags = METH_NOARGS,
        .ml_doc = SNEK8_DOC_STR_SNEK8_EMULATOR_GET_GRAPHICS,
    },
    {
        .ml_name = "getStack",
        .ml_meth = snek8_emulatorGetStack,
        .ml_flags = METH_NOARGS,
        .ml_doc = SNEK8_DOC_STR_SNEK8_EMULATOR_GET_STACK,
    },
    {
        .ml_name = "getRegisters",
        .ml_meth = snek8_emulatorGetRegisters,
        .ml_flags = METH_NOARGS,
        .ml_doc = SNEK8_DOC_STR_SNEK8_EMULATOR_GET_REGISTERS,
    },
    {
        .ml_name = "getRegister",
        .ml_meth = (PyCFunction) snek8_emulatorGetRegister,
        .ml_flags = METH_VARARGS | METH_KEYWORDS,
        .ml_doc = SNEK8_DOC_STR_SNEK8_EMULATOR_GET_REGISTER,
    },
    {
        .ml_name = "loadRom",
        .ml_meth = (PyCFunction) snek8_emulatorLoadRom,
        .ml_flags = METH_VARARGS | METH_KEYWORDS,
        .ml_doc = SNEK8_DOC_STR_SNEK8_EMULATOR_LOAD_ROM,
    },
    {
        .ml_name = "turnFlagsOn",
        .ml_meth = (PyCFunction) snek8_emulatorTurnFlagsOn,
        .ml_flags = METH_VARARGS | METH_KEYWORDS,
        .ml_doc = SNEK8_DOC_STR_SNEK8_EMULATOR_TURN_FLAGS_ON,
    },
    {
        .ml_name = "turnFlagsOff",
        .ml_meth = (PyCFunction) snek8_emulatorTurnFlagsOff,
        .ml_flags = METH_VARARGS | METH_KEYWORDS,
        .ml_doc = SNEK8_DOC_STR_SNEK8_EMULATOR_TURN_FLAGS_OFF,
    },
    {
        .ml_name = "setRunning",
        .ml_meth = (PyCFunction) snek8_emulatorSetRunning,
        .ml_flags = METH_VARARGS | METH_KEYWORDS,
        .ml_doc = SNEK8_DOC_STR_SNEK8_EMULATOR_SET_RUNNING,
    },
    {
        .ml_name = "getKeyValue",
        .ml_meth = (PyCFunction) snek8_emulatorGetKeyValue,
        .ml_flags = METH_VARARGS | METH_KEYWORDS,
        .ml_doc = SNEK8_DOC_STR_SNEK8_EMULATOR_GET_KEY_VALUE,
    },
    {
        .ml_name = "setKeyValue",
        .ml_meth = (PyCFunction) snek8_emulatorSetKeyValue,
        .ml_flags = METH_VARARGS | METH_KEYWORDS,
        .ml_doc = SNEK8_DOC_STR_SNEK8_EMULATOR_SET_KEY_VALUE,
    },
    {
        .ml_name = "_execOpc",
        .ml_meth = (PyCFunction) _snek8_emulatorExecOpc,
        .ml_flags = METH_VARARGS | METH_KEYWORDS,
        .ml_doc = SNEK8_DOC_STR_SNEK8_EMULATOR_EXEC_OPT,
    },
    {
        .ml_name = "emulationStep",
        .ml_meth = snek8_emulatorEmulationStep,
        .ml_flags = METH_NOARGS,
        .ml_doc = SNEK8_DOC_STR_SNEK8_EMULATOR_EMU_STEP,
    },
    {NULL},
};
#pragma GCC diagnostic pop

static struct PyMethodDef module_meths[] = {
    // {
    //     .ml_name = "version",
    //     .ml_meth = version,
    //     .ml_flags = METH_NOARGS,
    //     .ml_doc = "Versioning of the project."
    // },
    {NULL},
};

static PyTypeObject Snek8EmulatorType = {
     .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
     .tp_name = "snek8.core.Snek8Emulator",
     .tp_basicsize = sizeof(Snek8Emulator),
     .tp_itemsize = 0,
     .tp_doc = SNEK8_STR_DOC_SNEK8_EMULATOR,
     .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
     .tp_new = (newfunc) snek8_emulatorNew,
     .tp_init = (initproc) snek8_emulatorInit,
     .tp_dealloc = (destructor) snek8_emulatorDel,
     .tp_members = snek8_emulator_members,
     .tp_methods = snek8_emulator_methods,
};

PyDoc_STRVAR(SNEK8_STR_DOC_PY8,
    "CHIP8's core emulation process\n\n"
    "This module provide the core functionalities necessary for a\n"
    "CHIP8's emulation. It was based on the following documents:\n"
    "- Cowgod's Chip-8 Technical Reference v1.0. Accessed at\n"
    "\t`http://devernay.free.fr/hacks/chip8/C8TECH10.HTM`\n"
    "and\n"
    "- Guide to making a CHIP-8 emulator by Tobias Langhoff. Accessed at\n"
    "`https://tobiasvl.github.io/blog/write-a-chip-8-emulator/`\n\n"
    "The following constants are part of the module:\n"
    "- 1) EXECOUT constants: controls the sucess of an emulation step.\n"
    "- 2) SIZE constants: controls the size of arrays or screen dimensions.\n"
    "- 3) MEM_ADDR constants: \n"
    "- 4) IMPL_MODE constants:\n"
);

static PyModuleDef snek8_core = {
    .m_base = PyModuleDef_HEAD_INIT,
    .m_name = "snek8.core",
    .m_doc = SNEK8_STR_DOC_PY8,
    .m_size = -1,
    .m_methods = module_meths,
};

PyMODINIT_FUNC
PyInit_core(void){
    PyObject* module;
    Py_Initialize();
    if (PyType_Ready(&Snek8EmulatorType) < 0){
        return NULL;
    }
    module = PyModule_Create(&snek8_core);
    if (!module){
        return NULL;
    }
    Py_INCREF(&Snek8EmulatorType);
    if (PyModule_AddObject(module, "Snek8Emulator", (PyObject*) &Snek8EmulatorType)){
        Py_DECREF(module);
    }
    PyModule_AddIntConstant(module, "EXECOUT_SUCCESS",
                            (long) SNEK8_EXECOUT_SUCCESS);
    PyModule_AddIntConstant(module, "EXECOUT_INVALID_OPCODE",
                            (long) SNEK8_EXECOUT_INVALID_OPCODE);
    PyModule_AddIntConstant(module, "EXECOUT_STACK_EMPTY",
                            (long) SNEK8_EXECOUT_STACK_EMPTY);
    PyModule_AddIntConstant(module, "EXECOUT_STACK_OVERFLOW",
                            (long) SNEK8_EXECOUT_STACK_OVERFLOW);
    PyModule_AddIntConstant(module, "EXECOUT_MEM_ADDR_OUT_BOUNDS",
                            (long) SNEK8_EXECOUT_MEM_ADDR_OUT_OF_BOUNDS);
    PyModule_AddIntConstant(module, "EXECOUT_ROM_FILE_NOT_FOUND",
                            (long) SNEK8_EXECOUT_ROM_FILE_NOT_FOUND);
    PyModule_AddIntConstant(module, "EXECOUT_ROM_FILE_FAILED_TO_OPEN",
                            (long) SNEK8_EXECOUT_ROM_FILE_FAILED_TO_OPEN);
    PyModule_AddIntConstant(module, "EXECOUT_ROM_FILE_FAILED_TO_READ",
                            (long) SNEK8_EXECOUT_ROM_FILE_FAILED_TO_READ);
    PyModule_AddIntConstant(module, "EXECOUT_ROM_FILE_EXCEEDS_MAX_MEM",
                            (long) SNEK8_EXECOUT_ROM_FILE_EXCEEDS_MAX_MEM);
    PyModule_AddIntConstant(module, "EXECOUT_EMPTY_STRUCT",
                            (long) SNEK8_EXECOUT_EMPTY_STRUCT);
    PyModule_AddIntConstant(module, "SIZE_KEYSET", SNEK8_SIZE_KEYSET);
    PyModule_AddIntConstant(module, "SIZE_STACK", SNEK8_SIZE_STACK);
    PyModule_AddIntConstant(module, "SIZE_REGISTERS", SNEK8_SIZE_REGISTERS);
    PyModule_AddIntConstant(module, "SIZE_RAM", SNEK8_SIZE_RAM);
    PyModule_AddIntConstant(module, "SIZE_MAX_ROM_FILE", SNEK8_SIZE_MAX_ROM_FILE);
    PyModule_AddIntConstant(module, "SIZE_GRAPHICS_WIDTH", SNEK8_GRAPHICS_WIDTH);
    PyModule_AddIntConstant(module, "SIZE_GRAPHICS_HEIGHT", SNEK8_GRAPHICS_HEIGTH);
    PyModule_AddIntConstant(module, "SIZE_GRAPHICS", SNEK8_SIZE_GRAPHICS);
    PyModule_AddIntConstant(module, "SIZE_FONTSET_PIXELS", SNEK8_SIZE_FONTSET_PIXELS);
    PyModule_AddIntConstant(module, "SIZE_FONTSET_SPRITE", SNEK8_SIZE_FONTSET_PIXEL_PER_SPRITE);
    PyModule_AddIntConstant(module, "MEM_ADDR_PROGRM_START", SNEK8_MEM_ADDR_PROG_START);
    PyModule_AddIntConstant(module, "MEM_ADDR_FONTSET_START", SNEK8_MEM_ADDR_FONTSET_START);
    PyModule_AddIntConstant(module, "IMPL_MODE_SHIFTS_USE_VY", SNEK8_IMPLM_MODE_SHIFTS_USE_VY);
    PyModule_AddIntConstant(module, "IMPL_MODE_BNNN_USE_VX", SNEK8_IMPLM_MODE_BNNN_USE_VX);
    PyModule_AddIntConstant(module, "IMPL_MODE_FX_CHANGE_I", SNEK8_IMPLM_MODE_FX_CHANGE_I);
    return module;
}

#ifdef __cplusplus
    }
#endif
#endif // SNEK8_CORE_H
