#!/bin/bash

#receiver bash script!

echo 'Hello'

#run selected file TODO: add command line inputs to file if needed

./server 8080 data_0.bin &
./server 8081 data_1.bin &
./server 8082 data_2.bin &
./server 8083 data_3.bin &

wait

cat data_0.bin  data_1.bin  data_2.bin  data_3.bin > data.bin
echo 'File Transfer Complete'
