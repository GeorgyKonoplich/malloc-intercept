#!/bin/bash
mode='debug'

if [ "$1" = "release" ]; then mode='release'; fi
if [ "$1" = "r" ]; then mode='release'; fi

if [ ! -d "dist/$mode" ]; then mkdir "dist/$mode"; fi

if [ $mode = 'debug' ]; then
    g++ --shared -std=c++0x -fPIC -g -o dist/$mode/libghoard.so allocator.cpp allocator.h constants.h heap.cpp heap.h malloc-intercept.cpp superblock.cpp superblock.h tracing.cpp tracing.h utility.cpp utility.h
    g++ --shared -std=c++0x -fPIC -g -o dist/$mode/libghoard_no_intercept.so allocator.cpp allocator.h constants.h heap.cpp heap.h superblock.cpp superblock.h utility.cpp utility.h tracing.h tracing.cpp
else
    g++ -Wno-unused-result --shared -std=c++0x -fPIC -O2 -o dist/$mode/libghoard.so allocator.cpp allocator.h constants.h heap.cpp heap.h malloc-intercept.cpp superblock.cpp superblock.h tracing.cpp tracing.h utility.cpp utility.h
    g++ -Wno-unused-result --shared -std=c++0x -fPIC -O2 -o dist/$mode/libghoard_no_intercept.so allocator.cpp allocator.h constants.h heap.cpp heap.h superblock.cpp superblock.h utility.cpp utility.h tracing.h tracing.cpp
fi
