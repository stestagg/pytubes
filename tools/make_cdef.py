import re
import sys
from collections import defaultdict
from itertools import chain
from os import path

import yaml
from jinja2 import Template

PROJECT_DIR = path.dirname(path.dirname(__file__))


START_MARKER = re.escape("/*<-")
END_MARKER = re.escape("->*/")
def find_blocks(file_name):
    with open(file_name) as fp:
        matches = re.findall(START_MARKER + "(.*?)" + END_MARKER, fp.read(), re.M | re.S)
    blocks = []
    for match in matches:
        lines = [l for l in match.splitlines() if l.strip != ""]
        padding = len(lines[0]) - len(lines[0].lstrip())
        doc = "\n".join([l[padding:] for l in lines])
        blocks.append(yaml.load(doc, Loader=yaml.Loader))
    return blocks


class Iter:

    def __init__(self, name, spec, file_name):
        self.file_name = file_name
        self.name = name
        self.spec = {"init": spec} if isinstance(spec, list) else spec

    @property
    def templated_name(self):
        template = self.spec.get("template")
        if template:
            if isinstance(template, (list, tuple)):
                return "{0}[{1}]".format(self.name, ', '.join(template))
            return "{0}[{1}]".format(self.name, template)
        return self.name

    @property
    def extra(self):
        return self.spec.get("extra" ,"")


    @property
    def constructor_args(self):
        return ", ".join(self.spec['init'])


class Prop:
    def __init__(self, spec):
        if isinstance(spec, str):
            ty, name = spec.split(" ", 1)
            if "=" in name:
                name, default = name.split("=", 1)
                self.spec = {"type": ty, "name": name, "default": default}
            else:
                self.spec = {"type": ty, "name": name}

        else:
            self.spec = spec

    def __getattr__(self, name):
        return self.spec[name]

    @property
    def has_default(self):
        return "default" in self.spec

    @property
    def dtypes(self):
        return self.spec.get("dtypes", ())

    @property
    def printable(self):
        return self.spec.get("print", not self.is_tube)

    @property
    def is_tube(self):
        return self.type == "Tube"


class Tube:

    def __init__(self, name, spec, file_name, iters):
        self._iters = iters
        self.file_name = file_name
        self.name = name
        self.spec = spec

    @property
    def props(self):
        return [Prop(p) for p in self.spec["props"]]

    def __getattr__(self, name):
        return self.spec.get(name, None)

    @property
    def init_args(self):
        args = []
        for p in self.props:
            if "default" in p.spec:
                args.append("{0} {1}={2}".format(p.type, p.name, p.default))
            else:
                args.append("{0} {1}".format(p.type, p.name))
        return ", ".join(args)

    @property
    def inputs(self):
        return [p for p in self.props if p.is_tube]

    @property
    def chains(self):
        return self.spec.get("chains", ())

    @property
    def iter(self):
        return self._iters[self.spec['iter'][0]]

    @property
    def iter_args(self):
        return self.spec['iter'][1]

    @property
    def methods(self):
        return self.spec.get("methods", "")



def make_cdef(file_names):
    iters = {}
    tubes = {}
    fns = []

    for file_name in file_names:
        file_name = path.abspath(file_name)
        for block in find_blocks(file_name):
            if "Fn" in block:
                fns.append((file_name, block["Fn"]))
            if "Iter" in block:
                for name, values in block["Iter"].items():
                    iters[name] = Iter(name, values, file_name)
            if "Tube" in block:
                for name, values in block["Tube"].items():
                    tubes[name] = Tube(name, values, file_name, iters)

    with open(path.join(PROJECT_DIR, "tools/defn.tmpl"), "r") as fh:
        template = Template(fh.read())

    print(template.render(
        iters=iters,
        tubes=tubes,
        fns=fns,
        enumerate=enumerate,
        len=len
    ))


if __name__ == '__main__':
    make_cdef(sys.argv[1:])
