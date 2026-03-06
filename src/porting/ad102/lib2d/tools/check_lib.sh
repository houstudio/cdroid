#!/bin/bash

CC=$1
lib=$2

result=`$CC -print-file-name=$lib`

if [ "$result" = "$lib" ]; then
    echo n
else
    echo y
fi
