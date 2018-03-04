import re
from collections import defaultdict
import tubes 


def setup(app):
    app.connect('autodoc-process-docstring', process_docstring)


def make_tubes():
    for dtype, value in [
        (tubes.Null,  None),
        (tubes.Bool, False),
        (tubes.Int64, 1),
        (tubes.Float, 1.1),
        (tubes.ByteSlice, b'abc'),
        (tubes.Utf8, 'abc'),
        (tubes.Object, 'a'),
        (tubes.JsonUtf8, '1')]:
        yield (dtype, tubes.Each([value]).to(dtype), value)


def test_compatibility(test):
    compatible = []
    for dtype, tube, value in make_tubes():
        try:
            eval(test, {"dtype": dtype, "tube": tube, "value": value})
        except ValueError:
            pass
        else:
            compatible.append('``{0}``'.format(dtype.name))
    return compatible


def describe_compatibility(test):
    compatible_types = test_compatibility(test)
    yield 'Compatible input types: {0}'.format(", ".join(compatible_types))
    yield ''


def test_cross_compatibility(test):
    results = defaultdict(dict)
    for (from_dtype, from_tube, _) in make_tubes():
        for (to_dtype, to_tube, _) in make_tubes():
            try:
                eval(test, {"from_tube": from_tube, "to_tube": to_tube})
            except ValueError:
                worked = False
            else:
                worked = True
            results[from_dtype][to_dtype] = worked
    return results


def describe_cross_compatibility(test):
    results = test_cross_compatibility(test)
    types = list(results.keys())
    yield ''
    yield "+------------+" + ("--------+" * len(results))
    yield "|        To→" + (" |       " * len(types)) + " |"
    yield "+------------+" + ("        +" * len(results))
    yield "| ↓From      | " + " | ".join(t.name.ljust(6) for t in types) + " |"
    yield "+============+" + ("========+" * len(results))
    for from_type in types:
        line = "| {0} ".format(from_type.name.ljust(10))
        for to_type in types:
            result = results[from_type][to_type]
            value = "✓" if result else " "
            line += "|   {0}    ".format(value)
        line += "|"
        yield line
        yield "+------------+" + ("--------+" * len(results))


def process_docstring(app, what, name, obj, options, lines):
    if not lines:
        return None
    out = []
    def add(gen):
        for line in gen:
            out.append(line.rstrip())

    for line in lines:
        match = re.match("Compatibility: (.*)", line)
        if match:
            add(describe_compatibility(match.group(1)))
        else:
            x_match = re.match("XCompatibility: (.*)", line)
            if x_match:
                add(describe_cross_compatibility(x_match.group(1)))
            else:
                out.append(line)
    lines[:] = out
    # import ipdb; ipdb.set_trace()

    # The 'lines' in Sphinx is a list of strings and the value should be changed
