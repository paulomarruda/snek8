/**
* @file core.c
* @author Paulo Arruda
* @license MIT
* @brief Implementation of the Python Py8 Emulator extension module.
* @note Built with Python 3.13.
*/
#ifndef PY8_CORE_C
    #define PY8_CORE_C
#ifdef __cplusplus
    extern "C"{
#endif
#include <Python.h>
#include "cpu.h"

typedef struct{
    PyObject_HEAD
    Py8CPU ob_cpu;
    bool ob_is_emulating;
    uint32_t ob_fps;
} Py8Emulator;

PyDoc_STRVAR(PY8_STR_DOC_PY8_EMULATOR,
             "_Py8Emulator(implm: int, fps: int)\n"
             "Chip8's CPU Emulator.\n\n"
             "Attributes\n"
             "----------\n"
             "is_emulating: bool\n"
             "\tControls whether the emulation process is running.\n"
             "fps: int\n"
             "\tFrames per second\n"
             "\n\n"
             "Parameters\n"
             "----------\n"
             "implm: int\n"
             "\tWhich implementation to use. The possible values are:\n"
             "\t\t-0: The original COSMAC-VIP implementation.\n"
             "\t\t-1: Modern implementations.\n"
             "fps: int\n"
             "\tFrames per second"
);

PyDoc_STRVAR(PY8_STR_DOC_EMULATOR_PY8_EMULATOR_IS_EMULATING,
    "Controls whether the emulation is running.\n"
    "This is a read-only attribute that can only be modified by the emulation process."
);

PyDoc_STRVAR(PY8_STR_DOC_EMULATOR_PY8_EMULATOR_FPS,
    "Frames per second.\n"
    "This is a read-only attribute that can only be modified by the `setFPS`,\n."
    "`increaseFPS` or `decreaseFPS` methods.\n"
);

/**
* @brief Declaration of the emulator attributes.
*/
static PyMemberDef py8_emulator_members[] = {
    {
        .name = "is_emulating",
        .type = Py_T_BOOL,
        .offset = offsetof(Py8Emulator, ob_is_emulating),
        .flags = Py_READONLY, 
        .doc = PY8_STR_DOC_EMULATOR_PY8_EMULATOR_IS_EMULATING,
    },
    {
        .name = "fps",
        .type = Py_T_INT,
        .offset = offsetof(Py8Emulator, ob_fps),
        .flags = Py_READONLY, 
        .doc = PY8_STR_DOC_EMULATOR_PY8_EMULATOR_FPS,
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
    int fps = 30;
    char* kwlist[] = {
        "implm",
        "fps",
        NULL,
    };
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|ii", kwlist, &mode, &fps)){
        return -1;
    }
    CAST_PTR(Py8Emulator, self)->ob_is_emulating = true;
    if (fps <= 0){
        PyErr_Format(PyExc_ValueError, "FPS value cannot <= 0.");
        return -1;
    }
    CAST_PTR(Py8Emulator, self)->ob_fps = fps;
    switch (mode) {
        case 0:
            py8_cpuInit(&CAST_PTR(Py8Emulator, self)->ob_cpu, PY8_IMPLM_MODE_COSMAC_VIP);
            break;
        case 1:
            py8_cpuInit(&CAST_PTR(Py8Emulator, self)->ob_cpu, PY8_IMPLM_MODE_MODERN);
            break;
        default:
            PyErr_Format(PyExc_ValueError, "Value %d is invalid for implementation.", mode);
            return -1;
    }
    return 0;
}

/*
* CONSTANT METHODS
* ------------------
*/

static PyObject*
py8_emulatorGetMode(PyObject* self, PyObject* args){
    UNUSED(args);
    return PyLong_FromLong((long) CAST_PTR(Py8Emulator, self)->ob_cpu.mode);
}

PyDoc_STRVAR(PY8_DOC_STR_PY8_EMULATOR_GET_MODE,
             "getMode() -> int\n"
             "Retrieve the implementation mode.\n"
             "Returns\n"
             "-------\n"
             "int\n"
             "\tThe current value of the implementation mode."
);

/**
* @brief Retrieve the program counter.
*/
static PyObject*
py8_emulatorGetPC(PyObject* self, PyObject* args){
    UNUSED(args);
    return Py_BuildValue("i", CAST_PTR(Py8Emulator, self)->ob_cpu.pc);
}

PyDoc_STRVAR(PY8_DOC_STR_PY8_EMULATOR_GET_PC,
             "getPC() -> int\n"
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
py8_emulatorGetDT(PyObject* self, PyObject* args){
    UNUSED(args);
    return Py_BuildValue("i", CAST_PTR(Py8Emulator, self)->ob_cpu.dt);
}

PyDoc_STRVAR(PY8_DOC_STR_PY8_EMULATOR_GET_DT,
             "getDT() -> int\n"
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
py8_emulatorGetST(PyObject* self, PyObject* args){
    UNUSED(args);
    return Py_BuildValue("i", CAST_PTR(Py8Emulator, self)->ob_cpu.st);
}

PyDoc_STRVAR(PY8_DOC_STR_PY8_EMULATOR_GET_ST,
             "getST() -> int\n"
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
py8_emulatorGetIR(PyObject* self, PyObject* args){
    UNUSED(args);
    return Py_BuildValue("i", CAST_PTR(Py8Emulator, self)->ob_cpu.ir);
}

PyDoc_STRVAR(PY8_DOC_STR_PY8_EMULATOR_GET_IR,
             "getIR() -> int\n"
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
py8_emulatorGetSP(PyObject* self, PyObject* args){
    UNUSED(args);
    return Py_BuildValue("i", CAST_PTR(Py8Emulator, self)->ob_cpu.sp);
}

PyDoc_STRVAR(PY8_DOC_STR_PY8_EMULATOR_GET_SP,
             "getSP() -> int\n"
             "Retrieve the stack pointer.\n"
             "Returns\n"
             "-------\n"
             "int\n"
             "\tThe current value of the stack pointer."
);

static PyObject*
py8_emulatorGetRegister(PyObject* self, PyObject* args, PyObject* kwargs){
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
    return Py_BuildValue("i", CAST_PTR(Py8Emulator, self)->ob_cpu.registers[index]);
}

PyDoc_STRVAR(PY8_DOC_STR_PY8_EMULATOR_GET_REGISTER,
             "getRegister(index: int) -> int\n"
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

static PyObject*
py8_emulatorGetRegisters(PyObject* self, PyObject* args){
    UNUSED(args);
    PyObject* register_list = PyList_New(PY8_SIZE_REGISTERS);
    if (!register_list){
        PyErr_SetString(PyExc_MemoryError, "Failed to create register list.");
        return NULL;
    }
    for (size_t i = 0; i < PY8_SIZE_REGISTERS; i++){
        PyObject* value = Py_BuildValue("i", CAST_PTR(Py8Emulator, self)->ob_cpu.registers[i]);
        if (!value){
            PyErr_SetString(PyExc_MemoryError, "Failed to create register list element.");
            Py_DECREF(register_list);
            return NULL;
        }
        PyList_SET_ITEM(register_list, i, value);
    }
    return register_list;
}

PyDoc_STRVAR(PY8_DOC_STR_PY8_EMULATOR_GET_REGISTERS,
             "getRegisters() -> List[int]\n"
             "Retrieve all values of the registers.\n"
             "Returns\n"
             "-------\n"
             "List[int]\n"
             "\tThe current value of the registers."
);

static PyObject*
py8_emulatorGetGraphics(PyObject* self, PyObject* args){
    UNUSED(args);
    PyObject* graphics_list = PyList_New(PY8_SIZE_GRAPHICS);
    if (!graphics_list){
        PyErr_SetString(PyExc_MemoryError, "Failed to create graphics list.");
        return NULL;
    }
    for (size_t i = 0; i < PY8_SIZE_GRAPHICS; i++){
        if (CAST_PTR(Py8Emulator, self)->ob_cpu.graphics[i]){
            PyList_SET_ITEM(graphics_list, i, Py_True);
        }else{
            PyList_SET_ITEM(graphics_list, i, Py_False);
        }
    }
    return graphics_list;
}

PyDoc_STRVAR(PY8_DOC_STR_PY8_EMULATOR_GET_GRAPHICS,
             "getGraphics() -> List[bool]"
             "Retrieve the graphics list representation.\n"
             "Returns\n"
             "-------\n"
             "List[bool]\n"
             "\tThe current value of the graphic list representation."
);

static PyObject*
py8_emulatorGetStack(PyObject* self, PyObject* args){
    UNUSED(args);
    PyObject* stack_list = PyList_New(PY8_SIZE_STACK);
    if (!stack_list){
        PyErr_SetString(PyExc_MemoryError, "Failed to create stack list.");
        return NULL;
    }
    for (size_t i = 0; i < PY8_SIZE_STACK; i++){
        PyObject* value = Py_BuildValue("i", CAST_PTR(Py8Emulator, self)->ob_cpu.stack[i]);
        if (!value){
            PyErr_SetString(PyExc_MemoryError, "Failed to create register list element.");
            Py_DECREF(stack_list);
            return NULL;
        }
        PyList_SET_ITEM(stack_list, i, value);
    }
    return stack_list;
}

PyDoc_STRVAR(PY8_DOC_STR_PY8_EMULATOR_GET_STACK,
             "getStack() -> List[int]\n"
             "Retrieve the stack list representation.\n"
             "Returns\n"
             "-------\n"
             "List[int]\n"
             "\tThe current value of the stack list representation."
);

static PyObject*
py8_emulatorGetKeyValue(PyObject* self, PyObject* args, PyObject* kwargs){
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
    bool value = py8_cpuGetKeyVal(CAST_PTR(Py8Emulator, self)->ob_cpu, key);
    return value? Py_True: Py_False;
}

PyDoc_STRVAR(PY8_DOC_STR_PY8_EMULATOR_GET_KEY_VALUE,
             "getKeyValue(key: int) -> bool\n"
             "Retrieve the graphics list representation.\n"
             "Attributes\n"
             "----------\n"
             "key: int\n"
             "\tThe key index to be retrieved. 0 <= key <= 15\n\n"
             "Returns\n"
             "-------\n"
             "bool\n"
             "\tThe current value of the requested key."
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
py8_emulatorSetFPS(PyObject* self, PyObject* args, PyObject* kwargs){
    int fps;
    char* kwlist[] = {
        "fps",
        NULL,
    };
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "i", kwlist, &fps)){
        return NULL;
    }
    if (fps <= 0){
        PyErr_Format(PyExc_ValueError, "FPS value cannot <= 0.");
        return NULL;
    }
    CAST_PTR(Py8Emulator, self)->ob_fps = fps;
    Py_RETURN_NONE;
}

PyDoc_STRVAR(PY8_DOC_STR_PY8_EMULATOR_SET_FPS,
             "setFPS(fps: int) -> None\n"
             "Modifies the value of the emulation's FPS.\n\n"
             "Attributes\n"
             "----------\n"
             "fps: int\n"
             "\tThe new FPS value.\n"
             "Raises\n"
             "------\n"
             "ValueError\n"
             "\tIf the fps value is less than or equal to zero."

);

static PyObject*
py8_emulatorIncreaseFPS(PyObject* self, PyObject* args){
    UNUSED(args);
    CAST_PTR(Py8Emulator, self)->ob_fps += 10;
    Py_RETURN_NONE;
}

PyDoc_STRVAR(PY8_DOC_STR_PY8_EMULATOR_INCR_FPS,
             "increaseFPS() -> None\n"
             "Increases the emulation's FPS by 10.\n\n"
);


static PyObject*
py8_emulatorDecreaseFPS(PyObject* self, PyObject* args){
    UNUSED(args);
    CAST_PTR(Py8Emulator, self)->ob_fps -= 10;
    Py_RETURN_NONE;
}

PyDoc_STRVAR(PY8_DOC_STR_PY8_EMULATOR_DECR_FPS,
             "decreaseFPS() -> None\n"
             "Decreases the emulation's FPS by 10 if the resulting FPS not less\n"
             "than or equal to zero.\n"
);

static PyObject*
py8_emulatorSetMode(PyObject* self, PyObject* args, PyObject* kwargs){
    int mode;
    char* kwlist[] = {
        "implm",
        NULL,
    };
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "i", kwlist, &mode)){
        return NULL;
    }
    switch (mode) {
        case 0:
            CAST_PTR(Py8Emulator, self)->ob_cpu.mode = PY8_IMPLM_MODE_COSMAC_VIP;
            break;
        case 1:
            CAST_PTR(Py8Emulator, self)->ob_cpu.mode = PY8_IMPLM_MODE_MODERN;
            break;
        default:
            PyErr_Format(PyExc_ValueError, "Value %d is invalid for implementation.", mode);
            break;
    }
    Py_RETURN_NONE;
}

PyDoc_STRVAR(PY8_DOC_STR_PY8_EMULATOR_SET_MODE,
             "setMode(implm: int) -> None\n"
             "Modify the emulator's implementation mode.\n\n"
             "Attributes\n"
             "----------\n"
             "implm: int\n"
             "\tWhich implementation to use. Possible values are:\n"
             "\t\t-0: The original COSMAC-VIP implementation.\n"
             "\t\t-1: Modern implementations.\n"
             "Raises\n"
             "------\n"
             "ValueError\n"
             "\tIf the implm is not a valid value."
);

static PyObject*
py8_emulatorSetKeyValue(PyObject* self, PyObject* args, PyObject* kwargs){
    int index = 0;
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
    py8_cpuSetKey(&CAST_PTR(Py8Emulator, self)->ob_cpu, index, value);
    Py_RETURN_NONE;
}

PyDoc_STRVAR(PY8_DOC_STR_PY8_EMULATOR_SET_KEY_VALUE,
             "setKeyValue(key: int, value: bool) -> None\n"
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
_py8_emulatorExecOpc(PyObject* self, PyObject* args, PyObject* kwargs){
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
    Py8Opcode opc = py8_opcodeInit((uint16_t) code);
    Py8Instruction instruction = py8_getInstruction(opc);
    enum Py8ExecutionOutput out = instruction.exec(&CAST_PTR(Py8Emulator, self)->ob_cpu, opc, instruction.code);
    return PyLong_FromLong((long) out);
}

PyDoc_STRVAR(PY8_DOC_STR_PY8_EMULATOR_EXEC_OPT,
             "_execOpc(opcode: int) -> int\n"
             "Execute a step in the emulation process determined by the opcode.\n\n"
             "This function should only be used to test CPU functionallities.\n\n"
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
py8_emulatorLoadRom(PyObject* self, PyObject* args, PyObject* kwargs){
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
    enum Py8ExecutionOutput out = py8_cpuLoadRom(&CAST_PTR(Py8Emulator, self)->ob_cpu, rom_filepath);
    return PyBool_FromLong((long) out);
}

PyDoc_STRVAR(PY8_DOC_STR_PY8_EMULATOR_LOAD_ROM,
             "loadRom(rom_filepath: str) -> int\n"
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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"

static struct PyMethodDef py8_emulator_methods[] = {
    {
        .ml_name = "getMode",
        .ml_meth = py8_emulatorGetMode,
        .ml_flags = METH_NOARGS,
        .ml_doc = PY8_DOC_STR_PY8_EMULATOR_GET_MODE,
    },
    {
        .ml_name = "getPC",
        .ml_meth = py8_emulatorGetPC,
        .ml_flags = METH_NOARGS,
        .ml_doc = PY8_DOC_STR_PY8_EMULATOR_GET_PC,
    },
    {
        .ml_name = "getIR",
        .ml_meth = py8_emulatorGetIR,
        .ml_flags = METH_NOARGS,
        .ml_doc = PY8_DOC_STR_PY8_EMULATOR_GET_IR,
    },
    {
        .ml_name = "getDT",
        .ml_meth = py8_emulatorGetDT,
        .ml_flags = METH_NOARGS,
        .ml_doc = PY8_DOC_STR_PY8_EMULATOR_GET_DT,
    },
    {
        .ml_name = "getST",
        .ml_meth = py8_emulatorGetST,
        .ml_flags = METH_NOARGS,
        .ml_doc = PY8_DOC_STR_PY8_EMULATOR_GET_ST,
    },
    {
        .ml_name = "getSP",
        .ml_meth = py8_emulatorGetSP,
        .ml_flags = METH_NOARGS,
        .ml_doc = PY8_DOC_STR_PY8_EMULATOR_GET_SP,
    },
    {
        .ml_name = "getGraphics",
        .ml_meth = py8_emulatorGetGraphics,
        .ml_flags = METH_NOARGS,
        .ml_doc = PY8_DOC_STR_PY8_EMULATOR_GET_GRAPHICS,
    },
    {
        .ml_name = "getStack",
        .ml_meth = py8_emulatorGetStack,
        .ml_flags = METH_NOARGS,
        .ml_doc = PY8_DOC_STR_PY8_EMULATOR_GET_STACK,
    },
    {
        .ml_name = "getRegisters",
        .ml_meth = py8_emulatorGetRegisters,
        .ml_flags = METH_NOARGS,
        .ml_doc = PY8_DOC_STR_PY8_EMULATOR_GET_REGISTERS,
    },
    {
        .ml_name = "getRegister",
        .ml_meth = (PyCFunction) py8_emulatorGetRegister,
        .ml_flags = METH_VARARGS | METH_KEYWORDS,
        .ml_doc = PY8_DOC_STR_PY8_EMULATOR_GET_REGISTER,
    },
    {
        .ml_name = "loadRom",
        .ml_meth = (PyCFunction) py8_emulatorLoadRom,
        .ml_flags = METH_VARARGS | METH_KEYWORDS,
        .ml_doc = PY8_DOC_STR_PY8_EMULATOR_LOAD_ROM,
    },
    {
        .ml_name = "setMode",
        .ml_meth = (PyCFunction) py8_emulatorSetMode,
        .ml_flags = METH_VARARGS,
        .ml_doc = PY8_DOC_STR_PY8_EMULATOR_SET_MODE,
    },
    {
        .ml_name = "setFPS",
        .ml_meth = (PyCFunction) py8_emulatorSetFPS,
        .ml_flags = METH_VARARGS | METH_KEYWORDS,
        .ml_doc = PY8_DOC_STR_PY8_EMULATOR_SET_FPS,
    },
    {
        .ml_name = "increaseFPS",
        .ml_meth = py8_emulatorIncreaseFPS,
        .ml_flags = METH_NOARGS,
        .ml_doc = PY8_DOC_STR_PY8_EMULATOR_INCR_FPS,
    },
    {
        .ml_name = "decreaseFPS",
        .ml_meth = py8_emulatorDecreaseFPS,
        .ml_flags = METH_NOARGS,
        .ml_doc = PY8_DOC_STR_PY8_EMULATOR_DECR_FPS,
    },
    {
        .ml_name = "getKeyValue",
        .ml_meth = (PyCFunction) py8_emulatorGetKeyValue,
        .ml_flags = METH_VARARGS | METH_KEYWORDS,
        .ml_doc = PY8_DOC_STR_PY8_EMULATOR_GET_KEY_VALUE,
    },
    {
        .ml_name = "setKeyValue",
        .ml_meth = (PyCFunction) py8_emulatorSetKeyValue,
        .ml_flags = METH_VARARGS | METH_KEYWORDS,
        .ml_doc = PY8_DOC_STR_PY8_EMULATOR_SET_KEY_VALUE,
    },
    {
        .ml_name = "_execOpc",
        .ml_meth = (PyCFunction) _py8_emulatorExecOpc,
        .ml_flags = METH_VARARGS | METH_KEYWORDS,
        .ml_doc = PY8_DOC_STR_PY8_EMULATOR_EXEC_OPT,
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

static PyTypeObject Py8EmulatorType = {
     .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
     .tp_name = "_py8core_._Py8Emulator",
     .tp_basicsize = sizeof(Py8Emulator),
     .tp_itemsize = 0,
     .tp_doc = PY8_STR_DOC_PY8_EMULATOR,
     .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
     .tp_new = (newfunc) py8_emulatorNew,
     .tp_init = (initproc) py8_emulatorInit,
     .tp_dealloc = (destructor) py8_emulatorDel,
     .tp_members = py8_emulator_members,
     .tp_methods = py8_emulator_methods,
};

PyDoc_STRVAR(PY8_STR_DOC_PY8,
    ""
);

static PyModuleDef py8_core = {
    .m_base = PyModuleDef_HEAD_INIT,
    .m_name = "_py8core_",
    .m_doc = PY8_STR_DOC_PY8,
    .m_size = -1,
    .m_methods = module_meths,
};

PyMODINIT_FUNC
PyInit__py8core_(void){
    PyObject* module;
    Py_Initialize();
    if (PyType_Ready(&Py8EmulatorType) < 0){
        return NULL;
    }
    module = PyModule_Create(&py8_core);
    if (!module){
        return NULL;
    }
    Py_INCREF(&Py8EmulatorType);
    if (PyModule_AddObject(module, "_Py8Emulator", (PyObject*) &Py8EmulatorType)){
        Py_DECREF(module);
    }
    PyModule_AddIntConstant(module, "EXECOUT_SUCCESS",
                            (long) PY8_EXECOUT_SUCCESS);
    PyModule_AddIntConstant(module, "EXECOUT_INVALID_OPCODE",
                            (long) PY8_EXECOUT_INVALID_OPCODE);
    PyModule_AddIntConstant(module, "EXECOUT_STACK_EMPTY",
                            (long) PY8_EXECOUT_STACK_EMPTY);
    PyModule_AddIntConstant(module, "EXECOUT_STACK_OVERFLOW",
                            (long) PY8_EXECOUT_STACK_OVERFLOW);
    PyModule_AddIntConstant(module, "EXECOUT_MEM_ADDR_OUT_BOUNDS",
                            (long) PY8_EXECOUT_MEM_ADDR_OUT_OF_BOUNDS);
    PyModule_AddIntConstant(module, "EXECOUT_ROM_FILE_NOT_FOUND",
                            (long) PY8_EXECOUT_ROM_FILE_NOT_FOUND);
    PyModule_AddIntConstant(module, "EXECOUT_ROM_FILE_FAILED_TO_OPEN",
                            (long) PY8_EXECOUT_ROM_FILE_FAILED_TO_OPEN);
    PyModule_AddIntConstant(module, "EXECOUT_ROM_FILE_FAILED_TO_READ",
                            (long) PY8_EXECOUT_ROM_FILE_FAILED_TO_READ);
    PyModule_AddIntConstant(module, "EXECOUT_ROM_FILE_EXCEEDS_MAX_MEM",
                            (long) PY8_EXECOUT_ROM_FILE_EXCEEDS_MAX_MEM);
    PyModule_AddIntConstant(module, "EXECOUT_EMPTY_STRUCT",
                            (long) PY8_EXECOUT_EMPTY_STRUCT);
    PyModule_AddIntConstant(module, "SIZE_KEYSET", PY8_SIZE_KEYSET);
    PyModule_AddIntConstant(module, "SIZE_STACK", PY8_SIZE_STACK);
    PyModule_AddIntConstant(module, "SIZE_REGISTERS", PY8_SIZE_REGISTERS);
    PyModule_AddIntConstant(module, "SIZE_RAM", PY8_SIZE_RAM);
    PyModule_AddIntConstant(module, "SIZE_MAX_ROM_FILE", PY8_SIZE_MAX_ROM_FILE);
    PyModule_AddIntConstant(module, "SIZE_GRAPHICS_WIDTH", PY8_GRAPHICS_WIDTH);
    PyModule_AddIntConstant(module, "SIZE_GRAPHICS_HEIGHT", PY8_GRAPHICS_HEIGTH);
    PyModule_AddIntConstant(module, "SIZE_GRAPHICS", PY8_SIZE_GRAPHICS);
    PyModule_AddIntConstant(module, "MEM_ADDR_PROGRM_START", PY8_MEM_ADDR_PROG_START);
    PyModule_AddIntConstant(module, "SIZE_FONTSET_PIXELS", PY8_SIZE_FONTSET_PIXEL);
    PyModule_AddIntConstant(module, "SIZE_FONTSET_SPRITE", PY8_SIZE_FONTSET_SPRITE);
    PyModule_AddIntConstant(module, "MEM_ADDR_FONTSET_START", PY8_MEM_ADDR_FONTSET_START);
    return module;
}

#ifdef __cplusplus
    }
#endif
#endif // PY8_CORE_H
