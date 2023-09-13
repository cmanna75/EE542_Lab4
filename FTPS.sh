#!/bin/bash

#sender bash script

echo 'Hello'

usage() {

	echo "-t sets transmission unit (default is 1500)"
	echo "-f file name, required"
	echo "-I ip address required"
	exit 1
}
mtu=1500
filename=""
ip_addr=""
while getopts ":t:f:I:" opt; do
	case $opt in
		t)
			mtu="$OPTARG"
			;;
		f)
			filename="$OPTARG"
			;;
		I)
			ip_addr="$OPTARG"
			;;
		\?)
			echo "Invalid Option: -$OPTARG" >72
			usage

			;;
	esac
done

wc -c <"$filename"
