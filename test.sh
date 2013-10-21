#!/bin/bash
echo "--- Building ghoard .so's"
./build.sh
echo "--- Building tracker's .so"
g++ --shared -std=c++0x -fPIC -g -o dist/debug/libghoard_tracker.so tracing.h tracing.cpp test/tracker.cpp
echo "--- Building and running tests"
find ./test -type d|while read dir; do
    if [ ! "$dir" == "./test" ]; then
    echo "------ Test from $dir" 
    files=$(ls "$dir"|grep -E '\.(h|cpp)$'|while read f; do echo "$dir/$f"; done)
    if [ ! "$files" = '' ]; then
        executable="$dir/run_test"
        g++ -std=c++0x -fPIC -g -o "$executable" $files -lghoard_no_intercept -Ldist/debug
        if [ -f "$executable" ]; then
            full_path_to_so=$(pwd)/dist/debug/libghoard_tracker.so
            LD_PRELOAD="$full_path_to_so" LD_LIBRARY_PATH=./dist/debug "$executable"
        fi
    fi
    fi
done
