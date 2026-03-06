#!/bin/bash

if [ "$1" == "" ]; then
config_in=.config.in
else
config_in=$1
fi

if ! [ -f .config.in ]; then
    touch .config.in
fi

if ! [ -f config.h ]; then
    touch config.h
fi

tools/configparser --input Config.in ${config_in} --defconfig .config.in.tmp --header config.h.tmp > /dev/null

if ! [ -f .config.in.tmp ]; then
    echo generate config.in failed
    rm -f .config.in
    exit 1
fi

if ! [ -f config.h.tmp ]; then
    echo generate config.h failed
    rm -f config.h
    exit 1
fi

# 检查 .config.in 是否有更新
diff .config.in .config.in.tmp > /dev/null
if [ $? != "0" ];then
    mv .config.in.tmp .config.in
else
    rm .config.in.tmp
fi

# 检查 config.h 是否有更新
diff config.h config.h.tmp > /dev/null
if [ $? != "0" ];then
    mv config.h.tmp config.h
else
    rm config.h.tmp
fi
