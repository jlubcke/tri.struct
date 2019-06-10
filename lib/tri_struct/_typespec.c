#include "Python.h"
#include "structmember.h"


/*
    this functionality is built-in in Python 3.2+
*/
#if PY_MAJOR_VERSION < 3
#include "_typespec.h"

#define _LOCAL_ __attribute__((visibility("hidden")))

/*
    from typeobject.c
*/

static void
clear_slots(PyTypeObject *type, PyObject *self)
{
    Py_ssize_t i, n;
    PyMemberDef *mp;

    n = Py_SIZE(type);
    mp = PyHeapType_GET_MEMBERS((PyHeapTypeObject *)type);
    for (i = 0; i < n; i++, mp++) {
        if (mp->type == T_OBJECT_EX && !(mp->flags & READONLY)) {
            char *addr = (char *)self + mp->offset;
            PyObject *obj = *(PyObject **)addr;
            if (obj != NULL) {
                *(PyObject **)addr = NULL;
                Py_DECREF(obj);
            }
        }
    }
}

static void
subtype_dealloc(PyObject *self)
{
    PyTypeObject *type, *base;
    destructor basedealloc;
    PyThreadState *tstate = PyThreadState_GET();
    int has_finalizer;

    /* Extract the type; we expect it to be a heap type */
    type = Py_TYPE(self);
    assert(type->tp_flags & Py_TPFLAGS_HEAPTYPE);

    /* Test whether the type has GC exactly once */

    if (!PyType_IS_GC(type)) {
        /* It's really rare to find a dynamic type that doesn't have
           GC; it can only happen when deriving from 'object' and not
           adding any slots or instance variables.  This allows
           certain simplifications: there's no need to call
           clear_slots(), or DECREF the dict, or clear weakrefs. */

        /* Maybe call finalizer; exit early if resurrected */
        if (type->tp_del) {
            type->tp_del(self);
            if (self->ob_refcnt > 0)
                return;
        }

        /* Find the nearest base with a different tp_dealloc */
        base = type;
        while ((basedealloc = base->tp_dealloc) == subtype_dealloc) {
            assert(Py_SIZE(base) == 0);
            base = base->tp_base;
            assert(base);
        }

        /* Extract the type again; tp_del may have changed it */
        type = Py_TYPE(self);

        /* Call the base tp_dealloc() */
        assert(basedealloc);
        basedealloc(self);

        /* Can't reference self beyond this point */
        Py_DECREF(type);

        /* Done */
        return;
    }

    /* We get here only if the type has GC */

    /* UnTrack and re-Track around the trashcan macro, alas */
    /* See explanation at end of function for full disclosure */
    PyObject_GC_UnTrack(self);
    ++_PyTrash_delete_nesting;
    ++ tstate->trash_delete_nesting;
    Py_TRASHCAN_SAFE_BEGIN(self);
    --_PyTrash_delete_nesting;
    -- tstate->trash_delete_nesting;
    /* DO NOT restore GC tracking at this point.  weakref callbacks
     * (if any, and whether directly here or indirectly in something we
     * call) may trigger GC, and if self is tracked at that point, it
     * will look like trash to GC and GC will try to delete self again.
     */

    /* Find the nearest base with a different tp_dealloc */
    base = type;
    while ((/*basedealloc =*/ base->tp_dealloc) == subtype_dealloc) {
        base = base->tp_base;
        assert(base);
    }

    has_finalizer = type->tp_del != NULL;

    /* Maybe call finalizer; exit early if resurrected */
    if (has_finalizer)
        _PyObject_GC_TRACK(self);

    /* If we added a weaklist, we clear it.      Do this *before* calling
       tp_del, clearing slots, or clearing the instance dict. */
    if (type->tp_weaklistoffset && !base->tp_weaklistoffset)
        PyObject_ClearWeakRefs(self);

    if (type->tp_del) {
        type->tp_del(self);
        if (self->ob_refcnt > 0) {
            /* Resurrected */
            goto endlabel;
        }
    }
    if (has_finalizer) {
        _PyObject_GC_UNTRACK(self);
        /* New weakrefs could be created during the finalizer call.
           If this occurs, clear them out without calling their
           finalizers since they might rely on part of the object
           being finalized that has already been destroyed. */
        if (type->tp_weaklistoffset && !base->tp_weaklistoffset) {
            /* Modeled after GET_WEAKREFS_LISTPTR() */
            PyWeakReference **list = (PyWeakReference **) \
                PyObject_GET_WEAKREFS_LISTPTR(self);
            while (*list)
                _PyWeakref_ClearRef(*list);
        }
    }

    /*  Clear slots up to the nearest base with a different tp_dealloc */
    base = type;
    while ((basedealloc = base->tp_dealloc) == subtype_dealloc) {
        if (Py_SIZE(base))
            clear_slots(base, self);
        base = base->tp_base;
        assert(base);
    }

    /* If we added a dict, DECREF it */
    if (type->tp_dictoffset && !base->tp_dictoffset) {
        PyObject **dictptr = _PyObject_GetDictPtr(self);
        if (dictptr != NULL) {
            PyObject *dict = *dictptr;
            if (dict != NULL) {
                Py_DECREF(dict);
                *dictptr = NULL;
            }
        }
    }

    /* Extract the type again; tp_del may have changed it */
    type = Py_TYPE(self);

    /* Call the base tp_dealloc(); first retrack self if
     * basedealloc knows about gc.
     */
    if (PyType_IS_GC(base))
        _PyObject_GC_TRACK(self);
    assert(basedealloc);
    basedealloc(self);

    /* Can't reference self beyond this point. It's possible tp_del switched
       our type from a HEAPTYPE to a non-HEAPTYPE, so be careful about
       reference counting. */
    if (type->tp_flags & Py_TPFLAGS_HEAPTYPE)
      Py_DECREF(type);

  endlabel:
    ++_PyTrash_delete_nesting;
    ++ tstate->trash_delete_nesting;
    Py_TRASHCAN_SAFE_END(self);
    --_PyTrash_delete_nesting;
    -- tstate->trash_delete_nesting;

    /* Explanation of the weirdness around the trashcan macros:

       Q. What do the trashcan macros do?

       A. Read the comment titled "Trashcan mechanism" in object.h.
          For one, this explains why there must be a call to GC-untrack
          before the trashcan begin macro.      Without understanding the
          trashcan code, the answers to the following questions don't make
          sense.

       Q. Why do we GC-untrack before the trashcan and then immediately
          GC-track again afterward?

       A. In the case that the base class is GC-aware, the base class
          probably GC-untracks the object.      If it does that using the
          UNTRACK macro, this will crash when the object is already
          untracked.  Because we don't know what the base class does, the
          only safe thing is to make sure the object is tracked when we
          call the base class dealloc.  But...  The trashcan begin macro
          requires that the object is *untracked* before it is called.  So
          the dance becomes:

         GC untrack
         trashcan begin
         GC track

       Q. Why did the last question say "immediately GC-track again"?
          It's nowhere near immediately.

       A. Because the code *used* to re-track immediately.      Bad Idea.
          self has a refcount of 0, and if gc ever gets its hands on it
          (which can happen if any weakref callback gets invoked), it
          looks like trash to gc too, and gc also tries to delete self
          then.  But we're already deleting self.  Double deallocation is
          a subtle disaster.

       Q. Why the bizarre (net-zero) manipulation of
          _PyTrash_delete_nesting around the trashcan macros?

       A. Some base classes (e.g. list) also use the trashcan mechanism.
          The following scenario used to be possible:

          - suppose the trashcan level is one below the trashcan limit

          - subtype_dealloc() is called

          - the trashcan limit is not yet reached, so the trashcan level
        is incremented and the code between trashcan begin and end is
        executed

          - this destroys much of the object's contents, including its
        slots and __dict__

          - basedealloc() is called; this is really list_dealloc(), or
        some other type which also uses the trashcan macros

          - the trashcan limit is now reached, so the object is put on the
        trashcan's to-be-deleted-later list

          - basedealloc() returns

          - subtype_dealloc() decrefs the object's type

          - subtype_dealloc() returns

          - later, the trashcan code starts deleting the objects from its
        to-be-deleted-later list

          - subtype_dealloc() is called *AGAIN* for the same object

          - at the very least (if the destroyed slots and __dict__ don't
        cause problems) the object's type gets decref'ed a second
        time, which is *BAD*!!!

          The remedy is to make sure that if the code between trashcan
          begin and end in subtype_dealloc() is called, the code between
          trashcan begin and end in basedealloc() will also be called.
          This is done by decrementing the level after passing into the
          trashcan block, and incrementing it just before leaving the
          block.

          But now it's possible that a chain of objects consisting solely
          of objects whose deallocator is subtype_dealloc() will defeat
          the trashcan mechanism completely: the decremented level means
          that the effective level never reaches the limit.      Therefore, we
          *increment* the level *before* entering the trashcan block, and
          matchingly decrement it after leaving.  This means the trashcan
          code will trigger a little early, but that's no big deal.

       Q. Are there any live examples of code in need of all this
          complexity?

       A. Yes.  See SF bug 668433 for code that crashed (when Python was
          compiled in debug mode) before the trashcan level manipulations
          were added.  For more discussion, see SF patches 581742, 575073
          and bug 574207.
    */
}


