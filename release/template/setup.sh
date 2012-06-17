#!/bin/sh
#---------------------------------------------------------------
#
# SimpleIDE user setup for linux.
#
# This script assumes you don't have root access.
# A separate package manager can be used for root.
#
#---------------------------------------------------------------
#
STARTPRG=./simpleide
SIDEPROG=./bin/SimpleIDE
CTAGPROG=./bin/ctags
TEMPLATE=./bin/template.sh
PRODNAME=parallax
PRGCCBIN=/opt/${PRODNAME}/bin

#
# if we don't have simpleide name here,
# do maintenance and copy script.
#
if [ ! -e $STARTPRG ]; then
    if [ -r ./${PRODNAME} ]; then
        mkdir -p /opt
        cp -rf ./${PRODNAME} /opt
    fi
    if [ -e $PRGCCBIN ]; then
        echo "Found Users $PRGCCBIN"
        cp -f $CTAGPROG $PRGCCBIN
        cp $TEMPLATE $STARTPRG
        chmod u+x $STARTPRG
        echo "Setup complete. To run program use: $STARTPRG"
    else
        echo "$PRGCCBIN not found. Please install Propeller-GCC and run setup.sh again."
        exit 1
    fi
    exit 0
fi
