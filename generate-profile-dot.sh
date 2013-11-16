#!/bin/bash
dir=$(pwd)
g++ -Wno-unused-result --shared -std=c++0x -fPIC -O2 -o dist/release/libghoard_for_profiler.so allocator.cpp allocator.h constants.h constants.cpp heap.cpp heap.h malloc-intercept.cpp superblock.cpp superblock.h tracing.cpp tracing.h utility.cpp utility.h -lprofiler -L.
if [ ! -d profile-results ]; then
    mkdir profile-results
fi

find -L ./benchmark -type d|while read dir; do
if [ ! "$dir" = "./benchmark" ]; then
    first_char=$(basename "$dir"|cut -c1)
    if [ ! $first_char = '_' ]; then
        files=$(ls "$dir"|grep -E '\.(c|hpp|h|cpp)$'|while read f; do echo "$dir/$f"; done)
        if [ -f "$dir/main.cpp" ]; then
            executable="$dir/run_benchmark"
            g++ -std=c++0x -fPIC -O2 -o "$executable" $files -lpthread -lprofiler -L.
        fi
        if [ -f "$executable" ]; then
            echo "------ Benchmark from $dir"
            full_path_to_so=$(pwd)/dist/release/libghoard.so
            CPUPROFILE=/tmp/profile LD_PRELOAD="/usr/lib/libprofiler.so.0.1.0 $full_path_to_so" "$executable" 2>&1 >/dev/null
            google-pprof --dot "$executable" /tmp/profile > "profile-results/$(basename "$dir").dot"
        fi
    fi
fi
done

