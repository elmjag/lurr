#include <stdbool.h>
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "frameobject.h"

#define _ERR false
#define _OK true

static PyObject *write_val_func = NULL;


static int
_call_write_val(PyObject *file, PyObject *obj)
{
    PyObject *args = Py_BuildValue("OO", file, obj);
    if (args == NULL)
    {
        return _ERR;
    }

    if (PyObject_Call(write_val_func, args, NULL) == NULL)
    {
        return _ERR;
    }
    Py_DECREF(args);

    return _OK;
}

/**
 * save 'fastlocals' part of frame->f_localsplus,
 * that it is save the frame->f_localsplus[0:co_nlocals] slice
 */
static int
_save_fastlocals(PyObject *file, PyFrameObject *frame)
{
    int co_nlocals = frame->f_code->co_nlocals;

    for (int i = 0; i < co_nlocals; i += 1)
    {
        if (_call_write_val(file, frame->f_localsplus[i]) == _ERR)
        {
            return _ERR;
        }
    }

    return _OK;
}

static int
_save_f_locals(PyObject *file, PyFrameObject *frame)
{
    PyObject *f_locals = frame->f_locals;

    if (f_locals == NULL)
    {
       return _OK;
    }

    if (_call_write_val(file, f_locals) == _ERR)
    {
        return _ERR;
    }
    return _OK;
}

PyObject *
tr_vm_save_state(PyObject *self, PyObject *args)
{
    /*
     * parse call args
     */
    PyObject *file;
    PyFrameObject *frame;
    if (!PyArg_ParseTuple(args, "OO", &file, &frame))
    {
        return NULL;
    }

    if (_save_fastlocals(file, frame) == _ERR)
    {
        return NULL;
    }

    if (_save_f_locals(file, frame) == _ERR)
    {
        return NULL;
    }

    Py_RETURN_NONE;
}

int
tr_vm_state_init()
{
    PyObject *mod = PyImport_ImportModule("lurr.data");
    if (mod == NULL)
    {
        printf("error doing PyImport_ImportModule\n");
        return _ERR;
    }

    PyObject *mod_dic = PyModule_GetDict(mod);
    if (mod_dic == NULL)
    {
        printf("error doing PyModule_GetDict\n");
        return _ERR;
    }

    write_val_func = PyMapping_GetItemString(mod_dic, "write_val");
    if (write_val_func == NULL)
    {
        printf("error doing PyMapping_GetItemString\n");
        return _ERR;
    }

    Py_DECREF(mod_dic);
    Py_DECREF(mod);

    return _OK;
}
