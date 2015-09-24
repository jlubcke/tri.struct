import pickle
import pytest
from tri.struct import Struct, FrozenStruct


def test_constructor():
    s = Struct(a=1, b=2, c=3)
    assert s == Struct(dict(a=1, b=2, c=3))
    assert s == Struct([('a', 1), ('b', 2), ('c', 3)])
    assert s == Struct((k, v) for k, v in zip('abc', (1, 2, 3)))


def test_get_item():
    s = Struct(a=1, b=2, c=3)
    assert 1 == s['a']
    assert 1 == s.a
    assert 1 == s.get('a')


def test_set_item():
    s = Struct(a=1, b=2, c=3)
    s['a'] = 8
    assert 8 == s['a']
    assert 8 == s.a


def test_isinstance():

    s = Struct()
    assert isinstance(s, dict)

    class FooStruct(Struct):
        pass

    f = FooStruct(x=17)
    assert isinstance(f, Struct)


def test_containment():
    s = Struct(c=3)
    assert 3 not in s
    assert 'c' in s


def test_copy():
    s = Struct(a=1, b=2, c=3)
    q = s.copy()

    assert 'a' in q
    assert 'b' in q
    assert 'c' in q

    q.x = 6
    with pytest.raises(AttributeError) as e:
        # noinspection PyStatementEffect
        s.x

    assert "'Struct' object has no attribute 'x'" in str(e)


def test_to_dict():
    s = Struct(a=1, b=2, c=3)
    assert {'a': 1, 'b': 2, 'c': 3} == dict(s)


def test_ordering():
    s = Struct(a=1)
    t = Struct(b=2)
    assert s < t
    assert not t < s


def test_items():
    s = Struct(a=1, b=2, c=3)
    assert [('a', 1), ('b', 2), ('c', 3)] == sorted(s.items())


def test_no_longer_has_dict():
    s = Struct()

    with pytest.raises(AttributeError) as e:
        # noinspection PyStatementEffect
        s.__dict__

    assert "'Struct' object has no attribute '__dict__'" in str(e)


def test_shadow_methods():
    s = Struct(not_get=17)
    assert "<built-in method get of Struct object at" in str(s.get)

    s = Struct(get=17)
    assert 17 == s.get

    del s.get

    assert "<built-in method get of Struct object at" in str(s.get)


def test_hash():

    s = Struct(x=17)
    with pytest.raises(TypeError) as e:
        hash(s)
    assert "unhashable type: 'Struct'" in str(e)

    f = FrozenStruct(x=17)
    assert int == type(hash(f))


def test_frozen_struct():
    f1 = FrozenStruct(x=17)
    f2 = FrozenStruct(x=17)
    assert f1 == f2
    assert hash(f1) == hash(f2)

    assert f1 in {f1}
    assert f2 in {f1}
    assert f1 not in {FrozenStruct(x=42)}
    assert f1 not in {FrozenStruct(y=17)}

    assert Struct(x=17) == FrozenStruct(x=17)


def test_modify_frozen_struct():
    f = FrozenStruct(x=17)
    with pytest.raises(AttributeError) as e:
        f.x = 42
    assert "'FrozenStruct' object attributes are read-only" in str(e)

    with pytest.raises(AttributeError) as e:
        f.update(dict(x=42))
    assert "'FrozenStruct' object attributes are read-only" in str(e)

    with pytest.raises(AttributeError) as e:
        f.setdefault('foo', 11)
    assert "'FrozenStruct' object attributes are read-only" in str(e)

    with pytest.raises(AttributeError) as e:
        f.clear()
    assert "'FrozenStruct' object attributes are read-only" in str(e)

    with pytest.raises(AttributeError) as e:
        del f.x
    assert "'FrozenStruct' object attributes are read-only" in str(e)


def test_pickle_struct():
    s = Struct(x=17)
    assert s == pickle.loads(pickle.dumps(s, pickle.HIGHEST_PROTOCOL))
    assert type(s) == type(pickle.loads(pickle.dumps(s, pickle.HIGHEST_PROTOCOL)))


def test_pickle_frozen_struct():
    s = FrozenStruct(x=17)
    assert s == pickle.loads(pickle.dumps(s, pickle.HIGHEST_PROTOCOL))
    assert type(s) == type(pickle.loads(pickle.dumps(s, pickle.HIGHEST_PROTOCOL)))


def test_add():
    assert Struct(x=1, y=2) == Struct(x=1) + Struct(y=2)
    assert Struct(x=1, y=2) == Struct(x=1) + FrozenStruct(y=2)
    assert FrozenStruct(x=1, y=2) == FrozenStruct(x=1) + Struct(y=2)


def test_add_with_kwarg_constructor():

    class MyStruct(Struct):
        def __init__(self, **kwargs):
            super(MyStruct, self).__init__(**kwargs)

    s = MyStruct(foo='foo')
    assert MyStruct(foo='foo', bar='bar') == s + dict(bar='bar')


def test_add_to_self():
    s = Struct(x=1)
    s2 = s
    s2 += dict(x=2)
    assert Struct(x=2) == s2
    assert s is s2


def test_add_to_self_frozen_struct():
    s = FrozenStruct(x=1)
    s2 = s
    s2 += dict(x=2)
    assert FrozenStruct(x=2) == s2
    assert s is not s2


def test_or():
    assert Struct(x=1, y=2) == Struct(x=1) | Struct(y=2)
    assert Struct(x=1, y=2) == Struct(x=1) | FrozenStruct(y=2)
    assert FrozenStruct(x=1, y=2) == FrozenStruct(x=1) | Struct(y=2)


def test_del():
    s = Struct(a=1)
    del s.a
    assert s.get('a', 'sentinel') == 'sentinel'
    with pytest.raises(AttributeError) as e:
        del s.a
    assert e.value.message == "'Struct' object has no attribute 'a'"


def test_stable_unicode():
    assert unicode(Struct(b=1, a=2)) == 'Struct(a=2, b=1)'
