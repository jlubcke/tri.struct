import pytest

from tri.struct import NamedStruct, named_struct, FrozenStruct, NamedStructField


def test_init():
    assert named_struct('foo')().keys() == ['foo']
    assert sorted(named_struct(['foo', 'bar'])().keys()) == ['bar', 'foo']
    assert sorted(named_struct('foo, bar')().keys()) == ['bar', 'foo']
    assert named_struct('foo, bar')().__class__.__name__ == 'NamedStruct'

    assert sorted(named_struct('foo, bar', 'SomeName')().keys()) == ['bar', 'foo']
    assert named_struct('foo, bar', 'SomeName')().__class__.__name__ == 'SomeName'


MyNamedStruct = named_struct('foo')


def test_access():
    s = MyNamedStruct()
    assert s.foo is None

    s.foo = 17
    assert s.foo == 17
    assert s['foo'] == 17


def test_read_constraints():
    s = MyNamedStruct()
    with pytest.raises(AttributeError):
        # noinspection PyStatementEffect
        s.bar


def test_write_constraints():
    s = MyNamedStruct()
    with pytest.raises(AttributeError):
        # noinspection PyStatementEffect
        s.bar = 17


def test_constructor():
    MyNamedStruct = named_struct('foo, bar')
    s = MyNamedStruct(bar=42, foo=17)
    assert s == dict(foo=17, bar=42)


def test_contstructor_failure():
    MyNamedStruct = named_struct('foo, bar')

    with pytest.raises(ValueError):
        MyNamedStruct(1, 2, 3)  # Too many args

    with pytest.raises(ValueError):
        MyNamedStruct(1, foo=2)  # Conflicting value for foo

    with pytest.raises(KeyError):
        MyNamedStruct(foo=17, bar=42, boink=25)  # Constraint violation


def test_position_arg_constructor():
    MyNamedStruct = named_struct('foo, bar')
    s = MyNamedStruct(17, 42)
    assert s == dict(foo=17, bar=42)


def test_repr():
    assert repr(named_struct("foo, bar")(foo=17, bar=42)) == "NamedStruct(bar=42, foo=17)"
    assert repr(named_struct("foo, bar", "SomeNamedStruct")(foo=17, bar=42)) == "SomeNamedStruct(bar=42, foo=17)"


def test_subclass():

    class MyNamedStruct(named_struct(['foo', 'bar'])):
        pass

    s = MyNamedStruct(foo=17)
    assert repr(s) == 'MyNamedStruct(bar=None, foo=17)'


def test_declarative_style():

    class MyNamedStruct(NamedStruct):
        foo = NamedStructField()
        bar = NamedStructField()

    assert MyNamedStruct(17, 42) == dict(foo=17, bar=42)


def test_declarative_style_inheritance():

    class MyNamedStructBase(NamedStruct):
        foo = NamedStructField()

    class MyNamedStruct(MyNamedStructBase):
        bar = NamedStructField()

    assert MyNamedStruct(17, 42) == dict(foo=17, bar=42)


def test_default_value():

    class MyNamedStruct(NamedStruct):
        foo = NamedStructField()
        bar = NamedStructField()
        baz = NamedStructField(default='default')

    assert MyNamedStruct(17) == dict(foo=17, bar=None, baz='default')


def test_freeze():
    MyNamedStruct = named_struct('foo, bar')
    s = MyNamedStruct(foo=17)
    s.bar = 42
    assert FrozenStruct(s) == FrozenStruct(foo=17, bar=42)


def test_inheritance_with_marker_class():
    class MyType(NamedStruct):
        pass

    class MySubType(MyType):
        foo = NamedStructField()

    x = MySubType(foo=1)

    assert x.foo == 1