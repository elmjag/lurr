import sys
from pickle import dump, load
from types import ModuleType
from dataclasses import dataclass

PICKLE_PROTOCOL = 0


@dataclass
class ModuleWrapper:
    name: str  # '__name__'


def _wrap_dict(value):
    return {_wrap(k): _wrap(v) for k, v in value.items()}


def _wrap_tuple(value: tuple):
    return tuple(_wrap(v) for v in value)


def _wrap(value):
    if isinstance(value, dict):
        return _wrap_dict(value)
    elif isinstance(value, ModuleType):
        return ModuleWrapper(value.__name__)
    elif isinstance(value, tuple):
        return _wrap_tuple(value)

    return value


def write_val(file, val):
    dump(_wrap(val), file, PICKLE_PROTOCOL)


def _unwrap_dict(value):
    return {_unwrap(k): _unwrap(v) for k, v in value.items()}


def _unwrap_tuple(value: tuple):
    return tuple(_unwrap(v) for v in value)


def _unwrap_module(value: ModuleWrapper):
    # we assume the module have been loaded previously
    assert value.name in sys.modules
    return sys.modules[value.name]


def _unwrap(value):
    if isinstance(value, dict):
        return _unwrap_dict(value)
    elif isinstance(value, ModuleWrapper):
        return _unwrap_module(value)
    elif isinstance(value, tuple):
        return _unwrap_tuple(value)

    return value


def read_val(file):
    return _unwrap(load(file))
