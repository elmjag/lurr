#define PY_SSIZE_T_CLEAN
#include <Python.h>


PyObject *
vm_dump_frame(PyObject *self, PyObject *args);

PyObject *
vm_save_state(PyObject *self, PyObject *args);

PyObject *
vm_restore_state(PyObject *self, PyObject *args);

int
vm_state_init();

static PyMethodDef VmMethods[] =
{
    {"dump_frame",  vm_dump_frame, METH_VARARGS,
     "debug function, print frame details to stdout"},
    {"save_state",  vm_save_state, METH_VARARGS, "TBD"},
    {"restore_state",  vm_restore_state, METH_VARARGS, "TBD"},
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

static struct PyModuleDef vm_module =
{
    PyModuleDef_HEAD_INIT,
    "lurr.vm",     /* name of module */
    NULL,        /* module documentation, may be NULL */
    -1,          /* size of per-interpreter state of the module,
                    or -1 if the module keeps state in global variables. */
    VmMethods
};

PyMODINIT_FUNC
PyInit_vm(void)
{
    if (!vm_state_init())
    {
        return NULL;
    }
    return PyModule_Create(&vm_module);
}
