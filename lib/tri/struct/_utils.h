#include "Python.h"

void set_attribute_error(PyObject *, PyObject *);
void set_key_error(PyObject *);
PyObject * PyDict_GetItemWithError(PyObject *, PyObject *);
PyObject * str_from_string(const char *);
PyObject * str_format(PyObject *, PyObject *);
PyObject * str_join(PyObject *, PyObject *);
PyObject * format_with_type(PyTypeObject *, PyObject *);
void str_concat(PyObject **, PyObject *);
void str_concat_and_del(PyObject **, PyObject *);
