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


def a(method="gilc"):
    """Case a): single threaded:
    for _ in range(10000000):
      test_abc()"""

    print("case a with", method)
    g = gilc.Gilc()
    speeds = loop(int(1e6), call=getattr(g, method))
    print("% 12.2f +- %.2f" % (np.mean(speeds), np.std(speeds)))


def b(method="gilc_nogil"):
    """Case b): multi threaded, two threads, each doing a)."""
    print("case b with", method)
    import queue
    import threading

    q = queue.Queue()

    def target():
        g = gilc.Gilc()
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
    print("% 12.2f +- %.2f" % (np.mean(speeds), np.std(speeds)))


def main():
    for method in ("gilc", "gilc_nogil"):
        a(method)
        b(method)


if __name__ == "__main__":
    main()
