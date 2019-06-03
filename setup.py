#!/usr/bin/env python
# -*- coding: utf-8 -*-
import os
import platform
import re

from setuptools import setup, find_packages, Command, Extension
from distutils.command.build_ext import build_ext
from distutils.errors import (
    DistutilsPlatformError,
    CCompilerError,
    DistutilsExecError
)


readme = open('README.rst').read()
history = open('HISTORY.rst').read().replace('.. :changelog:', '')


def read_reqs(name):
    with open(os.path.join(os.path.dirname(__file__), name)) as f:
        return [line for line in f.read().split('\n') if line and not line.strip().startswith('#')]


def read_version():
    with open(os.path.join('lib', 'tri_struct', '__init__.py')) as f:
        m = re.search(r'''__version__\s*=\s*['"]([^'"]*)['"]''', f.read())
        if m:
            return m.group(1)
        raise ValueError("couldn't find version")


class ve_build_ext(build_ext):

    def run(self):
        try:
            build_ext.run(self)
        except DistutilsPlatformError:
            self._error()

    def build_extension(self, ext):
        try:
            build_ext.build_extension(self, ext)
        except (CCompilerError, DistutilsExecError, DistutilsPlatformError):
            self._error()

    def _error(self):
        print('*' * 75)
        print('WARNING: The C extension could not be compiled, '
              'speedups are not enabled.')
        print('Failure information, if any, is above.')
        print('The build will continue without extension modules.')
        print('*' * 75)


if platform.python_implementation() == 'CPython':
    ext_modules = [
        Extension("tri_struct._cstruct", ["lib/tri_struct/_cstruct.c",
                                          "lib/tri_struct/_typespec.c",
                                          "lib/tri_struct/_utils.c"])
    ]
else:
    ext_modules = []


class Tag(Command):
    user_options = []

    def initialize_options(self):
        pass

    def finalize_options(self):
        pass

    def run(self):
        from subprocess import call
        version = read_version()
        errno = call(['git', 'tag', '--annotate', version, '--message', 'Version %s' % version])
        if errno == 0:
            print("Added tag for version %s" % version)
        raise SystemExit(errno)


class ReleaseCheck(Command):
    user_options = []

    def initialize_options(self):
        pass

    def finalize_options(self):
        pass

    def run(self):
        from subprocess import check_output, CalledProcessError
        try:
            tag = check_output(['git', 'describe', 'HEAD']).strip().decode('utf8')
        except CalledProcessError:
            tag = ''
        version = read_version()
        if tag != version:
            print('Missing %s tag on release' % version)
            raise SystemExit(1)

        current_branch = check_output(['git', 'rev-parse', '--abbrev-ref', 'HEAD']).strip().decode('utf8')
        if current_branch != 'master':
            print('Only release from master')
            raise SystemExit(1)

        print("Ok to distribute files")


setup(
    name='tri.struct',
    version=read_version(),
    description='tri.struct supplies classes that can be used like dictionaries and as objects with attribute access at the same time',
    long_description=readme + '\n\n' + history,
    author='Johan Lübcke',
    author_email='johan.lubcke@trioptima.com',
    url='https://github.com/TriOptima/tri.struct',
    ext_modules=ext_modules,
    packages=find_packages('lib'),
    package_dir={'': 'lib'},
    include_package_data=False,
    install_requires=read_reqs('requirements.txt'),
    license="BSD",
    zip_safe=False,
    keywords='tri.struct',
    classifiers=[
        'Development Status :: 5 - Production/Stable',
        'Intended Audience :: Developers',
        'License :: OSI Approved :: BSD License',
        'Natural Language :: English',
        "Programming Language :: Python :: 2",
        'Programming Language :: Python :: 2.7',
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: 3.6',
    ],
    test_suite='tests',
    cmdclass={'build_ext': ve_build_ext,
              'tag': Tag,
              'release_check': ReleaseCheck},
)
