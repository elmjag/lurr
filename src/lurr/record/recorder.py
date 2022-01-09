from pathlib import Path
from lurr.trace import Tracer
from lurr.recording import Recording


class Recorder(Tracer):
    class StateIntercept:
        def __init__(
            self,
            recoder,
            recording: Recording,
            func,
        ):
            self.recorder = recoder
            self.recording = recording
            self.func = func

        def __call__(self, *args):
            return_value = self.func(*args)

            self.recording.save_call_state(self.recorder._getframe(1), return_value)

            self.recorder.tracing_on = True
            return return_value

    def __init__(self, script_file: Path, recording: Recording, debug_logging=False):
        super().__init__(
            script_file,
            recording,
            debug_logging,
        )
        self.last_shown_frame = None

    def _show_frame(self, frame):
        if self.last_shown_frame is frame:
            # already shown
            return

        self.last_shown_frame = frame
        self.dis.dis(frame.f_code, file=self.stderr)

    def _print_trace_call(self, frame, event, arg):
        self._print(f"{event}:", end="")

        match event:
            case "call":
                self._print(f"           frame: {id(frame):x}")
                self._show_frame(frame)
                self._print()
            case "line":
                self._print(f"           {frame.f_code.co_filename}:{frame.f_lineno}")
                self._print()
            case "opcode":
                code_obj = frame.f_code
                opcode = code_obj.co_code[frame.f_lasti]
                opcode_arg = code_obj.co_code[frame.f_lasti + 1]

                self._print(f"{frame.f_lasti:8} {self.opname[opcode]:25} {opcode_arg}")
            case "return":
                self._print(f"         {arg}")

    def __call__(self, frame, event, arg):
        if not self.tracing_on:
            frame.f_trace_opcodes = True
            return None

        self._print_trace_call(frame, event, arg)

        if event == "opcode":
            res = self.get_builtin_call(frame)
            if res:
                func, pos = res
                self.set_stack(
                    frame,
                    pos,
                    self.StateIntercept(
                        self,
                        self.recording,
                        func,
                    ),
                )
                self.tracing_on = False

        frame.f_trace_opcodes = True

        return self
