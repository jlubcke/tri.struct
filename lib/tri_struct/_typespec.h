#include "Python.h"

PyObject *type_name(PyTypeObject *);

#if PY_MAJOR_VERSION < 3
#ifndef _TYPESPEC_H
#define _TYPESPEC_H
#include "typeslots.h"


/* Assert a build-time dependency, as an expression.

   Your compile will fail if the condition isn't true, or can't be evaluated
   by the compiler. This can be used in an expression: its value is 0.

   Example:

   #define foo_to_char(foo)  \
       ((char *)(foo)        \
        + Py_BUILD_ASSERT_EXPR(offsetof(struct foo, string) == 0))

   Written by Rusty Russell, public domain, http://ccodearchive.net/ */
#define Py_BUILD_ASSERT_EXPR(cond) \
    (sizeof(char [1 - 2*!(cond)]) - 1)

/* Get the number of elements in a visible array

   This does not work on pointers, or arrays declared as [], or function
   parameters. With correct compiler support, such usage will cause a build
   error (see Py_BUILD_ASSERT_EXPR).

   Written by Rusty Russell, public domain, http://ccodearchive.net/

   Requires at GCC 3.1+ */
#if (defined(__GNUC__) && !defined(__STRICT_ANSI__) && \
    (((__GNUC__ == 3) && (__GNU_MINOR__ >= 1)) || (__GNUC__ >= 4)))
/* Two gcc extensions.
   &a[0] degrades to a pointer: a different type from an array */
#define Py_ARRAY_LENGTH(array) \
    (sizeof(array) / sizeof((array)[0]) \
     + Py_BUILD_ASSERT_EXPR(!__builtin_types_compatible_p(typeof(array), \
                                                          typeof(&(array)[0]))))
#else
#define Py_ARRAY_LENGTH(array) \
    (sizeof(array) / sizeof((array)[0]))
#endif


/*
    from object.h
 */

typedef struct{
    int slot;    /* slot id, see below */
    void *pfunc; /* function pointer */
} PyType_Slot;

typedef struct{
    const char* name;
    int basicsize;
    int itemsize;
    unsigned int flags;
    PyType_Slot *slots; /* terminated by slot==0. */
} PyType_Spec;

PyObject *PyType_FromSpecWithBases(PyType_Spec *, PyObject *);
PyObject *PyType_FromSpec(PyType_Spec *);
#endif //!_TYPESPEC_H
#endif //!PY_MAJOR_VERSION < 3
