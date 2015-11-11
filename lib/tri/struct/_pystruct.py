class Struct(dict):
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
