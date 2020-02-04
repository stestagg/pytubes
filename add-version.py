import argparse
import re
import shutil
from pathlib import Path

PARSER = argparse.ArgumentParser(description='Add the docs for a new version.')
PARSER.add_argument('path', help='Path to the new docs to add')

ROOT = Path(__file__).parent.resolve()
VERSION_DIR = ROOT / 'versions'

def try_int(value):
    try:
        return int(value)
    except ValueError:
        return 0

def looks_like_version(ver):
    return ver and ver.strip()[0] != '.' and '.' in ver

def get_ver_parts(ver):
    parts = ver.lstrip('v').strip().split('.')
    return [(try_int(part), part) for part in parts]

def get_version_list():
    subdirs = [x.parent.name for x in VERSION_DIR.glob("*/index.html")]
    valid = [s for s in subdirs if looks_like_version(s)]
    return sorted(valid, key=get_ver_parts)

def file_sub(path, pattern, replacement):
    with path.open() as fh:
        content = fh.read()
    new_content = re.sub(pattern, replacement, content, flags=re.M + re.S)
    if content != new_content:
        print(f"Updating: {path.relative_to(ROOT)}")
        with path.open("w") as fh:
            fh.write(new_content)


def get_version(path):
    index = path / 'index.html'
    with index.open() as fh:
        index_body = fh.read()
    versions = re.findall("^\s+Version\:\s*(.*)$", index_body, re.M)
    assert len(versions) > 0
    version = versions[0]

    destination = VERSION_DIR / version
    assert not destination.exists()
    print(f"Copying {path} to {destination}")
    shutil.copytree(path, destination)

if __name__ == '__main__':
    options = PARSER.parse_args()
    new_path = Path(options.path)
    ver = get_version(new_path)
