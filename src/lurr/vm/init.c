#define PY_SSIZE_T_CLEAN
#include <Python.h>


PyObject *
vm_dump_frame(PyObject *self, PyObject *args);

PyObject *
vm_peek_stack(PyObject *self, PyObject *args);

PyObject *
vm_save_state(PyObject *self, PyObject *args);

PyObject *
vm_restore_state(PyObject *self, PyObject *args);

PyObject *
vm_set_stack(PyObject *self, PyObject *args);

int
vm_state_init();

static PyMethodDef VmMethods[] =
{
    {
        "dump_frame",  vm_dump_frame, METH_VARARGS,
        "Debug function, print frame details to stdout."
    },
    {
        "peek_stack",  vm_peek_stack, METH_VARARGS,
        "Get value from VM stack for specified frame.\n"
        "\n"
        "Arguments:\n"
        "\n"
        " frame   - frame object\n"
        " depth   - stack position\n"
        "\n"
        "Example:\n"
        "\n"
        "    peek_stack(my_frame, 2)\n"
        "\n"
        "Will look-up value in 'my_frame' stack, at depth 2."
    },
    {
        "set_stack",  vm_set_stack, METH_VARARGS,
        "Overwrite value on VM stack for specified frame."
        "\n"
        "Arguments:\n"
        "\n"
        " frame   - frame object\n"
        " depth   - stack position\n"
        " value   - new value to set\n"
        "\n"
        "Example:\n"
        "\n"
        "    set_stack(my_frame, 3, 'foo')\n"
        "\n"
        "Will overwrite 'my_frame' stack, at depth 3 with string 'foo'."
    },
    {
        "save_state",  vm_save_state, METH_VARARGS,
        "TBD"
    },
    {
        "restore_state",  vm_restore_state, METH_VARARGS,
        "TBD"
    },
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
