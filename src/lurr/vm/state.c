#include <stdbool.h>
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "frameobject.h"

#define _ERR false
#define _OK true

static PyObject *write_val_func = NULL;
static PyObject *read_val_func = NULL;


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
        Py_DECREF(args);
        return _ERR;
    }
    Py_DECREF(args);

    return _OK;
}

static PyObject *
_call_read_val(PyObject *file)
{
    PyObject *args = Py_BuildValue("(O)", file);
    if (args == NULL)
    {
        return NULL;
    }

    PyObject *val = PyObject_Call(read_val_func, args, NULL);
    Py_DECREF(args);

    return val;
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
_restore_fastlocals(PyObject *file, PyFrameObject *frame)
{
    int co_nlocals = frame->f_code->co_nlocals;

    for (int i = 0; i < co_nlocals; i += 1)
    {
        PyObject *val = _call_read_val(file);
        if (val == NULL)
        {
            return _ERR;
        }

        Py_DECREF(frame->f_localsplus[i]);
        frame->f_localsplus[i] = val;
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

static int
_restore_f_locals(PyObject *file, PyFrameObject *frame)
{
    PyObject *f_locals = frame->f_locals;
    if (f_locals == NULL)
    {
       return _OK;
    }

    PyObject *loaded_f_locals = _call_read_val(file);

    /*
     * delete all f_locals entries
     */
    PyObject *keys = PyMapping_Keys(f_locals);
    if (keys == NULL)
    {
        return _ERR;
    }
    Py_ssize_t keys_len = PySequence_Length(keys);
    PyObject *key;
    for (Py_ssize_t i = 0; i < keys_len; i += 1)
    {
        key = PySequence_GetItem(keys, i);
        if (PyMapping_DelItem(f_locals, key) == -1)
        {
            return _ERR;
        }
        Py_DECREF(key);
    }
    Py_DECREF(keys);

    /*
     * write loaded entries to f_locals
     */
    PyObject *items = PyMapping_Items(loaded_f_locals);
    if (items == NULL)
    {
        return _ERR;
    }

    Py_ssize_t items_len = PySequence_Length(items);
    PyObject *item, *value;

    for (Py_ssize_t i = 0; i < items_len; i += 1)
    {
        item = PySequence_GetItem(items, i);
        if (item == NULL)
        {
            return _ERR;
        }
        key = PySequence_GetItem(item, 0);
        if (key == NULL)
        {
            return _ERR;
        }

        value = PySequence_GetItem(item, 1);
        if (value == NULL)
        {
            return _ERR;
        }

        if (PyObject_SetItem(f_locals, key, value) == -1)
        {
            return _ERR;
        }

        Py_DECREF(value);
        Py_DECREF(key);
        Py_DECREF(item);
    }
    Py_DECREF(items);

    return _OK;
}

PyObject *
vm_save_state(PyObject *self, PyObject *args)
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

PyObject *
vm_restore_state(PyObject *self, PyObject *args)
{
    PyObject *file;
    PyFrameObject *frame;
    if (!PyArg_ParseTuple(args, "OO", &file, &frame))
    {
        return NULL;
    }

    if (_restore_fastlocals(file, frame) == _ERR)
    {
        return NULL;
    }

    if (_restore_f_locals(file, frame) == _ERR)
    {
        return NULL;
    }

    Py_RETURN_NONE;
}

int
vm_state_init()
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

    read_val_func = PyMapping_GetItemString(mod_dic, "read_val");
    if (read_val_func == NULL)
    {
        printf("error doing PyMapping_GetItemString\n");
        return _ERR;
    }

    Py_DECREF(mod_dic);
    Py_DECREF(mod);

    return _OK;
}