static PyTypeObject *solid_base(PyTypeObject *type);

/* Calculate the best base amongst multiple base classes.
   This is the first one that's on the path to the "solid base". */

static PyTypeObject *
best_base(PyObject *bases)
{
    Py_ssize_t i, n;
    PyTypeObject *base, *winner, *candidate, *base_i;
    PyObject *base_proto;

    assert(PyTuple_Check(bases));
    n = PyTuple_GET_SIZE(bases);
    assert(n > 0);
    base = NULL;
    winner = NULL;
    for (i = 0; i < n; i++) {
        base_proto = PyTuple_GET_ITEM(bases, i);
        if (!PyType_Check(base_proto)) {
            PyErr_SetString(
                PyExc_TypeError,
                "bases must be types");
            return NULL;
        }
        base_i = (PyTypeObject *)base_proto;
        if (base_i->tp_dict == NULL) {
            if (PyType_Ready(base_i) < 0)
                return NULL;
        }
        if (!PyType_HasFeature(base_i, Py_TPFLAGS_BASETYPE)) {
            PyErr_Format(PyExc_TypeError,
                         "type '%.100s' is not an acceptable base type",
                         base_i->tp_name);
            return NULL;
        }
        candidate = solid_base(base_i);
        if (winner == NULL) {
            winner = candidate;
            base = base_i;
        }
        else if (PyType_IsSubtype(winner, candidate))
            ;
        else if (PyType_IsSubtype(candidate, winner)) {
            winner = candidate;
            base = base_i;
        }
        else {
            PyErr_SetString(
                PyExc_TypeError,
                "multiple bases have "
                "instance lay-out conflict");
            return NULL;
        }
    }
    assert (base != NULL);

    return base;
}

