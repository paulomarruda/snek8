/**
* @file core.c
* @author Paulo Arruda
* @license MIT
* @brief Implementation of the Python Py8 Emulator extension module.
* @note Built with Python 3.13.
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

PyDoc_STRVAR(PY8_STR_DOC_PY8_EMULATOR,
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

PyDoc_STRVAR(PY8_STR_DOC_EMULATOR_PY8_EMULATOR_IS_EMULATING,
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
        .doc = PY8_STR_DOC_EMULATOR_PY8_EMULATOR_IS_EMULATING,
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

/**
* @brief Retrieve the program counter.
*/
PyObject*
py8_emulatorGetPC(PyObject* self, PyObject* args){
    UNUSED(args);
    return Py_BuildValue("i", CAST_PTR(Py8Emulator, self)->ob_cpu.pc);
}

PyDoc_STRVAR(PY8_DOC_STR_PY8_EMULATOR_GET_PC,
             "Retrieve the program counter.\n"
             "Returns\n"
             "-------\n"
             "int\n"
             "\tThe current value of the program counter."
);

/**
* @brief Retrieve the delay timer register.
*/
PyObject*
py8_emulatorGetDT(PyObject* self, PyObject* args){
    UNUSED(args);
    return Py_BuildValue("i", CAST_PTR(Py8Emulator, self)->ob_cpu.dt);
}

PyDoc_STRVAR(PY8_DOC_STR_PY8_EMULATOR_GET_DT,
             "Retrieve the delay timer register.\n"
             "Returns\n"
             "-------\n"
             "int\n"
             "\tThe current value of the delay timer."
);

/**
* @brief Retrieve the sound timer register.
*/
PyObject*
py8_emulatorGetST(PyObject* self, PyObject* args){
    UNUSED(args);
    return Py_BuildValue("i", CAST_PTR(Py8Emulator, self)->ob_cpu.st);
}

PyDoc_STRVAR(PY8_DOC_STR_PY8_EMULATOR_GET_ST,
             "Retrieve the sound timer register.\n"
             "Returns\n"
             "-------\n"
             "int\n"
             "\tThe current value of the sount timer."
);

/**
* @brief Retrieve the index register.
*/
PyObject*
py8_emulatorGetIR(PyObject* self, PyObject* args){
    UNUSED(args);
    return Py_BuildValue("i", CAST_PTR(Py8Emulator, self)->ob_cpu.ir);
}

PyDoc_STRVAR(PY8_DOC_STR_PY8_EMULATOR_GET_IR,
             "Retrieve the index register.\n"
             "Returns\n"
             "-------\n"
             "int\n"
             "\tThe current value of the index register."
);

/**
* @brief Retrieve the stack pointer.
*/
PyObject*
py8_emulatorGetSP(PyObject* self, PyObject* args){
    UNUSED(args);
    return Py_BuildValue("i", CAST_PTR(Py8Emulator, self)->ob_cpu.sp);
}

PyDoc_STRVAR(PY8_DOC_STR_PY8_EMULATOR_GET_SP,
             "Retrieve the stack pointer.\n"
             "Returns\n"
             "-------\n"
             "int\n"
             "\tThe current value of the stack pointer."
);

PyObject*
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
             "Retrieve the values of the registers.\n"
             "Returns\n"
             "-------\n"
             "List[int]\n"
             "\tThe current value of the registers."
);

PyObject*
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
             "Retrieve the graphics list representation.\n"
             "Returns\n"
             "-------\n"
             "List[bool]\n"
             "\tThe current value of the graphic list representation."
);

PyObject*
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
             "Retrieve the stack list representation.\n"
             "Returns\n"
             "-------\n"
             "List[bool]\n"
             "\tThe current value of the stack list representation."
);

PyObject*
py8_emulatorGetKeyValue(PyObject* self, PyObject* args){
    int index = 0;
    if (!PyArg_ParseTuple(args, "i", &index)){
        return NULL;
    }
    bool value = py8_cpuGetKeyVal(CAST_PTR(Py8Emulator, self)->ob_cpu, index);
    if (value){
        Py_RETURN_TRUE;
    }else{
        Py_RETURN_FALSE;
    }
}

PyDoc_STRVAR(PY8_DOC_STR_PY8_EMULATOR_GET_KEY_VALUE,
             "Retrieve the graphics list representation.\n"
             "Attributes\n"
             "----------\n"
             "index: int\n"
             "\tThe index to be retrieved.\n\n"
             "Returns\n"
             "-------\n"
             "List[bool]\n"
             "\tThe current value of the graphic list representation."
);

/*
* EMULATION METHODS
* ------------------
*/

PyObject*
py8_emulatorSetKeyValue(PyObject* self, PyObject* args){
    int index = 0;
    bool value;
    if (!PyArg_ParseTuple(args, "ib", &index, &value)){
        return NULL;
    }
    py8_cpuSetKey(&CAST_PTR(Py8Emulator, self)->ob_cpu, index, value);
    Py_RETURN_NONE;
}

PyDoc_STRVAR(PY8_DOC_STR_PY8_EMULATOR_SET_KEY_VALUE,
             "Retrieve the graphics list representation.\n"
             "Attributes\n"
             "----------\n"
             "index: int\n"
             "\tThe index to be retrieved.\n"
             "value: bool\n"
             "\tThe index to be retrieved.\n\n"
             "Returns\n"
             "-------\n"
             "List[bool]\n"
             "\tThe current value of the graphic list representation."
);

PyObject*
_py8_emulatorExecOpc(PyObject* self, PyObject* args){
    int code = 0;
    if (!PyArg_ParseTuple(args, "i", &code)){
        return NULL;
    }
    Py8Opcode opc = py8_opcodeInit((uint16_t) code);
    Py8Instruction instruction = py8_getInstruction(opc);
    enum Py8ExecutionOutput out = instruction.exec(&CAST_PTR(Py8Emulator, self)->ob_cpu, opc, instruction.code);
    PyObject* ret = Py_False;
    switch (out) {
        // TO-DO: write more granular returns.
        case PY8_EXECOUT_SUCCESS:
            ret = Py_True;
            break;
        default:
            break;
    }
    return ret;
}

PyDoc_STRVAR(PY8_DOC_STR_PY8_EMULATOR_EXEC_OPT,
             "Retrieve the graphics list representation.\n"
             "Attributes\n"
             "----------\n"
             "index: int\n"
             "\tThe index to be retrieved.\n"
             "value: bool\n"
             "\tThe index to be retrieved.\n\n"
             "Returns\n"
             "-------\n"
             "bool\n"
             "\tThe current value of the graphic list representation."
);

static struct PyMethodDef py8_emulator_methods[] = {
    // {
    //     .ml_name = "loadKeys",
    //     .ml_meth = ch8Emulator_loadKeys,
    //     .ml_flags = METH_VARARGS,
    //     .ml_doc = loadKeysDoc,
    // },
    {NULL},
};

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
     .tp_name = "_ch8core_._Ch8Emulator",
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
    if (PyModule_AddObject(module, "_Ch8Emulator", (PyObject*) &Py8EmulatorType)){
        Py_DECREF(module);
    }
    return module;
}

#ifdef __cplusplus
    }
#endif
#endif // PY8_CORE_H
