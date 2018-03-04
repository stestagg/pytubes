
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
        check_call("pyenv", "install", "-s", ver)
        check_call("pyenv", "global", ver)
        sleep(1)
        current_version = check_call("python", "--version", output=True)
        if current_version != "Python %s" % (ver, ):
            check_call("which", "python", )
            check_call("ls", "-lah", check_call("which", "python", output=True))
            check_call("python", "-c", "import re; print(re)")
            raise ValueError("Requested version: %r is not current version: %r" % (ver, current_version))
        with Log("Installing build dependencies"):
            check_call("pip", "install", "-q", "-r", REQUIREMENTS)
        with Log("Cleaning build intermediate files"):
            check_call("make", "clean-py",  "clean-cpp")
        with Log("Running tests"):
            check_call("make", "test")
        with Log("Building wheel"):
            check_call("pip", "wheel", ".", "-w", WHEELHOUSE)


def build_osx(ver):
    with Log("Building for OSX python %s" % ver):
        set_py_version(ver)

def build_anylinux(ver):
    check_call(
        "docker", "run", "-it", "--rm", 
        '-v', "%s:/pytubes" % PROJECT_DIR, 
        "quay.io/pypa/manylinux1_x86_64", 
        "/pytubes/tools/build_anylinux.sh", ver
    )


def main():
    with Log("Patching environment"):
        sdk_path = check_call("xcrun", "--show-sdk-path", output=True)
        os.environ['CFLAGS'] = "-I" + path.join(sdk_path, "usr", "include")
        from pprint import pprint
        os.environ['PYENV_VERSION'] = ""
        os.environ['PATH'] = path.join(check_call("pyenv", "root", output=True), "shims") + ":" + os.environ["PATH"]
    with Log("Building OSX wheels"):
        build_osx("3.4.8")
        build_osx("3.5.5")
        build_osx("3.6.4")
    with Log("Building anylinux wheels"):
        build_anylinux("cp34-cp34m")
        build_anylinux("cp35-cp35m")
        build_anylinux("cp36-cp36m")




if __name__ == '__main__':
    sys.exit(main())