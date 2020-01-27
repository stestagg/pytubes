import re
import shutil
from pathlib import Path

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


VERSION_INDEX = r"PYTUBES\-BEGIN\-VERSION\-INDEX.*PYTUBES\-END\-VERSION\-INDEX"
VERSION_SELECTOR = r"PYTUBES\-BEGIN\-VERSION\-LIST.*PYTUBES\-END\-VERSION\-LIST"
VERSION_WARNING = r"PYTUBES\-BEGIN\-VERSION\-WARNING.*PYTUBES\-END\-VERSION\-WARNING"

def update_docs_pages(versions):
    for path in VERSION_DIR.rglob("*.html"):
        local_version = path.relative_to(VERSION_DIR).parts[0]
        update_version_selector(path, versions)
        update_version_warning(path, local_version, versions[-1])


def update_version_selector(path, versions):
    option_list = "".join([f'<option value="{v}">{v}</option>' for v in versions[::-1]])
    replacement = f"PYTUBES-BEGIN-VERSION-LIST -->{option_list}<!-- PYTUBES-END-VERSION-LIST"
    file_sub(path, VERSION_SELECTOR, replacement)


def update_version_warning(path, version, latest_version):
    if latest_version == version:
        message = ""
    else:
        message = f"""<div style="color:red;background: #ffe;">
      You're currently viewing an older version of pytubes.
      <a href="/versions/{latest_version}/">Switch to {latest_version}</a>
    </div>"""
    replacement = f"PYTUBES-BEGIN-VERSION-WARNING -->{message}<!-- PYTUBES-END-VERSION-WARNING"
    file_sub(path, VERSION_WARNING, replacement)

def update_version_index(versions):
    path = VERSION_DIR / 'index.html'
    items = "".join([f'<li><a href="/versions/{v}">{v}</a></li>' for v in versions[::-1]])
    replacement = f"PYTUBES-BEGIN-VERSION-INDEX -->{items}<!-- PYTUBES-END-VERSION-INDEX"
    file_sub(path, VERSION_INDEX, replacement)

def update_redirect(version, file):
    with open(file, 'r') as fh:
        current_index = fh.read()
    target_url = f'https://docs.pytubes.com/versions/{version}"'
    updated = re.sub('https://docs.pytubes.com/versions/.*?"', target_url, current_index)
    if updated != current_index:
        print(f"Updating {file.relative_to(ROOT)}")
        with open(file, 'w') as fh:
            fh.write(updated)

def update_statics(latest_version):
    alabaster = VERSION_DIR / latest_version / '_static' / 'alabaster.css'
    shutil.copyfile(str(alabaster), ROOT / 'alabaster.css')


if __name__ == '__main__':
    versions = sorted(get_version_list(), key=get_ver_parts)
    print(f"Found Versions: {versions}")
    assert len(versions) > 0
    update_statics(versions[-1])
    update_docs_pages(versions)
    update_redirect(versions[-1], ROOT / 'index.html')
    update_redirect(versions[-1], ROOT / 'versions' / 'latest' / 'index.html')
    update_version_index(versions)
