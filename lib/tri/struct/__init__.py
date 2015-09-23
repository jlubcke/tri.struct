from collections import OrderedDict
from functools import total_ordering
import itertools


__version__ = '0.3.0'


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

    def __ior__(self, other):
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


next_index = itertools.count().next

@total_ordering
class NamedStructField(object):

    def __init__(self, default=None):
        self.default = default
        self.index = next_index()

    def __lt__(self, other):
        return self.index < other.index


def is_named_struct_base_class(bases):
    return len(bases) == 1 and bases[0] is Struct


class NamedStructMeta(type):
    def __new__(mcs, class_name, bases, dct):
        if not is_named_struct_base_class(bases):
            definitions = [(name, value) for name, value in dct.items() if isinstance(value, NamedStructField)]
            definitions.sort(key=lambda t: t[1])

            def generate_inherited_definitions():
                for base in bases:
                    if not issubclass(base, NamedStruct):
                        continue
                    if base is NamedStruct:
                        continue
                    # noinspection PyProtectedMember
                    for definition in base._field_definitions.items():
                        yield definition
            inherited_definitions = list(generate_inherited_definitions())

            for attribute_name in dict(definitions):
                dct.pop(attribute_name)

            field_definitions = OrderedDict(inherited_definitions + definitions)
            dct['_field_definitions'] = field_definitions
            dct['_fields'] = tuple(field_definitions.keys())

        return super(NamedStructMeta, mcs).__new__(mcs, class_name, bases, dct)


class NamedStruct(Struct):

    __metaclass__ = NamedStructMeta

    __slots__ = ('_fields', '_field_definitions')

    def __init__(self, *args, **kwargs):
        fields = object.__getattribute__(self, '_fields')
        field_definitions = object.__getattribute__(self, '_field_definitions')

        if len(args) > len(fields):
            raise ValueError("Too many arguments")
        provided_values_by_field = dict(zip(fields, args))
        for key, value in kwargs.items():
            if key in provided_values_by_field:
                raise ValueError('Field "%s" already given as positional argument' % (key, ))
            provided_values_by_field[key] = value

        for provided_field in provided_values_by_field:
            if provided_field not in fields:
                raise KeyError(provided_field)

        super(NamedStruct, self).__init__(**{key: provided_values_by_field.get(key, field_definitions[key].default) for key in fields})

    def __getitem__(self, key):
        if key not in object.__getattribute__(self, '_fields'):
            raise KeyError(key)
        return super(NamedStruct, self).__getitem__(key)

    def __setitem__(self, key, value):
        if key not in object.__getattribute__(self, '_fields'):
            raise KeyError(key)
        super(NamedStruct, self).__setitem__(key, value)

    def __setattr__(self, k, v):
        try:
            self[k] = v
        except KeyError:
            raise AttributeError("'%s' object has no attribute '%s'" % (self.__class__.__name__, k))


def named_struct(field_names, typename="NamedStruct"):

    if isinstance(field_names, basestring):
        field_names = field_names.replace(',', ' ').split()
    field_names = map(str, field_names)
    typename = str(typename)

    return type(typename, (NamedStruct, ), {field: NamedStructField() for field in field_names})
