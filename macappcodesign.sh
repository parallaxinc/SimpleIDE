#!/bin/sh --
#
# macappcodesign.sh
#
# A shell script that codesigns the SimpleIDE application
#
# Assumes the app bundle is within a directory with the app's base name 
#
# Example usage:
# ./macappcodesign -s "Developer ID Application" -a SimpleIDE/SimpleIDE.app
#
# defaults 
IDENTITY="Developer ID Application"
APPNAME=SimpleIDE

usage()
{
cat << EOF
usage: $0 options

codesigns a Mac application (defaults to SimpleIDE.app)

OPTIONS:
    -h          show usage
    -s identity developer certificate key   - example: -s "Developer ID Application" (default)
    -a name     app name                    - example: -a SimpleIDE (default)
    -?          show usage
EOF
}

while getopts "h:s:a:?" OPTION
do
    case $OPTION in
        h)
            usage
            exit 0
            ;;
        s)
            IDENTITY=$OPTARG
            ;;
        a)
            APPNAME=$OPTARG
            ;;
        ?)
            usage
            exit 0
            ;;
    esac
done

# must have an identity and an app name
if [[ -z $IDENTITY ]] || [[ -z $APPNAME ]]
then
     usage
     exit 1
fi

# App path must exist
if [[ ! -d ${APPNAME}/${APPNAME}.app ]]
then
    echo ${APPNAME}.app does not exist!
    echo
    usage
    exit 1
fi

# use security utility to determine if the identity is valid
#SECUREID=""
SECUREID=`security find-certificate -c "$IDENTITY" | grep labl`
if [[ -n ${SECUREID} ]]
then
    echo "Identity: \"${IDENTITY}\" found..."
else
    echo Identity: \"${IDENTITY}\" does not exist!
    echo
    usage
    exit 1
fi

# full app path
APPPATH=${APPNAME}/${APPNAME}.app

# attempt codesigning of the app bundle
echo code signing the app bundle @: ${APPPATH} for identity: \"${IDENTITY}\"
codesign -s "$IDENTITY" --deep -f -v ${APPPATH}
if [ "$?" != "0" ]; then
    echo "[Error] codesign failed!" 1>&2
    exit 1
fi

exit 0

