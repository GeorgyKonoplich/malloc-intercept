#GHoard - Hoard allocator implementation

Run ```./build.sh``` to build .so

Run ```./test.sh -nt``` to build .so and run benchmarks

Benchmark results:

```
------ Benchmark from ./benchmark/sorokin-malloc-testing/blowup
Hoard: 461 millis
Hoard by author: 446 millis
Standart: 360 millis
------ Benchmark from ./benchmark/sorokin-malloc-testing/random-alloc
Hoard: 199 millis
Hoard by author: 191 millis
Standart: 146 millis
------ Benchmark from ./benchmark/simple_multithread
Hoard: 1398 millis
Hoard by author: 1404 millis
Standart: 1366 millis

```
