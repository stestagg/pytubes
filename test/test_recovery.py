import os
import os.path as path
import json

import pytest

import tubes


def test_recover_bad_json():
    tube = tubes.Each(['[1,2]', '[', '{"a": 1}']).to(str).json()
    it = iter(tube)
    results = []
    while True:
        try:
            results.append(next(it))
        except ValueError as e:
            results.append('ERR')
        except StopIteration:
            break
    assert results == [[1,2], 'ERR', {'a': 1}]


def test_recover_bad_csv():
    tube = tubes.Each(['a,b\n1,2\n3,4\n"x']).csv().multi(lambda x: (x.get(0), x.get(1)))
    it = iter(tube)
    results = []
    while True:
        try:
            results.append(next(it))
        except ValueError as e:
            results.append('ERR')
        except StopIteration:
            break
    assert results == [(b'1',b'2'), (b'3', b'4'), 'ERR']


def test_recover_bad_json_with_skip():
    tube = tubes.Each(['[1,2]', '[', '{"a": 1}', '12']).to(str).json().skip(2)
    it = iter(tube)
    results = []
    while True:
        try:
            results.append(next(it))
        except ValueError as e:
            results.append('ERR')
        except StopIteration:
            break
    # TODO: This /should/ return {"a": 1}, 12
    # but rewinding the stack to the right place is hard
    assert results == ['ERR', 12]


