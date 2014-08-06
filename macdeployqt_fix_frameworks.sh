#!/bin/sh --
#
# macdeployqt_fix_frameworks.sh
#
# A shell script based on the ruby script: macdeployqt_fix_frameworks.rb
#
# macdeployqt_fix_frameworks.rb author: Kevin Wojniak (https://gist.github.com/kainjow)
# GitHub: https://gist.github.com/kainjow/8059407
#
# Copies missing Info.plist files for a .app's Qt frameworks installed
# from macdeployqt. Without the plists, 'codesign' fails on 10.9 with
# "bundle format unrecognized, invalid, or unsuitable".
#
# Assumes the app bundle is within a directory with the app's base name 
#
# Example usage:
# ./macdeployqt_fix_frameworks.sh -p ~/Qt/5.3/clang_64/lib -n MyProgram.app
#
# my defaults 
#QT_LIBS_DIR=/Volumes/Developer/Qt/5.3/clang_64/lib
NAME=SimpleIDE

usage()
{
cat << EOF
usage: $0 options

This script copies missing Info.plist files for an .app's Qt frameworks 
installed by macdeployqt. Without the plists, 'codesign' fails on 10.9
with "bundle format unrecognized, invalid, or unsuitable".

OPTIONS:
    -h          show usage
    -p path     path to Qt SDK's libraries  - example: "-p ~/Qt/5.3/clang_64/lib" (required)
    -n name     app name                    - example: "-a SimpleIDE" (default)
    -?          show usage
EOF
}

while getopts "h:p:n:?" OPTION
do
    case $OPTION in
        h)
            usage
            exit 0
            ;;
        p)
            QT_LIBS_DIR=$OPTARG
            ;;
        n)
            NAME=$OPTARG
            ;;
        ?)
            usage
            exit 0
            ;;
    esac
done

# must have a path and an app name
if [[ -z $QT_LIBS_DIR ]] || [[ -z $NAME ]]
then
     usage
     exit 1
fi

# Qt SDK's library path must exist
if [[ ! -d $QT_LIBS_DIR ]]
then
    echo $QT_LIBS_DIR directory does not exist!
    echo
    usage
    exit 1
fi

# destination
APP=${NAME}.app
APP_FWRK_PATH=${NAME}/${APP}/Contents/Frameworks

# for each Qt framework in the app bundle copy an Info.plist from the Qt SDK's libraries of the same name
for APP_QT_FWRK_PATH in ${APP_FWRK_PATH}/*
do
#   split-out the framework's name from its path
    FRAMEWORK_NAME=$(basename ${APP_QT_FWRK_PATH})

#   Create a Qt framework Resources directory if none exists in the app's bundle 
    if [ ! -d ${APP_QT_FWRK_PATH}/Resources ]
        then
            mkdir -p ${APP_QT_FWRK_PATH}/Resources            
            if [ "$?" != "0" ]; then
                echo "[Error] failure creating ${APP_QT_FWRK_PATH}/Resources directory" 1>&2
                exit 1
            fi
            echo "created: ${APP_QT_FWRK_PATH}/Resources"
    fi

#   copy the Info.plist from from Qt SDK's libraries to the framework's Resource directory within the app bundle
    if [ -f ${QT_LIBS_DIR}/${FRAMEWORK_NAME}/Contents/Info.plist ]
        then
            echo "copying: ${FRAMEWORK_NAME} Info.plist to: ${APP_QT_FWRK_PATH}/Resources"
            cp ${QT_LIBS_DIR}/${FRAMEWORK_NAME}/Contents/Info.plist ${APP_QT_FWRK_PATH}/Resources
            if [ "$?" != "0" ]; then
                echo "[Error] copy failed!" 1>&2
                exit 1
            fi
        else
            echo "no Info.plist exists in: ${QT_LIBS_DIR}/${FRAMEWORK_NAME}/Contents/"
            exit 1
    fi
done
exit 0

