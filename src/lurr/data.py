from pickle import dump
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
