#include "Python.h"
#include "_typespec.h"
#include "_utils.h"


typedef PyDictObject StructObject;


static PyObject *
Struct_getattr(PyObject *self, PyObject *name)
{
    PyObject *value;

    value = PyDict_GetItemWithError(self, name);
    if (value) {
        Py_INCREF(value);
    }
    else {
        if (PyErr_Occurred())
            return NULL;

        value = PyObject_GenericGetAttr(self, name);
        if (value == NULL) {
            /* Look up __missing__ method if we're not the direct dict subclass */
            if (!(Py_TYPE(self)->tp_base == &PyDict_Type)) {
                PyObject *missing, *res;
                PyObject *err_type, *err_value, *err_tb;

                PyErr_Fetch(&err_type, &err_value, &err_tb);

#if PY_MAJOR_VERSION >= 3
                _Py_IDENTIFIER(__missing__);
                missing = _PyObject_LookupSpecial(self, &PyId___missing__);
#else
                static PyObject *missing_str = NULL;
                missing = _PyObject_LookupSpecial(self,
                                                  "__missing__",
                                                  &missing_str);
#endif
                if (missing != NULL) {
                    res = PyObject_CallFunctionObjArgs(missing,
                                                       name, NULL);
                    Py_DECREF(missing);
                    return res;
                }
                else if (PyErr_Occurred())
                    return NULL;

                PyErr_Restore(err_type, err_value, err_tb);
            }
        }
    }

    return value;
}


static int
Struct_setattr(PyObject *self, PyObject *name, PyObject *value)
{
    int res = -1;

    if (value == NULL) {
        res = PyDict_DelItem(self, name);
        if (res < 0 && PyErr_ExceptionMatches(PyExc_KeyError))
            set_attribute_error(self, name);
    }
    else
        res = PyDict_SetItem(self, name, value);

    return res;
}


static PyObject *
Struct_copy(PyObject *self)
{
    PyObject *copy;
    PyTypeObject *mytype;

    mytype = self->ob_type;
    copy = mytype->tp_new(mytype, NULL, NULL);

    if (copy == NULL)
        return NULL;
    if (0 == PyDict_Merge(copy, self, 1))
        return copy;
    Py_DECREF(copy);
    return NULL;
}


static PyObject *
Struct_repr(PyObject *self)
{
    Py_ssize_t i;
    PyObject *inner_repr = NULL;
    PyObject *equals = NULL;
    PyObject *pieces = NULL, *result = NULL;
    PyObject *items = NULL, *s;
    PyTypeObject *type = Py_TYPE(self);

    i = Py_ReprEnter(self);
    if (i < 0) {
        return NULL;
    }

    if (i != 0) {
        if (i < 0) return NULL;

        inner_repr = str_from_string("...");
        if (inner_repr == NULL)
            return NULL;

        return format_with_type(type, inner_repr);
    }
    else if (((PyDictObject *)self)->ma_used == 0) {
        result = format_with_type(type, NULL);
    }
    else {
        /* basically `dict_repr` but with keyword notation */
        pieces = PyList_New(0);
        if (pieces == NULL)
            goto done;

        equals = str_from_string("=");
        if (equals == NULL)
            goto done;

        items = PyDict_Items(self);
        if (items == NULL)
            goto done;
        if (PyList_Sort(items) < 0)
            goto done;

        for (i = 0; i < PyList_GET_SIZE(items); i++) {
            PyObject *temp, *key, *value;
            int status;

            temp = PyList_GET_ITEM(items, i);
            key = PyTuple_GetItem(temp, 0);
            if (key == NULL)
                goto done;
            value = PyTuple_GetItem(temp, 1);
            if (value == NULL)
                goto done;

            /* Prevent repr from deleting value during key format. */
            Py_INCREF(value);
            s = PyObject_Str(key);
            str_concat(&s, equals);
            str_concat_and_del(&s, PyObject_Repr(value));
            Py_DECREF(value);
            if (s == NULL)
                goto done;
            status = PyList_Append(pieces, s);
            Py_DECREF(s);  /* append created a new ref */
            if (status < 0)
                goto done;
        }

        /* Paste them all together with ", " between. */
        s = str_from_string(", ");
        if (s == NULL)
            goto done;
        inner_repr = str_join(s, pieces);
        Py_DECREF(s);

        if (inner_repr == NULL)
            goto done;

        result = format_with_type(type, inner_repr);
    }

done:
    Py_XDECREF(inner_repr);
    Py_XDECREF(items);
    Py_XDECREF(pieces);
    Py_XDECREF(equals);
    Py_ReprLeave(self);
    return result;
}


