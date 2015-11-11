class Struct(dict):
    """
    Struct(**kwargs) -> new Struct initialized with the name=value pairs
        in the keyword argument list. For example: Struct(one=1, two=2)
    Struct() -> new empty Struct
    Struct(mapping) -> new Struct initialized from a mapping object's
        (key, value) pairs
    Struct(iterable) -> new Struct initialized as if via:
        s = Struct()
        for k, v in iterable:
            s[k] = v

    >>> bs = Struct(a=1, b=2, c=3)
    >>> bs
    Struct(a=1, b=2, c=3)
    >>> bs.a
    1
    """
    __slots__ = ()

    def __repr__(self):
        pieces = (
            "%s=%s" % (key,
                       (repr(val) if val is not self
                        else "%s(...)" % type(self).__name__)
                       )
            for (key, val) in sorted(self.items())
        )
        return "%s(%s)" % (type(self).__name__,
                           ", ".join(pieces))

    __str__ = __repr__

    def __getattribute__(self, item):
        try:
            return self[item]
        except KeyError:
            return object.__getattribute__(self, item)

    def __setattr__(self, key, value):
        self[key] = value

    def __delattr__(self, item):
        try:
            del self[item]
        except KeyError:
            object.__delattr__(self, item)

    def copy(self):
        return type(self)(self)


Struct.__module__ = "tri.struct"
