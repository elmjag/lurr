#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "frameobject.h"

#define SET(n, v)         (stack_pointer[-(n)] = (v))
#define PEEK(n)           (stack_pointer[-(n)])

static PyObject **
_get_stack_pointer(PyFrameObject *frame)
{
    return frame->f_valuestack + frame->f_stackdepth;
}

PyObject *
vm_peek_stack(PyObject *self, PyObject *args)
{
    PyFrameObject *frame;

    const int stack_position;
    if (!PyArg_ParseTuple(args, "Oi", &frame, &stack_position))
    {
        return NULL;
    }

    PyObject **stack_pointer = _get_stack_pointer(frame);

    if (stack_position > frame->f_stackdepth)
    {
        fprintf(stderr, "peek_stack: stack_position %d > stackdepth %d\n",
                stack_position, frame->f_stackdepth);
        Py_RETURN_NONE;
    }

    PyObject *value = PEEK(stack_position);
    if (value == NULL)
    {
         fprintf(stderr, "peek_stack: value is NULL!\n");
         Py_RETURN_NONE;
    }

    Py_INCREF(value);

    return value;
}

PyObject *
vm_set_stack(PyObject *self, PyObject *args)
{
    PyFrameObject *frame;
    PyObject *value;

    const int stack_position;
    if (!PyArg_ParseTuple(args, "OiO", &frame, &stack_position, &value))
    {
        return NULL;
    }

    PyObject **stack_pointer = _get_stack_pointer(frame);

    if (stack_position > frame->f_stackdepth)
    {
        fprintf(stderr, "vm_set_stack: stack_position %d > stackdepth %d\n",
                stack_position, frame->f_stackdepth);
        Py_RETURN_NONE;
    }

    Py_DECREF(PEEK(stack_position));
    SET(stack_position, value);
    Py_INCREF(value);

    Py_RETURN_NONE;
}
