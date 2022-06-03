#!/bin/env python3

import argparse
import shutil

parser = argparse.ArgumentParser(description='OpenAstroTech code formatting tool')


def check_environment():
    formatting_tool_path = shutil.which('clang-format')
    if not formatting_tool_path:
        pass
    for tool in required_executables:
        print(shutil.which(tool))


def main():
    pass


if __name__ == '__main__':
    args = parser.parse_args()
    check_environment()
    main()
