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

Results on MacBook (in 1000 calls/sec):

```
method                single thread  single thread std  two threads  two threads std
pybind w/ GIL held     3609.62        300.60             3522.75      761.29
pybind w/ GIL dropped  2936.43        218.11              297.10      296.08
C API w/ GIL held     28183.92       2679.00            27485.77     2676.13
C API w/ GIL dropped  10326.72        667.02             1819.31     1972.39
```

![Plot](plot.svg)
