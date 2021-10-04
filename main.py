import csv
import queue
import sys
import threading

import gilc


def loop(f):
    try:
        while not f():
            pass
    except KeyboardInterrupt:
        print("(Stopping.)")


def a(create_timing=gilc.Timing, method="call"):
    """Case a): single threaded."""
    t = create_timing()
    loop(getattr(t, method))
    return t.num_calls() / t.delta()


def b(create_timing=gilc.Timing, method="call_nogil"):
    """Case b): multi threaded, two threads, each doing a)."""
    q = queue.Queue()

    def target():
        t = create_timing()
        loop(getattr(t, method))
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

    return sum(t.num_calls() / t.delta() for t in timings)


def main():
    writer = csv.writer(sys.stdout, delimiter="\t")
    writer.writerow(("method", "single thread", "two threads"))

    def s(f):
        return "%.1f" % (f / 1000)

    def label(timing, method):
        if timing == gilc.Timing:
            result = "pybind w/ %s"
        elif timing == gilc.CTiming:
            result = "C API w/ %s"

        if method.endswith("_nogil"):
            return result % "GIL dropped"
        return result % "GIL held"

    for Timing in (gilc.Timing, gilc.CTiming):
        for method in ("call", "call_nogil"):
            writer.writerow(
                (
                    label(Timing, method),
                    s(a(Timing, method)),
                    s(b(Timing, method)),
                )
            )

    if "c++" in sys.argv[1:]:
        cpp_a = gilc.cpp_loop()
        cpp_bs = gilc.cpp_loop_threads(2)
        writer.writerow(("C++ loop", s(cpp_a), s(sum(cpp_bs))))


if __name__ == "__main__":
    main()
