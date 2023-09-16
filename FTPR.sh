#!/bin/bash

#receiver bash script!

echo 'Hello'

usage() {

	echo "-t sets transmission unit (default is 1500)"
	echo "-f file name, defualt: data.bin"
	exit 1
}
mtu=1500
filename="data.bin"
while getopts ":t:f:I:" opt; do
	case $opt in
		t)
			mtu="$OPTARG"
			;;
		f)
			filename="$OPTARG"
			;;
		\?)
			echo "Invalid Option: -$OPTARG" >72
			usage

			;;
	esac
done

./receiver 8080 0 &
./receiver 8081 1 &
./receiver 8082 2 &
./receiver 8083 3 &

wait

cat r0.data.bin  r1.data.bin  r2.data.bin  r3.data.bin > $filename
rm r0.data.bin  r1.data.bin  r2.data.bin  r3.data.bin
echo 'File Transfer Complete'
