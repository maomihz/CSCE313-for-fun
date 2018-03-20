#!/bin/sh
#  test2.sh
#  Created by Joshua Higginbotham on 11/8/15.

echo "Compiling..."
make
echo "Running tests..."
echo "Test 1, b = 1:" >> output2.txt
./client -n 10000 -b 1 -w 80 -m 2
echo "Test 2, b = 2:" >> output2.txt
./client -n 10000 -b 2 -w 80 -m 2
echo "Test 3, b = 4:" >> output2.txt
./client -n 10000 -b 4 -w 80 -m 2
echo "Test 4, b = 8:" >> output2.txt
./client -n 10000 -b 8 -w 80 -m 2
echo "Test 5, b = 16:" >> output2.txt
./client -n 10000 -b 16 -w 80 -m 2
echo "Test 6, b = 32:" >> output2.txt
./client -n 10000 -b 32 -w 80 -m 2
echo "Test 7, b = 64:" >> output2.txt
./client -n 10000 -b 64 -w 80 -m 2
echo "Test 8, b = 128:" >> output2.txt
./client -n 10000 -b 128 -w 80 -m 2
echo "Test 9, b = 256:" >> output2.txt
./client -n 10000 -b 256 -w 80 -m 2
echo "Test 10, b = 512:" >> output2.txt
./client -n 10000 -b 512 -w 80 -m 2
echo "Test 11, b = 1024:" >> output2.txt
./client -n 10000 -b 1024 -w 80 -m 2
echo "Test 12, b = 2048:" >> output2.txt
./client -n 10000 -b 2048 -w 80 -m 2
echo "Test 13, b = 8096:" >> output2.txt
./client -n 10000 -b 8096 -w 80 -m 2
echo "Test 14, b = 8192:" >> output2.txt
./client -n 10000 -b 8192 -w 80 -m 2
echo "Test 15, b = 16384:" >> output2.txt
./client -n 10000 -b 16384 -w 80 -m 2
echo "Finished!"
