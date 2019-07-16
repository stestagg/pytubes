import re
from pathlib import Path

ROOT = Path(".").resolve()


def get_version_list():
	subdirs = [x.name for x in ROOT.iterdir() if x.is_dir()]
	return sorted(subdirs)


PATTERN = r"PYTUBES\-BEGIN\-VERSION\-LIST.*PYTUBES\-END\-VERSION\-LIST"

def update_versions(versions):
	option_list = "".join(['<option value="%s">%s</option>' % (v, v) for v in versions])
	replacement = "PYTUBES-BEGIN-VERSION-LIST -->" + option_list + "<!-- PYTUBES-END-VERSION-LIST"
	for path in ROOT.glob("**/*.html"):
		with path.open() as fh:
			content = fh.read()
		new_content = re.sub(PATTERN, replacement, content, flags=re.M + re.S)
		if content != new_content:
			print("Updating: " + path.name)
			with path.open("w") as fh:
				fh.write(new_content)


if __name__ == '__main__':
	versions = get_version_list()
	update_versions(versions)