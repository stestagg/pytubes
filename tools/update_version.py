from os import path, environ
import re

RELEASE_TAG_PATTERN = r"refs/tags/\d.\d.\d"

PROJECT_DIR = path.dirname(path.dirname(__file__))
VERSION_FILE = path.join(PROJECT_DIR, 'pyx', 'version.pxi')

def main():
    with open(VERSION_FILE, "r") as fh:
        print("%s.%s.%s" % re.match(r'__version__\s*=\s*\((\d+),\s*(\d+),\s*(\d+)\)', fh.read()).groups())


def get_version():
    ref = environ.get("GITHUB_REF", "__LOCAL__")

    tag_match = re.match(RELEASE_TAG_PATTERN, ref)
    if tag_match:
        major = int(tag_match.group(1))
        minor = int(tag_match.group(2))
        maint = int(tag_match.group(3))
        return (major, minor, maint)

    return (0, 0, 99)


if __name__ == '__main__':
    version = get_version()
    with open(VERSION_FILE, "w") as fh:
        fh.write(f"__version__ = {repr(version)}\n")