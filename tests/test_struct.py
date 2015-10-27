import pickle
import platform

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


def test_items():
    s = Struct(a=1, b=2, c=3)
    assert [('a', 1), ('b', 2), ('c', 3)] == sorted(s.items())


def test_shadow_methods():
    if platform.python_implementation() == "PyPy":
        method_str = "<bound method Struct.get of Struct"
    else:
        method_str = "<built-in method get of Struct object at"

    s = Struct(not_get=17)
    assert method_str in str(s.get)

    s = Struct(get=17)
    assert 17 == s.get

    del s.get

    assert method_str in str(s.get)


def test_hash():

    s = Struct(x=17)
    with pytest.raises(TypeError) as e:
        hash(s)
    if platform.python_implementation() == "PyPy":
        assert "" in str(e)
    else:
        assert "unhashable type: 'Struct'" in str(e)

    f = FrozenStruct(x=17)
    assert isinstance(hash(f), int)


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

    with pytest.raises(KeyError) as e:
        f.update(dict(x=42))
    assert "'FrozenStruct' object attributes are read-only" in str(e)

    with pytest.raises(KeyError) as e:
        f.setdefault('foo', 11)
    assert "'FrozenStruct' object attributes are read-only" in str(e)

    with pytest.raises(KeyError) as e:
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


def test_del():
    s = Struct(a=1)
    del s.a
    assert s.get('a', 'sentinel') == 'sentinel'
    with pytest.raises(AttributeError) as e:
        del s.a
    assert str(e.value) == "'Struct' object has no attribute 'a'"


def test_stable_str():
    assert str(Struct(b=1, a=2)) == 'Struct(a=2, b=1)'
