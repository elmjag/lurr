#!/usr/bin/env python
from typing import Optional
from os import fork
from sys import exit
from pathlib import Path
from socket import socketpair, SOCK_DGRAM
from lurr.trace import runscript
from lurr.trace import Tracer
from lurr.recording import Recording, get_record_file
from lurr.debug.zhukov import encode_frame


class Replay(Tracer):
    class ReplayAborted(Exception):
        pass

    class StateInjector:
        def __init__(self, recoder, recording: Recording, func):
            self.recorder = recoder
            self.recording = recording
            self.func = func

        def __call__(self, *args):
            return_value = self.recording.restore_call_state(self.recorder._getframe(1))

            self.recorder.tracing_on = True
            return return_value

    def __init__(
        self, script_file: Path, recording: Recording, ctrl_socket, debug_logging=False
    ):
        super().__init__(script_file, recording, debug_logging)
        self._step_mode = "line"
        self._ctrl_socket = ctrl_socket

    def _read_ctrl_socket(self) -> Optional[str]:
        """
        wait for a 'step command' on the control socket

        returns:
            'line'      to step to next line
            'opcode'    to step to next opcode
            None        to abort replay, i.e. debugger is terminating
        """
        data = self._ctrl_socket.recv(1024)
        if data == b"":
            # socket closed, i.e. we need to abort replay
            return None

        return data.decode()

    def _push_frame_state(self, frame):
        self._ctrl_socket.send(encode_frame(frame))

    def _abort(self, frame, event, arg):
        raise self.ReplayAborted

    def __call__(self, frame, event, arg):
        if not self.tracing_on:
            frame.f_trace_opcodes = True
            return None

        if event == "opcode":
            res = self.get_builtin_call(frame)
            if res:
                func, pos = res
                self.set_stack(
                    frame, pos, self.StateInjector(self, self.recording, func)
                )
                self.tracing_on = False

        if event == self._step_mode:
            self._push_frame_state(frame)
            self._step_mode = self._read_ctrl_socket()
            if self._step_mode is None:
                # abort reply using by raising exception on
                # next non-opcode tracing event
                frame.f_trace_opcodes = False
                return self._abort

        frame.f_trace_opcodes = True
        return self


def _run_tracer(source_file: Path, ctrl_socket):
    script_file = source_file.absolute()
    record_file = get_record_file(script_file)

    with record_file.open("rb") as rec_f:
        runscript(
            script_file, Replay(script_file, Recording(rec_f), ctrl_socket, False)
        )


def start_player(source_file: Path):
    parent, child = socketpair()

    pid = fork()
    if pid:  # parent
        child.close()
        return parent

    # child
    parent.close()

    try:
        _run_tracer(source_file, child)
    except Replay.ReplayAborted:
        pass
    # terminate process here,
    # to avoid returning to caller of start_player()
    exit(0)
