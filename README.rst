.. image:: https://travis-ci.org/TriOptima/tri.struct.svg?branch=master
    :target: https://travis-ci.org/TriOptima/tri.struct
.. image:: http://codecov.io/github/TriOptima/tri.struct/coverage.svg?branch=master
    :target: http://codecov.io/github/TriOptima/tri.struct?branch=master

tri.struct
==========

tri.struct supplies classes that can be used like dictionaries and as objects with attribute access at the same time. There are two versions:

- Struct: mutable struct
- FrozenStruct: immutable struct

Some niceties include:

- Predictable repr() so it's easy to write tests
- `merged` function call to merge different types of dicts into a new: `merged(Struct(a=1), FrozenStruct(b=1), c=1) == Struct(a=1, b=1, c=1)`)
- Accelerated implementation in c for improved speed. (With python-only fallback reference implementation)

Example
-------

.. code:: python

    >>> foo = Struct()
    >>> foo.a = 1
    >>> foo['a']
    1
    >>> foo['a'] = 2
    >>> foo.a
    2


Running tests
-------------

You need tox installed then just `make test`.


License
-------

BSD


Documentation
-------------

http://tristruct.readthedocs.org.
