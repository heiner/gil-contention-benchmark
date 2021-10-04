import math

import numpy as np
import pytest

import gilc


def _test_values(meanvar, values):
    for v in values:
        meanvar.add(v)

    assert meanvar.count() == len(values)
    np.testing.assert_almost_equal(meanvar.mean(), np.mean(values))
    np.testing.assert_almost_equal(meanvar.var(), np.var(values))
    np.testing.assert_almost_equal(meanvar.std(), np.std(values))


def pool(t0, t1):
    """Pool two MeanVar objects."""
    # Cf.
    # https://en.wikipedia.org/wiki/Pooled_variance#Aggregation_of_standard_deviation_data
    ts = (t0, t1)
    count = sum(t.count() for t in ts)
    mean = sum(t.count() * t.mean() for t in ts) / count
    var = (
        sum(t.count() * t.var() for t in ts) / count
        + t0.count() * t1.count() / count ** 2 * (t0.mean() - t1.mean()) ** 2
    )
    return count, mean, var


class TestMeanVar:
    def test_simple(self):
        _test_values(gilc.MeanVar(), list(range(10)))

    xs = np.arange(0.01, 3, 0.01)
    ys = [math.gamma(x) for x in xs]

    def test_gamma(self):
        _test_values(gilc.MeanVar(), TestMeanVar.ys)

    def test_pooled(self):
        n = len(TestMeanVar.ys)
        n0 = n // 4

        ys = TestMeanVar.ys

        ys0 = ys[:n0]
        ys1 = ys[n0:]

        t0 = gilc.MeanVar()
        t1 = gilc.MeanVar()
        _test_values(t0, ys0)
        _test_values(t1, ys1)

        count, mean, var = pool(t0, t1)

        assert count == len(ys)
        assert mean == np.mean(ys)
        np.testing.assert_almost_equal(var, np.var(ys))


@pytest.mark.parametrize("Timing", [gilc.Timing, gilc.CTiming])
class TestTiming:
    def test_simple(self, Timing):
        t = Timing()
        t1 = Timing(10, 1)
        del t, t1

    def test_run(self, Timing):
        t = Timing(end=90, burn_in=10)

        for _ in range(99):
            assert not t.call()
        assert t.call()

        assert t.num_calls() == 90
        assert t.delta() < 1e-4
