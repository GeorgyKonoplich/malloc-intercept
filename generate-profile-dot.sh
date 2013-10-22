#!/bin/bash
dir=$(pwd)
g++ -Wno-unused-result --shared -std=c++0x -fPIC -O2 -o dist/release/libghoard.so allocator.cpp allocator.h constants.h heap.cpp heap.h malloc-intercept.cpp superblock.cpp superblock.h tracing.cpp tracing.h utility.cpp utility.h -lprofiler -L.
g++ -std=c++0x -fPIC -O2 -o benchmark/simple_multithread/run_benchmark  benchmark/simple_multithread/main.cpp -lpthread -lprofiler -L.
CPUPROFILE=/tmp/profile LD_PRELOAD="/usr/lib/libprofiler.so.0.1.0 $dir/dist/release/libghoard.so" benchmark/simple_multithread/run_benchmark 
google-pprof --dot benchmark/simple_multithread/run_benchmark /tmp/profile > profile.dot

