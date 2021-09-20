toy example for GIL contention test.

test_abc() measures calls/sec

a) single threaded:
for _ in range(10000000):
  test_abc()

b) Case b): multi threaded, two threads, each doing a), but
test_abc() dropps the GIL

```
python setup.py develop
python main.py
```

Results on MacBook:

```
case a with gilc
  3561092.83 +- 256499.85
case b with gilc
  3618501.08 +- 802961.93
case a with gilc_nogil
  2978816.49 +- 291618.80
case b with gilc_nogil
   281212.77 +- 233375.12
```

![Plot](plot.svg)
