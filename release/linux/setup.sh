#!/bin/sh
#---------------------------------------------------------------
#
# SimpleIDE user setup for Linux.
#
# This script assumes you have root access.
#
# To install SimpleIDE and Propeller GCC run: ./setup.sh
# This will copy Propeller GCC to /opt/parallax and Simple IDE
# to /opt/simpleide and create a shortcut for the executable
# /opt/simpleide/SimpleIDE in /usr/bin/simpleide 
#
# To start SimpleIDE use the command: simpleide
#
#---------------------------------------------------------------
#
# Copyright (c) 2016 by Parallax Inc.
# TERMS OF USE: MIT License (text at end of this file).
# 
#---------------------------------------------------------------

MYDIR=`pwd`
MYUSER=`echo $MYDIR | awk -F'/' '{print "/"$2"/"$3}'`

echo "$0 for: $MYUSER"

PROPGCC_DIR=/opt/parallax
SIMPLEIDE_DIR=/opt/simpleide

# Show usage
usage()
{
	echo "Usage: setup.sh [action [target]]"
	echo "Action:"
	echo "  help            Show this message"
	echo "  install         Install target"
	echo "  uninstall       Uninstall target"
	echo ""
	echo "Targets:"
	echo "  propellergcc    Set target to only Propeller GCC"
	echo "  simpleide       Set target to only SimpleIDE"
	echo ""
	echo "If the target is not given, both Propeller GCC and SimpleIDE will be installed."
}

# Uninstall Propeller GCC function
uninstall_propgcc()
{
	# Is Propeller GCC installed?
	if [ -e $PROPGCC_DIR ]; then
		read -p "Remove previously installed Propeller GCC? [y/N]:" response
		case $response in
			# Does the user really want to uninstall it?
			Y|y|YES|Yes|yes) echo "Removing Propeller GCC"
				rm -r $PROPGCC_DIR
			;;
			*) echo "Not removing Propeller GCC"
		esac
	else
		echo "Propeller GCC not installed"
	fi
}

# Install Propeller GCC function
install_propgcc()
{
	mkdir -p $PROPGCC_DIR
	echo "Installing Propeller GCC"
	cp -R parallax/* $PROPGCC_DIR
}

# Uninstall SimpleIDE function
uninstall_simpleide()
{
	if [ -e $SIMPLEIDE_DIR ] ; then
		read -p "Remove previously installed Simple IDE? [y/N]:" response
		case $response in
			# Does the user really want to uninstall it?
			Y|y|YES|Yes|yes) echo "Removing Simple IDE"
				rm -r $SIMPLEIDE_DIR
				rm /usr/bin/simpleide
			;;
			*) echo "Not removing Simple IDE"
		esac
	else
		echo "Simple IDE not installed"
	fi
}

# Install SimpleIDE function
install_simpleide()
{
	if [ -e $MYUSER/.config/ParallaxInc/SimpleIDE.conf ] ; then
		echo "Removing old properties"
		rm -f $MYUSER/.config/ParallaxInc/SimpleIDE.conf
	fi

	if [ -e $MYUSER/Documents/SimpleIDE/Learn/Simple\ Libraries/Text\ Devices ] ; then
		echo "Removing old Documents/SimpleIDE Text Devices library"
		rm -rf $MYUSER/Documents/SimpleIDE/Learn/Simple\ Libraries/Text\ Devices
	fi

	echo "Installing SimpleIDE"
	mkdir -p $SIMPLEIDE_DIR
	cp -R bin demos license translations $SIMPLEIDE_DIR
	cp $SIMPLEIDE_DIR/bin/simpleide.sh /usr/bin/simpleide

	# do this if simpleide.sh file permissions don't carry.
	chmod 755 /usr/bin/simpleide
}

# Verify the user is root function
if [ $(whoami) != root ]; then
	echo This script must be run as root
	echo try: sudo ./setup.sh
	exit
fi

# Check for arguments, show help, and exit if nothing specified.
if [ -z $1 ]
then
	usage
	exit
fi

# Execution starts here
# Did the user provide any commands?
case $1 in
	# Install something
	install)
		case $2 in
			# Install Propeller GCC
			propellergcc)
				install_propgcc
				exit
			;;
			# Install SimpleIDE
			simpleide)
				install_simpleide
				exit
			;;
			# Installation instructions unclear, show usage
			?*)
				echo Target \"$2\" not recognized
				usage
				exit
		esac
		# No specific uninstall instructions, install everything
		install_propgcc
		install_simpleide
		exit
	;;

	# Uninstall something
	uninstall)
		case $2 in
			# Uninstall Propeller GCC
			propellergcc)
				uninstall_propgcc
				exit
			;;
			# Uninstall SimpleIDE
			simpleide)
				uninstall_simpleide
				exit
			;;
			# Uninstallation instructions unclear, show usage
			?*)
				echo Target \"$2\" not recognized
				usage
				exit
		esac
		# No specific uninstall instructions, uninstall everything
		uninstall_propgcc
		uninstall_simpleide
		exit
	;;

	# Instructions unclear, show usage
	help)
		usage
		exit
	;;

	# Instructions unclear, show usage
	?*)
		echo Action \"$1\" not recognized
		usage
		exit
	;;
esac

# No instructions, install everything
install_propgcc
install_simpleide

exit

#+---------------------------------------------------------------------
#| TERMS OF USE: MIT License
#+---------------------------------------------------------------------
#Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
#
#The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
#
#THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#+---------------------------------------------------------------------
#| End of terms of use License Agreement.
#+---------------------------------------------------------------------
#
