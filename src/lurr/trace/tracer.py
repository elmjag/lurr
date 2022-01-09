from pathlib import Path
from lurr.recording import Recording


class Tracer:
    # do import at class scope, as the module
    # scope is not available during tracing
    from sys import stderr, _getframe
    from lurr.vm import peek_stack, set_stack
    from dis import opname
    from types import (
        FunctionType,
        BuiltinFunctionType,
        BuiltinMethodType,
        MethodDescriptorType,
    )

    #
    # opcode numbers, for quicker lookup
    #
    import dis

    CALL_FUNCTION = dis.opmap["CALL_FUNCTION"]
    CALL_METHOD = dis.opmap["CALL_METHOD"]

    def __init__(
        self,
        script_file: Path,
        recording: Recording,
        debug_logging=False,
    ):
        self.script_file = str(script_file)
        self.recording = recording
        self.tracing_on = True

        if debug_logging:
            self._print = self._do_print

    def _do_print(self, *args, **kwargs):
        print(*args, **kwargs, file=self.stderr)

    def _print(self, *arg, **kwargs):
        # swallow all self._print() calls by default
        pass

    def get_builtin_call(self, frame):
        code_obj = frame.f_code
        opcode = code_obj.co_code[frame.f_lasti]
        if opcode != self.CALL_METHOD:
            return None

        opcode_arg = code_obj.co_code[frame.f_lasti + 1]
        pos = opcode_arg + 2
        f = self.peek_stack(frame, pos)
        if f is None:
            pos = opcode_arg + 1
            f = self.peek_stack(frame, pos)

        self._print(f"call method {opcode_arg=} {f=} {type(f)=}")

        if not (
            isinstance(f, self.MethodDescriptorType)
            or isinstance(f, self.BuiltinMethodType)
        ):
            return None

        return f, pos
