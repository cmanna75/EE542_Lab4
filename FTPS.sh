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

filesize=$(wc -c <"$filename")
echo "filesize: $filesize (B)"
start_time=`date +%s%N`
./client
end_time=`date +%s%N`
total_time_ns=$((end_time - start_time))
total_time=$(bc <<< "scale=6; $total_time_ns / 1000000000")
throughput=$(bc <<< "scale=6; $filesize / ($total_time * 1024 * 1024)")
echo "total time: $total_time (s)" 
echo "throughput:  $throughput (MBps)"
