#!/bin/sh

prefix=""

if [ "$1" == "-b" ]; then
    prefix="bear --append -- "
fi

make prefix="$prefix" -f Makefile.engine
make prefix="$prefix" -f Makefile.testbed
