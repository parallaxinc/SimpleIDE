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

PROPGCC_DIR=/opt/parallax
SIMPLEIDE_DIR=/opt/simpleide

# Show usage
usage()
{
	echo "Usage: setup.sh [action [target]]"
	echo "Action:"
	echo  "  install	Install target"
	echo "  uninstall	Uninstall target"
	echo ""
	echo "Targets:"
	echo "  propellergcc	Set target to only Propeller GCC"
	echo "  simpleide	Set target to only SimpleIDE"
	echo ""
	echo "If the target is not given, the default is both Propeller GCC and SimpleIDE"
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
			*) echo "Not modifying Propeller GCC"
		esac
	else
		echo "Propeller GCC not installed"
	fi
}

# Install Propeller GCC function
install_propgcc()
{
	uninstall_propgcc
	if [ -e $PROPGCC_DIR ]; then
		echo "Error: Propeller GCC already installed"
		exit
	fi

	echo "Installing Propeller GCC"
	mkdir -p $PROPGCC_DIR
	cp -R parallax/* $PROPGCC_DIR
}

# Uninstall SimpleIDE function
uninstall_simpleide()
{
	if [ -e $SIMPLEIDE_DIR ]; then
		read -p "Remove previously installed Simple IDE? [y/N]:" response
		case $response in
			# Does the user really want to uninstall it?
			Y|y|YES|Yes|yes) echo "Removing Simple IDE"
				rm -r $SIMPLEIDE_DIR
				rm /usr/bin/simpleide
			;;
			*) echo "Not modifying Simple IDE"
		esac
	else
		echo "Simple IDE not installed"
	fi
}

# Install SimpleIDE function
install_simpleide()
{
	uninstall_simpleide
	if [ -e $SIMPLEIDE_DIR ]; then
		echo "Error: Simple IDE already installed"
		exit
	fi

	echo "Installing SimpleIDE"
	mkdir -p $SIMPLEIDE_DIR
	cp -R bin demos license translations $SIMPLEIDE_DIR
	cp $SIMPLEIDE_DIR/bin/simpleide.sh /usr/bin/simpleide

	# do this if simpleide.sh file permissions don't carry.
	# chmod 755 /usr/bin/simpleide
}

# Verify the user is root function
if [ $(whoami) != root ]; then
	echo This script must be run as root
	echo try: sudo ./setup.sh
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
	?*)
		echo Action \"$1\" not recognized
		usage
		exit
esac

# No instructions, install everything
install_propgcc
install_simpleide

exit
