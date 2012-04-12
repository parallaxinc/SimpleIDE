#!/bin/sh
appname=SimpleIDE
dirname=$PWD/bin/

LD_LIBRARY_PATH=$dirname
export LD_LIBRARY_PATH

`$dirname/$appname "$@"` &

