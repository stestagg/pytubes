import tubes
import sys
import gc

class Flag:
    def __init__(self):
        self.is_set = False

    def set(self):
        assert self.is_set == False
        self.is_set = True


class Canary:
    def __init__(self, flag):
        self.flag = flag

    def __del__(self):
        self.flag.set()


def test_refcount_call():
    rc = sys.getrefcount(Flag())
    assert rc == 1
    b = Flag()
    rc = sys.getrefcount(b)
    assert rc == 2

def test_canary():
    flag = Flag()
    def make():
        a = Canary(flag)
    make()
    assert flag.is_set


def test_numpy_object_refcount():
    flag = Flag()
    val = Canary(flag)
    assert sys.getrefcount(val) == 2
    tube = tubes.Each([val])
    assert sys.getrefcount(val) == 3
    gc.collect()
    array = tube.ndarray()
    assert sys.getrefcount(val) == 4
    del tube
    gc.collect()
    assert sys.getrefcount(val) == 3
    del array
    gc.collect()
    assert sys.getrefcount(val) == 2
    del val
    gc.collect()
    assert flag.is_set



def test_to_py_handles_refcount_list():
    """
    sys.getrefcount() value is always one higher than expected
    because the call to getrefcount() itself needs a reference..
    """
    flag = Flag()
    a = Canary(flag)

    assert sys.getrefcount(a) == 2  # a
    tube = tubes.Each([True, a]).to_py()
    assert sys.getrefcount(tube) == 2  # tube
    assert sys.getrefcount(a) == 3  # a + each_val
    it = iter(tube)
    gc.collect()
    assert sys.getrefcount(tube) == 2  # iter() doesn't keep reference to tube
    assert sys.getrefcount(a) == 3  # a + each_val
    assert next(it) is True
    val = next(it)
    assert val is a
    gc.collect()
    assert sys.getrefcount(a) == 6  # a + each_val + val + iter_cur + topy_cur
    del it
    gc.collect()
    assert sys.getrefcount(a) == 4   # a + each_val + val
    del tube
    gc.collect()
    assert sys.getrefcount(a) == 3  # a + val
    del val
    assert sys.getrefcount(a) == 2  # a
    del a
    gc.collect()
    assert flag.is_set


def test_to_py_handles_refcount_iter():
    flag = Flag()
    a = Canary(flag)

    assert sys.getrefcount(a) == 2  # a
    tube = tubes.Each(iter([True, a])).to_py()
    assert sys.getrefcount(tube) == 2  # iter
    assert sys.getrefcount(a) == 3  # a + each_val
    it = iter(tube)
    gc.collect()
    assert sys.getrefcount(tube) == 2  # iter() doesn't keep reference to tube
    assert sys.getrefcount(a) == 3  # a + each_val
    assert next(it) is True
    val = next(it)
    assert val is a
    gc.collect()
    assert sys.getrefcount(a) == 6  # a + each_val + val + iter_cur + topy_cur
    del it
    gc.collect()
    assert sys.getrefcount(a) == 4   # a + each_val + val
    del tube
    gc.collect()
    assert sys.getrefcount(a) == 3  # a + val
    del val
    assert sys.getrefcount(a) == 2  # a
    del a
    gc.collect()
    assert flag.is_set
