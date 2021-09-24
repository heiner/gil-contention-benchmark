import math

import numpy as np
import pytest

import gilc
import main


def _test_values(timing, values):
    for v in values:
        timing.call(v)

    assert timing.count() == len(values)
    np.testing.assert_almost_equal(timing.mean(), np.mean(values))
    np.testing.assert_almost_equal(timing.var(), np.var(values))
    np.testing.assert_almost_equal(timing.std(), np.std(values))


@pytest.mark.parametrize("Timing", [gilc.Timing, gilc.CTiming])
class TestTiming:
    def test_simple(self, Timing):
        _test_values(Timing(), list(range(10)))

    xs = np.arange(0.01, 3, 0.01)
    ys = [math.gamma(x) for x in xs]

    def test_gamma(self, Timing):
        _test_values(gilc.Timing(), TestTiming.ys)

    def test_pooled(self, Timing):
        n = len(TestTiming.ys)
        n0 = n // 4

        ys = TestTiming.ys

        ys0 = ys[:n0]
        ys1 = ys[n0:]

        assert n0 == len(ys0)
        n1 = len(ys1)

        t0 = Timing()
        t1 = Timing()
        _test_values(t0, ys0)
        _test_values(t1, ys1)

        ts = [t0, t1]

        count, mean, var = main.pool(t0, t1)

        assert count == len(ys)
        assert mean == np.mean(ys)
        np.testing.assert_almost_equal(var, np.var(ys))
