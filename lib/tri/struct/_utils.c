#include "Python.h"

#define _LOCAL_ __attribute__((visibility("hidden")))


_LOCAL_ void
set_attribute_error(PyObject *self, PyObject *name)
{
#if PY_MAJOR_VERSION >= 3
    PyErr_Format(PyExc_AttributeError,
                 "'%.100s' object has no attribute '%U'",
                 Py_TYPE(self)->tp_name, name);
#else
    PyErr_Format(PyExc_AttributeError,
                 "'%.100s' object has no attribute '%s'",
                 Py_TYPE(self)->tp_name, PyString_AS_STRING(name));
#endif
}


/* "re-implement" `PyDict_GetItemWithError` for getattr in Python 2.7
    to get around calling `__missing__()`, which shouldn't be done
    for getattr.
 */
#if PY_MAJOR_VERSION < 3
_LOCAL_ void
set_key_error(PyObject *arg)
{
    PyObject *tup;
    tup = PyTuple_Pack(1, arg);
    if (!tup)
        return; /* caller will expect error to be set anyway */
    PyErr_SetObject(PyExc_KeyError, tup);
    Py_DECREF(tup);
}

_LOCAL_ PyObject *
PyDict_GetItemWithError(PyObject *op, PyObject *key)
{
    long hash;
    PyDictObject *mp = (PyDictObject *)op;
    PyDictEntry *ep;

    if (!PyString_CheckExact(key) ||
        (hash = ((PyStringObject *) key)->ob_shash) == -1)
    {
        hash = PyObject_Hash(key);
        if (hash == -1) {
            return NULL;
        }
    }

    ep = (mp->ma_lookup)(mp, key, hash);
    if (ep == NULL)
        return NULL;
    return ep->me_value;
}
#endif


_LOCAL_ PyObject *
str_from_string(const char *str)
{
#if PY_MAJOR_VERSION >= 3
    return PyUnicode_FromString(str);
#else
    return PyString_FromString(str);
#endif
}


_LOCAL_ PyObject *
str_format(PyObject *fmt, PyObject *args)
{
#if PY_MAJOR_VERSION >= 3
    return PyUnicode_Format(fmt, args);
#else
    return PyString_Format(fmt, args);
#endif
}


_LOCAL_ PyObject *
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


_LOCAL_ void
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


_LOCAL_ void
str_concat_and_del(PyObject **left, PyObject *right)
{
#if PY_MAJOR_VERSION >= 3
    str_concat(left, right);
    Py_XDECREF(right);
#else
    PyString_ConcatAndDel(left, right);
#endif
}


_LOCAL_ PyObject *
str_join(PyObject *separator, PyObject *seq)
{
#if PY_MAJOR_VERSION >= 3
    return PyUnicode_Join(separator, seq);
#else
    return _PyString_Join(separator, seq);
#endif
}
