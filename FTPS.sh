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
echo "splitting files"
split -n 4 -a 1 "$filename" output_part_
echo "sending files" 
start_time=`date +%s%N`
./sender $ip_addr 8080 0 output_part_a &
./sender $ip_addr 8081 1 output_part_b &
./sender $ip_addr 8082 2 output_part_c &
./sender $ip_addr 8083 3 output_part_d &
wait
end_time=`date +%s%N`
rm output_part_*
echo "filesize: $filesize (B)"
total_time_ns=$((end_time - start_time))
total_time=$(bc <<< "scale=6; $total_time_ns / 1000000000")
throughput=$(bc <<< "scale=6; ($filesize * 8) / ($total_time * 1024 * 1024)")
echo "total time: $total_time (s)" 
echo "throughput:  $throughput (Mbps)"

