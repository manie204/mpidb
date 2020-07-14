from config_formatting import ConfigFormatter_Base
from argparse import ArgumentParser, FileType, RawDescriptionHelpFormatter
import textwrap

def _get_all_subclasses(cls):
    return set(cls.__subclasses__()).union(
        [s for c in cls.__subclasses__() for s in _get_all_subclasses(c)])

class Options:
    _max_line_len = 79

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
            formatter_class=RawDescriptionHelpFormatter,
            description=textwrap.fill(
                "Reads info written by mpidb from stdin and generates one of the available "
                "debug configuration formats.", Options._max_line_len),
            epilog=self._generate_formatter_help_string())

        parser.add_argument("format", choices=self._formatters.keys(),
            help="format of the configuration file")
        parser.add_argument("file",
            help="path of the configuration file")
        parser.add_argument("-n", "--name", default="debug",
            help="name of the debug configuration")

        self._args = parser.parse_args()

    def _generate_formatter_help_string(self):
        left_side_len = 24
        right_side_len = Options._max_line_len - left_side_len
        help_string = "Available formatters:\n"

        for f in self._formatters.values():
            cur_help_lines = textwrap.wrap(f.get_option_help(), right_side_len)

            option_name = "  " + f.get_option_name()

            if len(option_name) + 2 < left_side_len:
                help_string += option_name + " " * (left_side_len - len(option_name))
            else:
                help_string += option_name + "\n" + " " * left_side_len
                
            help_string += ("\n" + " " * left_side_len).join(cur_help_lines) + "\n"

        return help_string