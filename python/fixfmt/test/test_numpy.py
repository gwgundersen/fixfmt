import numpy as np

import fixfmt
import fixfmt.numpy

#-------------------------------------------------------------------------------

def test_choose_formatter_int():
    arr = np.arange(800, 1200)
    fmt = fixfmt.numpy.choose_formatter(arr)
    assert isinstance(fmt, fixfmt.Number)
    assert fmt.size == 4
    assert fmt.precision is None


def test_choose_formatter_float():
    arr = np.arange(1001) * 0.01
    fmt = fixfmt.numpy.choose_formatter(arr)
    assert isinstance(fmt, fixfmt.Number)
    assert fmt.size == 2
    assert fmt.precision == 2


def test_choose_formatter_datetime1():
    t0 = np.datetime64("2018-01-18T17:05:00")
    t1 = np.datetime64("2018-01-18T18:05:02")
    arr = np.arange(t0, t1, np.timedelta64(100, "ms"))
    fmt = fixfmt.numpy.choose_formatter(arr)
    assert isinstance(fmt, fixfmt.TickTime)
    assert fmt.scale == 1000
    assert fmt.precision == 1


def test_choose_formatter_datetime2():
    t0 = np.datetime64("2018-01-18T17:05:00")
    t1 = np.datetime64("2018-01-18T18:05:02")
    arr = np.arange(t0, t1, np.timedelta64(10000, "us"))
    fmt = fixfmt.numpy.choose_formatter(arr)
    assert isinstance(fmt, fixfmt.TickTime)
    assert fmt.scale == 1000000
    assert fmt.precision == 2


