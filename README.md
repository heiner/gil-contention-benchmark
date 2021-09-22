toy example for GIL contention test.

test_abc() measures calls/sec

a) single threaded:
for _ in range(10000000):
  test_abc()

b) Case b): multi threaded, two threads, each doing a), but
test_abc() dropps the GIL

```
python setup.py develop
python main.py | tee data.dat
```

Results on MacBook:

```
method	single thread	single thread std	two threads	two threads std
pybind w/ gilc	3631826.1535959174	263096.4624542195	3561167.3317479948	791521.1382301509
pybind w/ gilc_nogil	3080329.7107364074	277441.6010673747	302585.88223813544	325705.6648542893
C API w/ call	29172772.918535203	1921468.032008213	27351489.45980912	5685750.208039337
C API w/ call_nogil	10990427.92607121	831886.3989174861	1602937.4870584928	1351681.7832822218
```

![Plot](plot.svg)
