
import os
from time import sleep
import shlex
from functools import partial
from os import path
import subprocess
import sys

from fin import contextlog

Log = partial(contextlog.Log, theme="mac")

PROJECT_DIR = path.abspath(path.dirname(path.dirname(__file__)))

WHEELHOUSE = path.join(PROJECT_DIR, "wheelhouse")
REQUIREMENTS = path.join(PROJECT_DIR, "build_requirements.txt")


def check_call(*args, output=False):
    cmdline = " ".join(shlex.quote(p) for p in args)
    with Log("Running: %s" % cmdline):
        if output:
            return subprocess.check_output(cmdline, shell=True).strip().decode("utf-8")
        else:
            return subprocess.check_call(cmdline, shell=True)


def set_py_version(ver):
    with Log("Enabling python: %s" % ver):
        previous_version = check_call('pyenv', 'global', output=True)
        check_call("pyenv", "install", "-s", ver)
        check_call("pyenv", "global", ver)
        try:
            sleep(1)
            current_version = check_call("python", "--version", output=True)
            if current_version != "Python %s" % (ver, ):
                check_call("which", "python", )
                check_call("ls", "-lah", check_call("which", "python", output=True))
                check_call("python", "-c", "import re; print(re)")
                raise ValueError("Requested version: %r is not current version: %r" % (ver, current_version))
            with Log("Updating pip"):
                check_call("pip", "install", "-q", "--upgrade", 'pip')
            with Log("Installing build dependencies"):
                check_call("pip", "install", "-q", "-r", REQUIREMENTS)
            with Log("Cleaning build intermediate files"):
                check_call("make", "clean-py",  "clean-cpp")
            with Log("Running tests"):
                check_call("make", "test")
            with Log("Building wheel"):
                check_call("pip", "wheel", ".", "-w", WHEELHOUSE)
        finally:
            check_call("pyenv", "global", previous_version)


def build_osx(ver):
    with Log("Building for OSX python %s" % ver):
        set_py_version(ver)

def build_anylinux(ver):
    check_call(
        "docker", "run", "-it", "--rm",
        '-v', "%s:/pytubes" % PROJECT_DIR,
        "quay.io/pypa/manylinux2010_x86_64",
        "/pytubes/tools/build_anylinux.sh", ver
    )


def main():
    with Log("Patching environment"):
        os.environ['PYENV_VERSION'] = ""
        os.environ['PATH'] = path.join(check_call("pyenv", "root", output=True), "shims") + ":" + os.environ["PATH"]
    with Log("Getting version") as l:
        version = check_call("python", "tools/version.py", output=True)
        l.format("Version: %s", version)
    with Log("Building OSX wheels"):
        build_osx("3.5.5")
        build_osx("3.6.4")
        build_osx("3.7.1")
    with Log("Building anylinux wheels"):
        build_anylinux("cp35-cp35m")
        build_anylinux("cp36-cp36m")
        build_anylinux("cp37-cp37m")
    with Log("Tagging"):
        check_call("git", "tag", "-a", version, "-m", "build_wheels.py Tagging version %s" % version)





if __name__ == '__main__':
    sys.exit(main())