import numpy as np

from gilc import gilc


def loop(upto, call):
    g = gilc.Gilc()

    speeds = []

    try:
        for _ in range(upto):
            speed = call()
            if speed:
                speeds.append(speed)
    except KeyboardInterrupt:
        print("(Stopping.)")
    return speeds


def a(new_object=gilc.Gilc, method="gilc", label=""):
    """Case a): single threaded:
    for _ in range(10000000):
      test_abc()"""

    g = new_object()
    speeds = loop(int(1e6), call=getattr(g, method))

    return np.mean(speeds), np.std(speeds)


def b(new_object=gilc.Gilc, method="gilc_nogil", label=""):
    """Case b): multi threaded, two threads, each doing a)."""
    import queue
    import threading

    q = queue.Queue()

    def target():
        g = new_object()
        q.put(loop(int(1e6), call=getattr(g, method)))

    threads = []
    for _ in range(2):
        threads.append(threading.Thread(target=target))
    for t in threads:
        t.start()
    for t in threads:
        t.join()

    speeds = []
    while not q.empty():
        speeds += q.get()

    return np.mean(speeds), np.std(speeds)


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
        return "GIL"

    for method in ("gilc", "gilc_nogil"):
        a_mean, a_std = a(
            gilc.Gilc, method, label="pybind11: single thread w/ %s" % label(method)
        )
        b_mean, b_std = b(
            gilc.Gilc, method, label="pybind11: two threads w/ %s" % label(method)
        )
        print(
            "\t".join(["%s"] + 4 * ["%s"])
            % ("pybind w/ %s" % method, a_mean, a_std, b_mean, b_std)
        )

    for method in ("call", "call_nogil"):
        a_mean, a_std = a(
            gilc.mycmodule.MyCObject,
            method,
            label="C API: single thread w/ %s" % label(method),
        )
        b_mean, b_std = b(
            gilc.mycmodule.MyCObject,
            method,
            label="C API: two threads w/ %s" % label(method),
        )
        print(
            "\t".join(["%s"] + 4 * ["%s"])
            % ("C API w/ %s" % method, a_mean, a_std, b_mean, b_std)
        )


if __name__ == "__main__":
    main()
