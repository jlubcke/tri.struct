__version__ = '2.1.1'
__all__ = ['Struct', 'FrozenStruct', 'merged']


from ._pystruct import Struct
try:
    from ._cstruct import _Struct as Struct
except ImportError:
    pass


class FrozenStruct(Struct):
    __slots__ = ('_hash', )

    def __hash__(self):
        try:
            _hash = self['_hash']
        except KeyError:
            _hash = hash(tuple((k, self[k]) for k in sorted(self.keys())))
            dict.__setattr__(self, '_hash', _hash)
        return _hash

    def __setitem__(self, *_, **__):
        raise TypeError("'%s' object attributes are read-only" % (type(self).__name__, ))

    def __setattr__(self, key, value):
        raise TypeError("'%s' object attributes are read-only" % (type(self).__name__,))

    def setdefault(self, *_, **__):
        raise TypeError("'%s' object attributes are read-only" % (type(self).__name__, ))

    def update(self, *_, **__):
        raise TypeError("'%s' object attributes are read-only" % (type(self).__name__, ))

    def clear(self, *_, **__):
        raise TypeError("'%s' object attributes are read-only" % (type(self).__name__, ))

    def __delitem__(self, *_, **__):
        raise TypeError("'%s' object attributes are read-only" % (type(self).__name__, ))

    def __delattr__(self, *_, **__):
        raise TypeError("'%s' object attributes are read-only" % (type(self).__name__,))

    def __reduce__(self):
        return type(self), (), dict(self)

    def __setstate__(self, state):
        dict.update(self, state)


def merged(*dicts):
    if not dicts:
        return Struct()
    result = dict()
    for d in dicts:
        result.update(d)
    struct_type = type(dicts[0])
    return struct_type(**result)
