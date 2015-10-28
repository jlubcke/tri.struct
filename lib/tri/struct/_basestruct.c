#include "Python.h"
#include "structmember.h"

#define DEFERRED_ADDRESS(ADDR) 0


static PyTypeObject BaseStructType;


typedef struct {
    PyDictObject dict;
    PyDictObject *__dict__;
} BaseStructObject;


static PyObject *
BaseStruct_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    BaseStructObject *self = (BaseStructObject *)PyDict_Type.tp_new(type, args, kwds);
    if (self != NULL) {
        self->__dict__ = (PyDictObject *)self;
    }
    return (PyObject *)self;
}

static PyObject *
BaseStruct_get_dict(BaseStructObject *self)
{
    Py_INCREF(self->__dict__);
    return (PyObject *)self->__dict__;
}

static PyGetSetDef BaseStructType_getset[] = {
    {"__dict__", (getter)BaseStruct_get_dict, (setter)NULL},
    {NULL},
};

static PyMethodDef BaseStruct_methods[] = {
    {NULL, NULL}
};

PyDoc_STRVAR(BaseStruct_doc,
">>> BaseStruct(a=1, b=2, c=3)\n"
"{'a': 1, 'c': 3, 'b': 2}\n"
"\n"
">>> bs = BaseStruct([('a', 1), ('b', 2), ('c', 3)])\n"
">>> bs.a\n"
"1\n"
);

static PyTypeObject BaseStructType = {
    PyVarObject_HEAD_INIT(DEFERRED_ADDRESS(&PyType_Type), 0)
    "tri.struct.BaseStruct",
    sizeof(BaseStructObject),

    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_HAVE_VERSION_TAG,
    .tp_base = DEFERRED_ADDRESS(&PyDict_Type),
    .tp_new = BaseStruct_new,

    .tp_dictoffset = offsetof(BaseStructObject, __dict__),
    .tp_getset = BaseStructType_getset,

    .tp_doc = BaseStruct_doc,
    .tp_methods = BaseStruct_methods,
};

#if PY_MAJOR_VERSION >= 3
static struct PyModuleDef moduledef = {
    PyModuleDef_HEAD_INIT,
    .m_name = "_basestruct",
    .m_doc = "",
    .m_size = -1,
};
#endif

static PyObject *
moduleinit(void)
{
    PyObject *m;

    /* some members must be initialized at runtime,
       because they're not compile-time constants
     */
    BaseStructType.tp_base = &PyDict_Type;
    BaseStructType.tp_hash = PyDict_Type.tp_hash;
    BaseStructType.tp_richcompare = PyDict_Type.tp_richcompare;
#if PY_MAJOR_VERSION >= 3
#else
    BaseStructType.tp_compare = PyDict_Type.tp_compare;
#endif

    if (PyType_Ready(&BaseStructType) < 0)
      return NULL;

#if PY_MAJOR_VERSION >= 3
    m = PyModule_Create(&moduledef);
#else
    m = Py_InitModule("tri.struct._basestruct", NULL);
#endif
    if (m == NULL)
        return NULL;

    Py_INCREF(&BaseStructType);
    if (PyModule_AddObject(m, "BaseStruct",
                           (PyObject *)&BaseStructType) < 0)
        return NULL;

    return m;
}


#if PY_MAJOR_VERSION >= 3
PyMODINIT_FUNC
PyInit__basestruct(void)
{
    return moduleinit();
}
#else
void
init_basestruct(void)
{
    moduleinit();
}
#endif
