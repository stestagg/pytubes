import time
import sys
import os
import tubes
import gzip
import json

DATA_DIR = "../data"
FILES = [os.path.join(DATA_DIR, f) for f in os.listdir(DATA_DIR) if f.startswith("pypi-downloads") and f.endswith("jsonz")]
SKIP = 0
TAKE = 100000

KEYS = (
    ("timestamp", ),
    ("country_code", ),
    ("url", ),
    ("file", "filename"),
    ("file", "project"),
    ("details", "installer", "name"),
    ("details", "python"),
    ("details", "system"),
    ("details", "system", "name"),
    ("details", "cpu"),
    ("details", "distro", "libc", "lib"),
    ("details", "distro", "libc", "version"),
)

def py_version():
    skip = SKIP
    take = TAKE
    results = []
    for file_name in FILES:
        with gzip.open(file_name, "rt") as fp:
            for line in fp:
                data = json.loads(line)
                if skip:
                    skip -= 1
                    continue
                if data.get('country_code') != "GB":
                    continue
                if not take:
                    return results
                take -= 1
                row = []
                for path in KEYS:
                    base = data
                    for part in path:
                        base = base.get(part, None)
                        if base is None:
                            break
                    row.append(base)
                results.append(row)
    return results

def make_getters(x):
    getters = []
    for path in KEYS:
        base = x
        for part in path:
            base = base.get(part, 'null')
        getters.append(base)
    return tuple(getters)

def tubes_version():
    x = (tubes.Each(FILES)
        .map_files()
        .gunzip()
        .split(b'\n')
        .skip(SKIP)
        .json()
        .skip_unless(lambda x: x.get('country_code', '""').to(tubes.Utf8).equals("GB"))
        .first(TAKE)
        .multi(make_getters)
    )
    return list(x)

def tubes_version_2():
    x = (tubes.Each(FILES)
        .read_files()
        .gunzip(stream=True)
        .split(b'\n')
        .chunk(1)
        .skip(SKIP)
        .json()
        .skip_unless(lambda x: x.get('country_code', '""').to(tubes.Utf8).equals("GB"))
        .first(TAKE)
        .multi(make_getters)
    )
    return list(x)

def compare():
    print("Skipping: {0}, Taking: {1}".format(SKIP, TAKE))
    print("Tubes v1")
    a = time.perf_counter()
    tube_1_vals = tubes_version()
    b = time.perf_counter()
    tubes_1_time = b - a
    print("Took: {0:.4f} s".format(tubes_1_time))
    print("Tubes v2")
    b = time.perf_counter()
    tube_2_vals = tubes_version_2()
    c = time.perf_counter()
    tubes_2_time = c - b
    print("Took: {0:.4f} s".format(tubes_2_time))
    print("Py version")
    c = time.perf_counter()
    py_vals = py_version()
    d = time.perf_counter()
    py_time = d - c
    print("Took: {0:.4f} s".format(py_time))
    best = min(tubes_1_time, tubes_2_time)
    print("Speedup: {0:.2f} x".format(py_time/best))

    print(py_vals[0])
    print(tube_2_vals[0])
    print(tube_1_vals[0])

    assert tuple(py_vals[-1]) == tube_2_vals[-1], (tuple(py_vals[-1]), tube_2_vals[-1])
    assert tuple(py_vals[-1]) == tube_1_vals[-1], (tuple(py_vals[-1]), tube_1_vals[-1])
    # assert len(pyx) == len(tubesx), "Different lengths"
    # tubex_str = [x.decode('utf-8') if x is not None else x for x in tubesx]
    # same = [a==b for a, b in zip(pyx, tubex_str)]
    # assert pyx == tubex_str, list(zip(pyx, tubex_str, same))

def main(ty):
    global SKIP, TAKE
    if ty == "perf":
        SKIP = 1
        TAKE = 2000000
        print("Perf test")
        result = tubes_version_2()
        print("Got: {0}".format(len(result)))
    else:
        print("== speed test ==")
        compare()

if __name__ == '__main__':
    print(tubes)
    main('run' if len(sys.argv) < 2 else sys.argv[1])