from os import path
import re

PROJECT_DIR = path.dirname(path.dirname(__file__))
VERSION_FILE = path.join(PROJECT_DIR, 'pyx', 'version.pxi')

def main():
    with open(VERSION_FILE, "r") as fh:
        print("%s.%s.%s" % re.match(r'__version__\s*=\s*\((\d+),\s*(\d+),\s*(\d+)\)', fh.read()).groups())


if __name__ == '__main__':
    main()