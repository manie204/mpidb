from abc import ABC, abstractmethod
from collections import namedtuple
from copy import deepcopy
import json

class ConfigFormatter_Base(ABC):
    _InfoTuple = namedtuple("InfoTuple", ["rank", "host", "port"])
    # _test_mani remove from base class
    _OPTION_NAME = None
    _OPTION_HELP = None

    def __init__(self, config_name, app_name):
        self._config_name = config_name
        self._app_name = app_name
        self._rank_info = []

    def add_rank_info(self, rank, hostName, port):
        self._rank_info.append(ConfigFormatter_Base._InfoTuple(rank, hostName, port))

    @classmethod
    def get_option_name(cls):
        if not "_OPTION_NAME" in cls.__dict__:
            raise NotImplementedError(f"Class attribute _OPTION_NAME in subclass {cls} not implemented.")
        return cls._OPTION_NAME

    @classmethod
    def get_option_help(cls):
        if not "_OPTION_HELP" in cls.__dict__:
            raise NotImplementedError(f"Class attribute _OPTION_HELP of subclass {cls} not implemented.")
        return cls._OPTION_HELP

    @abstractmethod
    def write_config(self, file):
        pass


class ConfigFormatter_PlainText(ConfigFormatter_Base):
    _OPTION_NAME = "txt"
    _OPTION_HELP = "Simple human-readable plain-text format."

    def write_config(self, file):
        file.write(f"{self._config_name}\n{self._app_name}\n")

        width_rank = max(map(len, ("Rank", *(str(info.rank) for info in self._rank_info)))) + 2
        width_host = max(map(len, ("Hostname", *(str(info.host) for info in self._rank_info)))) + 2
        width_port = max(map(len, ("Port", *(str(info.port) for info in self._rank_info)))) + 2

        file.write(
            f"{'Rank':<{width_rank}}"
            f"{'Hostname':<{width_host}}"
            f"{'Port':<{width_port}}"
            "\n")

        for info in self._rank_info:
            file.write(
                f"{info.rank:<{width_rank}}"
                f"{info.host:<{width_host}}"
                f"{info.port:<{width_port}}"
                "\n")

class ConfigFormatter_VSCode(ConfigFormatter_Base):
    _OPTION_NAME = "vscode"
    _OPTION_HELP = (
        "Creates launch configuration file for Visual Studio Code. Usually, named 'launch.json' "
        "and placed in the '.vscode' directory in your workspace folder. Allows for spawning "
        "of multiple concurrent debugging sessions via compound configurations. The compound "
        "configuration has the suffix 'all ranks'.")

    _config_template = {
        "name": None,
        "program": None,
        "miDebuggerServerAddress": None,
        "type": "cppdbg",
        "request": "launch",
        "args": [],
        "stopAtEntry": False,
        "cwd": "${workspaceRoot}",
        "environment": [],
        "externalConsole": True,
        "MIMode": "gdb",
        "setupCommands": [
            {
                "description": "Enable pretty-printing for gdb",
                "text": "-enable-pretty-printing",
                "ignoreFailures": True
            }
        ]
    }

    def write_config(self, file):
        config_base = {
            "version": "0.2.0",
            "compounds": [
                {
                    "name": self._config_name + " all ranks",
                    "configurations": []
                }
            ],
            "configurations": []
        }

        compound_config_names = config_base["compounds"][0]["configurations"]

        for info in self._rank_info:
            cur_config = deepcopy(ConfigFormatter_VSCode._config_template)

            config_name = self._config_name + " rank " + str(info.rank)
            cur_config["name"] = config_name
            cur_config["program"] = self._app_name
            cur_config["miDebuggerServerAddress"] = info.host + ":" + str(info.port)

            config_base["configurations"].append(cur_config)
            compound_config_names.append(config_name)

        json.dump(config_base, file, indent=4)
