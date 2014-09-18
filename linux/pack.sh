#!/bin/sh -x
#
# create a .deb package
#

FOLDERS=`echo "simpleide simpleide/usr simpleide/usr/bin simpleide/opt simpleide/opt/simpleide simpleide/opt/simpleide/bin"`

# Show usage
usage()
{
  echo "Usage: $0 <path-to-SimpleIDE>"
  exit 1
}

# build package
build()
{
  SPKG=`echo ${SRC_PACKAGE}`
  SPKG=`echo ${SPKG} | sed 's/\.*\///g'`
  echo "Building debian package for ${SPKG}"

  PROPGCC=${SRC_PACKAGE}/parallax
  if [ -e ${PROPGCC} ]; then
    echo "PropellerGCC found."
  else
    echo "Error ${SRC_PACKAGE}${PROPGCC} folder not found."
    exit 1
  fi

  echo "make simpleide folders"

  for DIR in ${FOLDERS} ; do
    mkdir -p $DIR
    if [ $? -ne 0 ]; then
      echo "Error making folder $DIR"
      exit 1
    fi
  done

  echo "Copy simpleide.sh"
  cp ${SRC_PACKAGE}/bin/simpleide.sh simpleide/usr/bin/simpleide
  if [ $? -ne 0 ]; then
    echo "Failure copying simpleide.sh"
    exit 1
  fi

  echo "Ensure executable"
  chmod u+x simpleide/usr/bin/simpleide
  if [ $? -ne 0 ]; then
    echo "Failure chmod u+x simpleide"
    exit 1
  fi

  echo "Copy SimpleIDE"
  cp ${SRC_PACKAGE}/bin/SimpleIDE simpleide/opt/simpleide/bin
  if [ $? -ne 0 ]; then
    echo "Failure copying SimpleIDE"
    exit 1
  fi

  echo "Copy propellergcc"
  cp -r ${PROPGCC} simpleide/opt/
  if [ $? -ne 0 ]; then
    echo "Failure copying propellergcc"
    exit 1
  fi

  echo "Copy License Info"
  cp -r ${SRC_PACKAGE}/license simpleide/opt/simpleide
  if [ $? -ne 0 ]; then
    echo "Failure copying license"
    exit 1
  fi

  echo "Copy Spin Library"
  cp -r ${PROPGCC}/spin simpleide/opt/simpleide
  if [ $? -ne 0 ]; then
    echo "Failure copying license"
    exit 1
  fi

  echo "Make .deb Package"
  #VER=`echo ${SRC_PACKAGE} | sed 's/^.*\([0-9].*\)-\([0-9].*\)/\1\.\2/g'`
  VER=`echo ${SRC_PACKAGE} | sed 's/^.*\([0-9].*\)-\([0-9].*\)-\([0-9].*\)/\1\.\2\.\3/g'`
  CPU=`uname -m`
  if [ ${CPU} = i686 ]; then
    CPU=i386
  fi
  if [ ${CPU} = x86_64 ]; then
    CPU=amd64
  fi
  mkdir -p simpleide/DEBIAN/
  cat debian_control | sed "s/VERSION/${VER}/g" > temp1
  cat temp1 | sed "s/CPU/${CPU}/g" > simpleide/DEBIAN/control
  rm -f temp1
  dpkg-deb -b simpleide simpleide-${VER}-${CPU}.deb
  if [ $? -ne 0 ]; then
    echo "Failure making package"
    exit 1
  fi
}

cleanup()
{
    for DIR in ${FOLDERS} ; do
      rm -rf $DIR
      if [ $? -ne 0 ]; then
        echo "Error removing $DIR"
        exit 1
      fi
    done
}

case $1 in

  clean)
    echo "Clean files"
    cleanup
  ;;

  ?*)
  if [ x${1} != "x" ]; then
    SRC_PACKAGE=$1
    build
  else
    usage
  fi
  ;;

esac


exit 0
