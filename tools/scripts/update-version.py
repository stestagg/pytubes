import pathlib
from os import environ
import re

RELEASE_TAG_PATTERN = r"refs/tags/\d.\d.\d"

THIS_FILE = pathlib.Path(__file__).resolve()

PROJECT_DIR = THIS_FILE.parent.parent.parent
VERSION_FILE = PROJECT_DIR / "pyx" / "version.pxi"

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
    with VERSION_FILE.open("w") as fh:
        fh.write(f"__version__ = {repr(version)}\n")