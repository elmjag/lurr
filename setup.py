#!/usr/bin/env python
from setuptools import setup, Extension

vm_module = Extension(
    "lurr.vm",
    sources=["src/lurr/vm/init.c", "src/lurr/vm/state.c", "src/lurr/vm/dump_frame.c"],
)

setup(
    name="lurr",
    version="0.0.0",
    package_dir={"": "src"},
    packages=["lurr"],
    ext_modules=[vm_module],
)
