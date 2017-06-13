Changelog
---------

2.5.4 (2017-06-13)
~~~~~~~~~~~~~~~~~~

* Added `DefaultStruct` in the spirit of the standard library `defaultdict`.
  Also added a `to_default_struct` for recursively converting dicts recursively.


2.5.3 (2017-02-10)
~~~~~~~~~~~~~~~~~~

* Fix use-after-free when repring a `Struct` that contains
  itself more than once.

2.5.2 (2016-04-07)
~~~~~~~~~~~~~~~~~~

* Fix make and tox targets for build and release.
* Fix lint issues.

2.5.1 (2015-12-15)
~~~~~~~~~~~~~~~~~~

* Bugfix: Fix compilation of the _cstruct module.

2.5.0 (unreleased)
~~~~~~~~~~~~~~~~~~

* Build changes.

2.4.0 (2015-12-08)
~~~~~~~~~~~~~~~~~~

* Improvement: If a Struct subclass implements the `__missing__` method,
  it will not be called before GetAttr on attribute access, but will be
  called before GetAttr on dict access.

2.3.1 (2015-12-07)
~~~~~~~~~~~~~~~~~~

* Bugfix: Clear KeyError in CStruct getattr before trying GetAttr,
  otherwise the KeyError may "leak out". Also, only clear the error
  and attempt GetAttr if the original error was a KeyError.

2.3.0 (2015-12-02)
~~~~~~~~~~~~~~~~~~

* Add mixin class `Frozen` to make read-only versions of a dict-derived
  class (typically a Struct or a subclass there of.)

* Use the `Frozen` mixin to implement `FrozenStruct`

2.2.0 (2015-11-12)
~~~~~~~~~~~~~~~~~~

* Add keyword arguments to `merged` function.

2.1.2 (2015-11-11)
~~~~~~~~~~~~~~~~~~

* Redo the C implementation to be a "heaptype", and remove the hack of
  setting `__dict__` = `self`. Instead, `object` will control the type
  storage, letting us "insert" attributes into the object without
  polluting the `dict`.

2.0 (unreleased)
~~~~~~~~~~~~~~~~

* slim down interface to again match dict
* add tri.struct.merged function to join structs
* add optional C implementation to speed up instantiation

1.0 (2015-09-29)
~~~~~~~~~~~~~~~~

* Struct with attribute & dict interface
* __add__ and __or__ to combine structs

