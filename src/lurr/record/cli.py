from pathlib import Path
from argparse import ArgumentParser, Action, ArgumentError
from lurr.trace import runscript
from lurr.recording import Recording, get_record_file
from lurr.record.recorder import Recorder


def _parse_arguments():
    class ScriptAction(Action):
        def __call__(self, parser, namespace, script_arg: Path, option_string=None):
            if not script_arg.is_file():
                raise ArgumentError(None, f"'{script_arg}': no such file")
            setattr(namespace, self.dest, script_arg)

    parser = ArgumentParser(description="run python script recording it's state")
    parser.add_argument(
        "script", action=ScriptAction, type=Path, help="python script to run"
    )

    return parser.parse_args()


def main():
    args = _parse_arguments()
    record_file = get_record_file(args.script)

    with record_file.open("wb") as f:
        runscript(args.script, Recorder(args.script, Recording(f), debug_logging=True))
