import queue
import threading
import numpy as np

import gilc


def loop(upto, call):
    try:
        for _ in range(upto):
            call()
    except KeyboardInterrupt:
        print("(Stopping.)")


def a(create_timing=gilc.Timing, method="time"):
    """Case a): single threaded."""
    t = create_timing()
    loop(int(1e6), call=getattr(t, method))
    return t.mean() / 1000, t.std() / 1000


def b(create_timing=gilc.Timing, method="time_nogil"):
    """Case b): multi threaded, two threads, each doing a)."""
    q = queue.Queue()

    def target():
        t = create_timing()
        loop(int(1e6), call=getattr(t, method))
        q.put(t)

    threads = []
    for _ in range(2):
        threads.append(threading.Thread(target=target))
    for t in threads:
        t.start()
    for t in threads:
        t.join()

    timings = []
    while not q.empty():
        timings.append(q.get())

    count, mean, var = pool(*timings)
    return mean / 1000, np.sqrt(var) / 1000


def pool(t0, t1):
    """Pool two timing objects."""
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


def main():
    print(
        "\t".join(5 * ["%s"])
        % (
            "method",
            "single thread",
            "single thread std",
            "two threads",
            "two threads std",
        )
    )

    def label(s):
        if s.endswith("_nogil"):
            return "GIL dropped"
        return "GIL held"

    for method in ("time", "time_nogil"):
        a_mean, a_std = a(gilc.Timing, method)
        b_mean, b_std = b(gilc.Timing, method)
        print(
            "\t".join(["%s"] + 4 * ["%.2f"])
            % ("pybind w/ %s" % label(method), a_mean, a_std, b_mean, b_std)
        )

    for method in ("time", "time_nogil"):
        a_mean, a_std = a(gilc.CTiming, method)
        b_mean, b_std = b(gilc.CTiming, method)
        print(
            "\t".join(["%s"] + 4 * ["%.2f"])
            % ("C API w/ %s" % label(method), a_mean, a_std, b_mean, b_std)
        )


if __name__ == "__main__":
    main()
