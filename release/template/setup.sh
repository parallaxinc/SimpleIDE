#!/bin/sh
#---------------------------------------------------------------
#
# SimpleIDE user setup for linux.
#
# This script assumes you have root access.
#
# To install simpleide and propeller-gcc enter: ./setup.sh $USER
# This will copy the toolchain from the package to /opt/parallax
# and create the script ./simpleide with $USER as the owner.
#
# To start SimpleIDE enter: ./simpleide
# or click the program to start it from your file manager.
#
#---------------------------------------------------------------
#
STARTPRG=./simpleide
SIDEPROG=./bin/SimpleIDE
CTAGPROG=./bin/ctags
TEMPLATE=./bin/template.sh
PRODNAME=parallax
PRGCCBIN=/opt/${PRODNAME}/bin

THISUSER=root
if [ x$1 != x ]; then
    THISUSER=$1
fi
echo "Installing ./simpleide as ${THISUSER}"

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
        chown $THISUSER $STARTPRG
        echo "Setup complete. To run program use: $STARTPRG"
    else
        echo "$PRGCCBIN not found. Please install Propeller-GCC and run setup.sh again."
        exit 1
    fi
    exit 0
fi
