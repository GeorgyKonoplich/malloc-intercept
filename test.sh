#!/bin/bash


no_compile=0
no_tcompile=0
no_bcompile=0
no_tests=0
no_benchmarks=0
no_standart_benchmark=0
enable_tracing=0

for arg in $@ ; do
    if [ "$arg" = "-nc" ]; then
        no_compile=1
    fi
    if [ "$arg" = "-nbc" ]; then
        no_bcompile=1
    fi
    if [ "$arg" = "-ntc" ]; then
        no_tcompile=1
    fi
    if [ "$arg" = "-nt" ]; then
        no_tests=1
    fi
    if [ "$arg" = "-nb" ]; then
        no_benchmarks=1
    fi
    if [ "$arg" = "-nsb" ]; then
        no_standart_benchmark=1
    fi
    if [ "$arg" = "-et" ]; then
        enable_tracing=1
    fi
done

if [ $no_tests = 0 ]; then
    if [ $no_tcompile = 0 ]; then
        if [ $no_compile = 0 ]; then
            echo "--- Building ghoard .so's"
            ./build.sh
        fi
        echo "--- Building tracker's .so"
        g++ --shared -std=c++0x -fPIC -g -o dist/debug/libghoard_tracker.so tracing.h tracing.cpp test/tracker.cpp
    fi
    echo "--- Building and running tests"
    find ./test -type d|while read dir; do
    if [ ! "$dir" == "./test" ]; then
        first_char=$(basename "$dir"|cut -c1)
        if [ ! $first_char = '_' ]; then
            echo "------ Test from $dir" 
            files=$(ls "$dir"|grep -E '\.(h|cpp)$'|while read f; do echo "$dir/$f"; done)
            if [ ! "$files" = '' ]; then
                executable="$dir/run_test"
                if [ $no_tcompile = 0 ]; then
                    g++ -std=c++0x -fPIC -g -o "$executable" $files -lghoard_no_intercept -Ldist/debug -lpthread
                fi
                if [ -f "$executable" ]; then
                    full_path_to_so=$(pwd)/dist/debug/libghoard_tracker.so
                    if [ $enable_tracing = 1 ]; then
                        GHOARD_RETURN_SUPERBLOCKS=1 GHOARD_TRACE=1 LD_PRELOAD="$full_path_to_so" LD_LIBRARY_PATH=./dist/debug "$executable"
                    else
                        GHOARD_RETURN_SUPERBLOCKS=1 LD_PRELOAD="$full_path_to_so" LD_LIBRARY_PATH=./dist/debug "$executable"
                    fi
                fi
            fi
        fi
    fi
done
fi
if [ $no_benchmarks = 0 ]; then
    echo -n '' > benchmark_results.txt
    if [ $no_bcompile = 0 ]; then
        if [ $no_compile = 0 ]; then
            echo "--- Building ghoard .so's"
            ./build.sh release
            if [ ! -f Hoard_by_author/libhoard.so ]; then
                cd Hoard_by_author
                make linux-gcc-x86-64
                cd ..
            fi
        fi
    fi
    find -L ./benchmark -type d|while read dir; do
    if [ ! "$dir" = "./benchmark" ]; then
        first_char=$(basename "$dir"|cut -c1)
        if [ ! $first_char = '_' ]; then
            files=$(ls "$dir"|grep -E '\.(h|cpp)$'|while read f; do echo "$dir/$f"; done)
            if [ ! "$files" = '' ]; then
                if [ -f "$dir/Makefile" ]; then
                    $files = '-'
                fi
            fi
            if [ -f "$dir/Makefile" ]; then
                if [ $no_bcompile = 0 ]; then
                    _d=$(pwd)
                    cd "$dir"
                    make 2>&1 > /dev/null
                    cd "$_d"
                fi
                executable="$dir/$(basename "$dir")"
            else
                if [ -f "$dir/main.cpp" ]; then
                    executable="$dir/run_benchmark"
                    if [ $no_bcompile = 0 ]; then
                        g++ -std=c++0x -fPIC -O2 -o "$executable" $files -lpthread
                    fi
                fi
            fi
            if [ -f "$executable" ]; then
                echo "------ Benchmark from $dir" | tee -a benchmark_results.txt
                full_path_to_so=$(pwd)/dist/release/libghoard.so
                T="$(date +%s%N)"
                LD_PRELOAD="$full_path_to_so" "$executable" 2>&1 >/dev/null
                M="$((($(date +%s%N)-T)/1000000))"
                echo "Hoard: $M millis" | tee -a benchmark_results.txt
                full_path_to_hoard_by_author_so=$(pwd)/dist/release/libghoard.so
                T="$(date +%s%N)"
                LD_PRELOAD="$full_path_to_hoard_by_author_so" "$executable" 2>&1 >/dev/null
                M="$((($(date +%s%N)-T)/1000000))"
                echo "Hoard by author: $M millis" | tee -a benchmark_results.txt
                if [ $no_standart_benchmark = 0 ]; then
                    T="$(date +%s%N)"
                    "$executable" 2>&1 >/dev/null
                    M="$((($(date +%s%N)-T)/1000000))"
                    echo "Standart: $M millis" | tee -a benchmark_results.txt
                fi
            fi
        fi
    fi
done
echo '' | tee -a benchmark_results.txt
fi
