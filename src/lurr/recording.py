from pathlib import Path
from lurr.vm import save_state, restore_state
from lurr.data import write_val, read_val


def get_record_file(script_file: Path) -> Path:
    name = f"{script_file.name}-records"
    return Path(script_file.parent, name)


class Recording:
    def __init__(self, file):
        self._file = file

    def save_call_state(self, frame, return_value):
        save_state(self._file, frame)
        write_val(self._file, return_value)

    def restore_call_state(self, frame):
        restore_state(self._file, frame)

        return_value = read_val(self._file)
        return return_value
