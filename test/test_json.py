import os
import os.path as path
import json

import pytest

import tubes

try:
    RecursionError
except NameError:
    RecursionError = RuntimeError

THIS_DIR = path.dirname(__file__)

def read_file(*parts):
    with open(path.join(*parts), encoding="utf-8") as fh:
        return fh.read()

#SAMPLE_JSON is a corpus of json snippets that should all be parsable.
SAMPLE_JSON = [
    '[]', '{}', 'false', '1', '1.1',
    'null', '""', '[{}]', '{"a": []}',
    '[[[[[[[[[[]]]]]]]]]]',
    read_file(THIS_DIR, "tricky_json.json")
]
for i in range(0, 10, 2):
    SAMPLE_JSON.append('"' + ('\\' * i) + '"')
for i in range(1, 10, 2):
    SAMPLE_JSON.append('"' + ('\\' * i) + '"1"')


# Please note, the tubes json parser is *not* validating.  It assumes that the 
# json is valid unless it is forced not to.  Therefore, many invalid JSONs may
# be accepted as valid.

# This test uses the JSONTestSuite from https://github.com/nst/JSONTestSuite
# and expects tubes.json() to parse/convert any y_ or i_ tests that the python
# json.loads() call can (and produce same output)

TEST_CASE_DIR = path.join(THIS_DIR, "JSONTestSuite_parse_tests")
BLACKLIST = {  # Generally speaking, json.loads() is very permissive about 
               # Utf-8 reading, but python string conversion fns aren't
               # so these produce errors/inconsistent outputs
    "i_string_overlong_sequence_6_bytes.json",
    "i_string_incomplete_surrogates_escape_valid.json",
    "i_string_lone_utf8_continuation_byte.json",
    "i_string_UTF-16LE_with_BOM.json",
    "i_string_iso_latin_1.json",
    "i_string_1st_valid_surrogate_2nd_invalid.json",
    "i_string_utf16BE_no_BOM.json",
    "i_string_UTF8_surrogate_U+D800.json",
    "i_string_utf16LE_no_BOM.json",
    "i_string_overlong_sequence_6_bytes_null.json",
    "i_string_truncated-utf-8.json",
    "i_string_overlong_sequence_2_bytes.json",
    "i_string_not_in_unicode_range.json",
    "i_string_invalid_utf-8.json",
    "i_string_UTF-8_invalid_sequence.json",
}
def _valid(f):
    if f in BLACKLIST:
        return False
    return f.endswith(".json") and not f.startswith("n_")

TEST_CASES = [f for f in os.listdir(TEST_CASE_DIR) if _valid(f)]
@pytest.mark.parametrize("filename", TEST_CASES)
def test_passing_json_test_suite_cases(filename):
    test_path = path.join(TEST_CASE_DIR, filename)
    data = read_file(test_path)
    try:
        py_version = json.loads(data)
    except (ValueError, RecursionError):
        return

    tubes_version = tubes.Each([test_path]).map_files().json()
    assert list(tubes_version)[0] == py_version


# Pytest puts the input data into the test name, which is useful for debugging,
# The current test name is put in the environment in pytest
# BUT, in this case, the tricky_json.json file is too long for the WIN32 environment
# so breaks.  Instead, pass the index into the SAMPLE_JSON list as a parameter to the test
# to avoid putting 100s KB json into the environment
@pytest.mark.parametrize("sample_num", range(len(SAMPLE_JSON)))
def test_json(sample_num):
    sample = SAMPLE_JSON[sample_num]
    assert list(tubes.Each([sample]).to(str).json())[0] == json.loads(sample)