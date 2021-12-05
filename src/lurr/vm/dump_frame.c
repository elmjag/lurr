#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "frameobject.h"

static void
_dump_f_locals(PyObject *f_locals)
{
    if (f_locals == NULL)
    {
        printf("    no f_locals map\n");
        return;
    }

    PyObject *items = PyMapping_Items(f_locals);
    Py_ssize_t items_len = PySequence_Length(items);

    for (Py_ssize_t i = 0; i < items_len; i += 1)
    {
        PyObject *item = PySequence_GetItem(items, i);
        PyObject *key = PySequence_GetItem(item, 0);
        PyObject *value = PySequence_GetItem(item, 1);

        printf("    %s -> %p\n", PyUnicode_AsUTF8AndSize(key, NULL), value);

        Py_DECREF(value);
        Py_DECREF(key);
        Py_DECREF(item);
    }

    Py_DECREF(items);
}

static void
_dump_f_localsplus(PyObject *f_localsplus[1], int co_nlocals)
{
    for (int i = 0; i < co_nlocals; i += 1)
    {
        PyObject *item = f_localsplus[i];
        printf("    f_localsplus[%d] = %p (%s)\n", i, item, item->ob_type->tp_name);
    }
}

PyObject *
tr_vm_dump_frame(PyObject *self, PyObject *args)
{
    PyFrameObject *frame;

    if (!PyArg_ParseTuple(args, "O", &frame))
    {
        return NULL;
    }

    printf("PyFrameObject (%p) {\n"
           "  f_code (%p)\n"
           "  f_locals (%p)\n",
           frame, frame->f_code, frame->f_locals);
    _dump_f_locals(frame->f_locals);

    printf("  f_localsplus (%p)\n"
           "  f_code->co_nlocals %d\n",
           frame->f_localsplus,
           frame->f_code->co_nlocals);

    _dump_f_localsplus(frame->f_localsplus, frame->f_code->co_nlocals);

    printf("}\n");


    Py_RETURN_NONE;
}
