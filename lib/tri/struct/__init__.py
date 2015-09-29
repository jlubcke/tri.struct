
__version__ = '1.0.0'


class Struct(dict):
    """
    A dict where keys can also be accessed as attributes.
    """

    __slots__ = ()

    def __getattribute__(self, k):
        try:
            return self[k]
        except KeyError:
            try:
                return object.__getattribute__(self, k)
            except AttributeError:
                raise AttributeError("'%s' object has no attribute '%s'" % (self.__class__.__name__, k))

    def __setattr__(self, k, v):
        self[k] = v

    def __delattr__(self, k):
        try:
            del self[k]
        except KeyError:
            raise AttributeError("'%s' object has no attribute '%s'" % (self.__class__.__name__, k))

    def copy(self):
        return type(self)(**self)

    def __unicode__(self):
        return u'%s(%s)' % (self.__class__.__name__, u', '.join([u'%s=%r' % (key, self[key]) for key in sorted(self.keys())]))

    def __repr__(self):
        return '%s(%s)' % (self.__class__.__name__, ', '.join(['%s=%r' % (key, self[key]) for key in sorted(self.keys())]))

    def __add__(self, other):
        result = Struct()
        result.update(self)
        result.update(other)
        return type(self)(**result)

    def __iadd__(self, other):
        self.update(other)
        return self

    def __or__(self, other):
        return self.__add__(other)

    def __ior__(self, other):  # pragma: no cover
        return self.__iadd__(other)


class FrozenStruct(Struct):

    __slots__ = ('_hash', )

    def __hash__(self):
        try:
            _hash = object.__getattribute__(self, '_hash')
        except AttributeError:
            _hash = hash(tuple((k, self[k]) for k in sorted(self.keys())))
            object.__setattr__(self, '_hash', _hash)
        return _hash

    def __setitem__(self, *_, **__):
        raise AttributeError("'%s' object attributes are read-only" % (self.__class__.__name__, ))

    def setdefault(self, *_, **__):
        raise AttributeError("'%s' object attributes are read-only" % (self.__class__.__name__, ))

    def update(self, *_, **__):
        raise AttributeError("'%s' object attributes are read-only" % (self.__class__.__name__, ))

    def clear(self, *_, **__):
        raise AttributeError("'%s' object attributes are read-only" % (self.__class__.__name__, ))

    def __delitem__(self, *_, **__):
        raise AttributeError("'%s' object attributes are read-only" % (self.__class__.__name__, ))

    def __iadd__(self, other):
        return self + other

    def __reduce__(self):
        return type(self), (), dict(self)

    def __setstate__(self, state):
        dict.update(self, state)