static int
extra_ivars(PyTypeObject *type, PyTypeObject *base)
{
    size_t t_size = type->tp_basicsize;
    size_t b_size = base->tp_basicsize;

    assert(t_size >= b_size); /* Else type smaller than base! */
    if (type->tp_itemsize || base->tp_itemsize) {
        /* If itemsize is involved, stricter rules */
        return t_size != b_size ||
            type->tp_itemsize != base->tp_itemsize;
    }
    if (type->tp_weaklistoffset && base->tp_weaklistoffset == 0 &&
        type->tp_weaklistoffset + sizeof(PyObject *) == t_size &&
        type->tp_flags & Py_TPFLAGS_HEAPTYPE)
        t_size -= sizeof(PyObject *);
    if (type->tp_dictoffset && base->tp_dictoffset == 0 &&
        type->tp_dictoffset + sizeof(PyObject *) == t_size &&
        type->tp_flags & Py_TPFLAGS_HEAPTYPE)
        t_size -= sizeof(PyObject *);

    return t_size != b_size;
}

static PyTypeObject *
solid_base(PyTypeObject *type)
{
    PyTypeObject *base;

    if (type->tp_base)
        base = solid_base(type->tp_base);
    else
        base = &PyBaseObject_Type;
    if (extra_ivars(type, base))
        return type;
    else
        return base;
}


static short slotoffsets[] = {
    -1, /* invalid slot */
#include "typeslots.inc"
};

