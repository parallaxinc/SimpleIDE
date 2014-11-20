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
# Updates:
#
#   11/18/2014  -   modified to patch each framework's directory organization
#                   to the Apple prescribed format
#
#       Framework Directory Organization Example:
#
#       QtCore.framework/
#           QtCore    --> Versions/Current/QtCore
#           Resources --> Versions/Current/Resources
#           Versions/
#               Current --> 5
#               5/
#                   QtCore
#                   Resources/
#                       Info.plist
#
#
# Assumes the app bundle is within a directory with the app's base name 
#
# Using sudo ./macdeploy_fix_frameworks.sh is strongly recommended
#
# Example usage:
# sudo ./macdeployqt_fix_frameworks.sh -p ~/Qt/5.3/clang_64/lib -n SimpleIDE
#
# defaults 
NAME=SimpleIDE

usage()
{
cat << EOF
usage: $0 options

This script copies missing Info.plist files for an .app's Qt frameworks 
installed by macdeployqt. Without the plists, 'codesign' fails on 10.9
with "bundle format unrecognized, invalid, or unsuitable". The script
also patches the organization of the fraemwork directories to conform to
the Apple-prescribed format.

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

# starting path
START_FULL_PATH="$PWD"
# echo "Starting at: ${START_FULL_PATH}"

# destination path
APP=${NAME}.app
APP_FWRK_PATH=${NAME}/${APP}/Contents/Frameworks

# for each Qt framework in the app bundle:
#   1. patch the framework's directory to spec for codesigning
#   2. copy an Info.plist from the Qt SDK's libraries of the same name
#
for APP_QT_FWRK_PATH in ${APP_FWRK_PATH}/*
do
#   get the full path to the framework
    cd "${START_FULL_PATH}/${APP_QT_FWRK_PATH}"
    APP_QT_FWRK_FULL_PATH="$PWD"
#   echo "Framework full path: ${APP_QT_FWRK_FULL_PATH}"

#   split-out the framework's name from its path
    FRAMEWORK_NAME=$(basename ${APP_QT_FWRK_PATH})
#   echo "Framework name: ${FRAMEWORK_NAME}"

#   split-out the framework's Version number
    FRAMEWORK_VERSION_NUMBER=$(basename ${APP_QT_FWRK_FULL_PATH}/Versions/*)
#   echo "Framework version: ${FRAMEWORK_VERSION_NUMBER}"

#   split-out the framework's binary file name
    FRAMEWORK_BINARY_NAME=$(basename ${APP_QT_FWRK_FULL_PATH}/Versions/${FRAMEWORK_VERSION_NUMBER}/*)
#   echo "Framework binary name: ${FRAMEWORK_BINARY_NAME}"

#   only copy Info.plist files if we are sure the framework is a directory
    if [ -d ${START_FULL_PATH}/${APP_QT_FWRK_PATH} ]
    then
#       verify that this is a true framework bundle and not a single-file
        if [ -d ${START_FULL_PATH}/${APP_QT_FWRK_PATH}/Versions ]
        then

            echo "Patching ${FRAMEWORK_NAME}:"
#           check if framework directory has already been patched
            if [[ -d ${START_FULL_PATH}/${APP_QT_FWRK_PATH}/Versions/${FRAMEWORK_VERSION_NUMBER}/Resources ]] \
                && [[ -h ${START_FULL_PATH}/${APP_QT_FWRK_PATH}/${FRAMEWORK_BINARY_NAME} ]] \
                && [[ -h ${START_FULL_PATH}/${APP_QT_FWRK_PATH}/Resources ]] \
                && [[ -h ${START_FULL_PATH}/${APP_QT_FWRK_PATH}/Versions/Current ]]
            then
                echo "WARNING: link to ${FRAMEWORK_BINARY_NAME} binary already exists. Script may have been run before"
                echo "         verify app framework content before resuming further processes..."
            else

#           Re-format the Framework's directory structure

#               move Resources directory to the directory linked to Current
                cd ${APP_QT_FWRK_FULL_PATH}
                mv Resources Versions/${FRAMEWORK_VERSION_NUMBER}/
                if [ "$?" != "0" ]; then
                    echo "[Error] moving Resources directory failed!" 1>&2
                    exit 1
                fi
                echo " - moved ${FRAMEWORK_NAME}/Resources directory to ${FRAMEWORK_NAME}/Versions/${FRAMEWORK_VERSION_NUMBER}/"

#               create the 'Current' link to a Version directory
                cd ${APP_QT_FWRK_FULL_PATH}/Versions/
                ln -s "${FRAMEWORK_VERSION_NUMBER}" "Current"
                if [ "$?" != "0" ]; then
                    echo "[Error] 'Current' link creation failed!" 1>&2
                    exit 1
                fi
                echo " - created ${FRAMEWORK_NAME}/Versions/${FRAMEWORK_VERSION_NUMBER}/Current --> ${FRAMEWORK_NAME}/Versions/${FRAMEWORK_VERSION_NUMBER}"

#               create a link to the Resource directory
                cd ${APP_QT_FWRK_FULL_PATH}
                ln -s "Versions/${FRAMEWORK_VERSION_NUMBER}/Resources" "Resources"
                    if [ "$?" != "0" ]; then
                    echo "[Error] 'Resources' link creation failed!" 1>&2
                    exit 1
                fi
                echo " - created ${FRAMEWORK_NAME}/Resources --> ${FRAMEWORK_NAME}/Versions/${FRAMEWORK_VERSION_NUMBER}/Resources"

#               create a link to the framework's binary
                cd ${APP_QT_FWRK_FULL_PATH}
                ln -s "Versions/${FRAMEWORK_VERSION_NUMBER}/${FRAMEWORK_BINARY_NAME}" "${FRAMEWORK_BINARY_NAME}"
                if [ "$?" != "0" ]; then
                    echo "[Error] ${FRAMEWORK_BINARY_NAME} link creation failed!" 1>&2
                    exit 1
                fi
                echo " - created ${FRAMEWORK_NAME}/${FRAMEWORK_BINARY_NAME} --> ${FRAMEWORK_NAME}/Versions/${FRAMEWORK_VERSION_NUMBER}/${FRAMEWORK_BINARY_NAME}"
            fi

        else
            echo "${APP_QT_FWRK_PATH}/Versions directory is missing. Framework is incomplete"
            exit 1
        fi

#       Copy the Info.plist from from Qt SDK's libraries to the framework's Resource directory within the app bundle

        if [ -f ${QT_LIBS_DIR}/${FRAMEWORK_NAME}/Contents/Info.plist ]
        then
            cp ${QT_LIBS_DIR}/${FRAMEWORK_NAME}/Contents/Info.plist ${START_FULL_PATH}/${APP_QT_FWRK_PATH}/Resources
            if [ "$?" != "0" ]; then
                echo "[Error] copy failed!" 1>&2
                exit 1
            fi
            echo " - copied: Info.plist to: ${FRAMEWORK_NAME}/Resources"
        else
            echo "no Info.plist exists in: ${QT_LIBS_DIR}/${FRAMEWORK_NAME}/Contents/"
            exit 1
        fi
    else
        echo "WARNING: ${FRAMEWORK_NAME} may not belong in ${APP_FWRK_PATH}"
        echo "         verify app framework content before resuming further processes..."

    fi
done
exit 0

