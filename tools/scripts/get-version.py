import pathlib
from os import path
import re

THIS_FILE = pathlib.Path(__file__).resolve()

import sys
print(">>>", THIS_FILE, file=sys.stderr)

PROJECT_DIR = THIS_FILE.parent.parent.parent
VERSION_FILE = PROJECT_DIR / "pyx" / "version.pxi"

def main():
    with VERSION_FILE.open() as fh:
        print("%s.%s.%s" % re.match(r'__version__\s*=\s*\((\d+),\s*(\d+),\s*(\d+)\)', fh.read()).groups())


if __name__ == '__main__':
    main()