import numpy as np

import gilc


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


def a(new_object=gilc.Gilc, method="gilc"):
    """Case a): single threaded:
    for _ in range(10000000):
      test_abc()"""

    g = new_object()
    speeds = loop(int(1e6), call=getattr(g, method))
    return np.mean(speeds) / 1000, np.std(speeds) / 1000


def b(new_object=gilc.Gilc, method="gilc_nogil"):
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

    return np.mean(speeds) / 1000, np.std(speeds) / 1000


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

    for method in ("gilc", "gilc_nogil"):
        a_mean, a_std = a(gilc.Gilc, method)
        b_mean, b_std = b(gilc.Gilc, method)
        print(
            "\t".join(["%s"] + 4 * ["%.2f"])
            % ("pybind w/ %s" % label(method), a_mean, a_std, b_mean, b_std)
        )

    for method in ("call", "call_nogil"):
        a_mean, a_std = a(gilc.mycmodule.MyCObject, method)
        b_mean, b_std = b(gilc.mycmodule.MyCObject, method)
        print(
            "\t".join(["%s"] + 4 * ["%.2f"])
            % ("C API w/ %s" % label(method), a_mean, a_std, b_mean, b_std)
        )


if __name__ == "__main__":
    main()
