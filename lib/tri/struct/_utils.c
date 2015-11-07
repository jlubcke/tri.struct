#include "Python.h"


PyObject *
str_from_string(const char *str)
{
#if PY_MAJOR_VERSION >= 3
    return PyUnicode_FromString(str);
#else
    return PyString_FromString(str);
#endif
}


PyObject *
str_format(PyObject *fmt, PyObject *args)
{
#if PY_MAJOR_VERSION >= 3
    return PyUnicode_Format(fmt, args);
#else
    return PyString_Format(fmt, args);
#endif
}


PyObject *
format_with_type(PyTypeObject *type, PyObject *inner)
{
    PyObject *fmt = NULL, *fmt_args = NULL;
    PyObject *type_name = NULL;
    PyObject *result = NULL;

    type_name = PyObject_GetAttrString((PyObject *)type, "__name__");
    if (type_name == NULL)
        goto done;;

    fmt = str_from_string(inner == NULL? "%s()" : "%s(%s)");
    if (fmt == NULL)
        goto done;

    if (inner == NULL) {
        fmt_args = PyTuple_Pack(1, type_name);
    }
    else {
        fmt_args = PyTuple_Pack(2, type_name, inner);
    }

    if (fmt_args == NULL) {
        goto done;
    }

    result = str_format(fmt, fmt_args);
done:
    Py_XDECREF(type_name);
    Py_XDECREF(fmt);
    Py_XDECREF(fmt_args);
    return result;
}


void
str_concat(PyObject **left, PyObject *right)
{
#if PY_MAJOR_VERSION >= 3
    PyObject *val;
    val = PyUnicode_Concat(*left, right);
    Py_DECREF(*left);
    *left = val;
#else
    PyString_Concat(left, right);
#endif
}


void
str_concat_and_del(PyObject **left, PyObject *right)
{
#if PY_MAJOR_VERSION >= 3
    str_concat(left, right);
    Py_XDECREF(right);
#else
    PyString_ConcatAndDel(left, right);
#endif
}


PyObject *
str_join(PyObject *separator, PyObject *seq)
{
#if PY_MAJOR_VERSION >= 3
    return PyUnicode_Join(separator, seq);
#else
    return _PyString_Join(separator, seq);
#endif
}