#if PY_MAJOR_VERSION < 3
static int
Struct_print(PyObject *self, FILE *fp, int flags)
{
    PyObject *repr = Py_TYPE(self)->tp_repr(self);
    if (repr == NULL)
        return -1;

    fprintf(fp, "%s", PyString_AS_STRING(repr));
    return 0;
}
#endif


static PyMethodDef Struct_methods[] = {
    {"copy", (PyCFunction)Struct_copy, METH_NOARGS},
    {NULL, NULL},
};


PyDoc_STRVAR(Struct_doc,
"Struct(**kwargs) -> new Struct initialized with the name=value pairs\n"
"    in the keyword argument list. For example: Struct(one=1, two=2)\n"
"Struct() -> new empty Struct\n"
"Struct(mapping) -> new Struct initialized from a mapping object's\n"
"    (key, value) pairs\n"
"Struct(iterable) -> new Struct initialized as if via:\n"
"    s = Struct()\n"
"    for k, v in iterable:\n"
"        s[k] = v\n"
"\n"
">>> bs = Struct(a=1, b=2, c=3)\n"
">>> bs\n"
"Struct(a=1, b=2, c=3)\n"
">>> bs.a\n"
"1\n"
);


static PyType_Slot
StructType_slots[] = {
    {Py_tp_doc, Struct_doc},
    {Py_tp_base, NULL},
    {Py_tp_new, NULL},
    {Py_tp_dealloc, NULL},
    {Py_tp_hash, NULL},
    {Py_tp_traverse, NULL},
    {Py_tp_clear, NULL},
    {Py_tp_richcompare, NULL},
#if PY_MAJOR_VERSION < 3
    {Py_tp_compare, NULL},
    {Py_tp_print, Struct_print},
#endif
    {Py_tp_repr, Struct_repr},
    {Py_tp_str, Struct_repr},
    {Py_tp_getattro, Struct_getattr},
    {Py_tp_setattro, Struct_setattr},
    {Py_tp_methods, Struct_methods},
    {0, NULL}
};


static PyType_Spec
StructType_spec = {
    .name = "tri.struct.Struct",
    .basicsize = sizeof(StructObject),
    .flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_HAVE_GC
        | Py_TPFLAGS_DICT_SUBCLASS | Py_TPFLAGS_HAVE_VERSION_TAG,
    .slots = StructType_slots
};


static int
basestruct_exec(PyObject *m)
{
    PyObject *o;

    /* some members must be initialized at runtime,
       because they're not compile-time constants
     */
    StructType_slots[1].pfunc = &PyDict_Type;
    StructType_slots[2].pfunc = PyDict_Type.tp_new;
    StructType_slots[3].pfunc = PyDict_Type.tp_dealloc;
    StructType_slots[4].pfunc = PyDict_Type.tp_hash;
    StructType_slots[5].pfunc = PyDict_Type.tp_traverse;
    StructType_slots[6].pfunc = PyDict_Type.tp_clear;
    StructType_slots[7].pfunc = PyDict_Type.tp_richcompare;
#if PY_MAJOR_VERSION < 3
    StructType_slots[8].pfunc = PyDict_Type.tp_compare;
#endif

    o = PyType_FromSpec(&StructType_spec);
    if (o == NULL)
        goto fail;
    /* emulate more closely "real" heap types */
    ((PyTypeObject *)o)->tp_name = "Struct";
    PyModule_AddObject(m, "_Struct", o);

    return 0;
fail:
    Py_XDECREF(m);
    return -1;
}


#if PY_MAJOR_VERSION >= 3
static PyModuleDef_Slot basestruct_slots[] = {
    {Py_mod_exec, basestruct_exec},
    {0, NULL},
};


static struct PyModuleDef moduledef = {
    PyModuleDef_HEAD_INIT,
    .m_name = "_cstruct",
    .m_doc = "",
    .m_slots = basestruct_slots,
};


PyMODINIT_FUNC
PyInit__cstruct(void)
{
    return PyModuleDef_Init(&moduledef);
}
#else
void
init_cstruct(void)
{
    PyObject *m = NULL;

    m = Py_InitModule("tri.struct._cstruct", NULL);
    if (m == NULL)
        return;

    if (basestruct_exec(m) < 0)
        Py_DECREF(m);
}
#endif // !PY_MAJOR_VERSION >= 3
