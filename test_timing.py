import math

import numpy as np

import gilc


def _test_values(timing, values):
    for v in values:
        timing.call(v)

    assert timing.count() == len(values)
    np.testing.assert_almost_equal(timing.mean(), np.mean(values))
    np.testing.assert_almost_equal(timing.var(), np.var(values))
    np.testing.assert_almost_equal(timing.std(), np.std(values))


class TestTiming:
    def test_simple(self):
        _test_values(gilc.Timing(), list(range(10)))

    def test_gamma(self):
        xs = np.arange(0.01, 3, 0.01)
        ys = [math.gamma(x) for x in xs]
        _test_values(gilc.Timing(), ys)
