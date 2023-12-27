#!/bin/bash
find samples elvis -type f -name "*.cpp" -o -name "*.h" | while read i ; do
    echo formatting $i …
    clang-format -style=file -i "$i"
done