_LOCAL_ PyObject *
PyType_FromSpecWithBases(PyType_Spec *spec, PyObject *bases)
{
    PyHeapTypeObject *res = (PyHeapTypeObject*)PyType_GenericAlloc(&PyType_Type, 0);
    PyTypeObject *type, *base;
    PyObject *modname;
    char *s;
    char *res_start = (char*)res;
    PyType_Slot *slot;

    /* Set the type name and qualname */
    s = strrchr(spec->name, '.');
    if (s == NULL)
        s = (char*)spec->name;
    else
        s++;

    if (res == NULL)
        return NULL;
    type = &res->ht_type;
    /* The flags must be initialized early, before the GC traverses us */
    type->tp_flags = spec->flags | Py_TPFLAGS_HEAPTYPE;
    res->ht_name = PyString_FromString(s);
    if (!res->ht_name)
        goto fail;
    type->tp_name = spec->name;
    if (!type->tp_name)
        goto fail;

    /* Adjust for empty tuple bases */
    if (!bases) {
        base = &PyBaseObject_Type;
        /* See whether Py_tp_base(s) was specified */
        for (slot = spec->slots; slot->slot; slot++) {
            if (slot->slot == Py_tp_base)
                base = slot->pfunc;
            else if (slot->slot == Py_tp_bases) {
                bases = slot->pfunc;
                Py_INCREF(bases);
            }
        }
        if (!bases)
            bases = PyTuple_Pack(1, base);
        if (!bases)
            goto fail;
    }
    else
        Py_INCREF(bases);

    /* Calculate best base, and check that all bases are type objects */
    base = best_base(bases);
    if (base == NULL) {
        goto fail;
    }
    if (!PyType_HasFeature(base, Py_TPFLAGS_BASETYPE)) {
        PyErr_Format(PyExc_TypeError,
                     "type '%.100s' is not an acceptable base type",
                     base->tp_name);
        goto fail;
    }

    /* Initialize essential fields */
    type->tp_as_number = &res->as_number;
    type->tp_as_sequence = &res->as_sequence;
    type->tp_as_mapping = &res->as_mapping;
    type->tp_as_buffer = &res->as_buffer;
    /* Set tp_base and tp_bases */
    type->tp_bases = bases;
    bases = NULL;
    Py_INCREF(base);
    type->tp_base = base;

    type->tp_basicsize = spec->basicsize;
    type->tp_itemsize = spec->itemsize;

    for (slot = spec->slots; slot->slot; slot++) {
        if (slot->slot < 0
            || (size_t)slot->slot >= Py_ARRAY_LENGTH(slotoffsets)) {
            PyErr_SetString(PyExc_RuntimeError, "invalid slot offset");
            goto fail;
        }
        if (slot->slot == Py_tp_base || slot->slot == Py_tp_bases)
            /* Processed above */
            continue;
        *(void**)(res_start + slotoffsets[slot->slot]) = slot->pfunc;

        /* need to make a copy of the docstring slot, which usually
           points to a static string literal */
        if (slot->slot == Py_tp_doc) {
            const char *old_doc = slot->pfunc;
            size_t len = strlen(old_doc)+1;
            char *tp_doc = PyObject_MALLOC(len);
            if (tp_doc == NULL) {
                PyErr_NoMemory();
                goto fail;
            }
            memcpy(tp_doc, old_doc, len);
            type->tp_doc = tp_doc;
        }
    }
    if (type->tp_dealloc == NULL) {
        /* It's a heap type, so needs the heap types' dealloc.
           subtype_dealloc will call the base type's tp_dealloc, if
           necessary. */
        type->tp_dealloc = subtype_dealloc;
    }

    if (PyType_Ready(type) < 0)
        goto fail;

    /* Set type.__module__ */
    s = strrchr(spec->name, '.');
    if (s != NULL) {
        modname = PyString_FromStringAndSize(
                spec->name, (Py_ssize_t)(s - spec->name));
        if (modname == NULL) {
            goto fail;
        }
        PyDict_SetItemString(type->tp_dict, "__module__", modname);
        Py_DECREF(modname);
    }

    return (PyObject*)res;

 fail:
    Py_DECREF(res);
    return NULL;
}

_LOCAL_ PyObject *
PyType_FromSpec(PyType_Spec *spec)
{
    return PyType_FromSpecWithBases(spec, NULL);
}
#endif //!PY_MAJOR_VERSION < 3
