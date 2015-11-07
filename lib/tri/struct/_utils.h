#include "Python.h"

PyObject * str_from_string(const char *);
PyObject * str_format(PyObject *, PyObject *);
PyObject * str_join(PyObject *, PyObject *);
PyObject * format_with_type(PyTypeObject *, PyObject *);
void str_concat(PyObject **, PyObject *);
void str_concat_and_del(PyObject **, PyObject *);
