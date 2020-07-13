from config_formatting import ConfigFormatter_Base
from argparse import ArgumentParser, FileType

def _get_all_subclasses(cls):
    return set(cls.__subclasses__()).union(
        [s for c in cls.__subclasses__() for s in _get_all_subclasses(c)])

class Options:
    @property
    def formatter(self):
        return self._formatters[self._args.format]

    @property
    def file_name(self):
        return self._args.file

    @property
    def config_name(self):
        return self._args.name

    def __init__(self):
        self._formatters = { cls.get_option_name(): cls for cls in _get_all_subclasses(ConfigFormatter_Base) }

        parser = ArgumentParser(
            description="Reads info written by mpidb from stdin and generates one of the available debug configuration formats.",
            epilog="abc\n\tefg\n\thaha")

        parser.add_argument("format", choices=self._formatters.keys(),
            help="Format of the configuration file.")
        parser.add_argument("file",
            help="Path of the configuration file.")
        parser.add_argument("-n", "--name", default="debug",
            help="Name of the debug configuration.")

        self._args = parser.parse_args()