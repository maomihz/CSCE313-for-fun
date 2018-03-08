#!/bin/sh
#  test.sh
#  Created by Joshua Higginbotham on 11/8/15.

# This is only provided for your convenience.
# The tests used in this file may or may not
# match up with what is called for for the report,
# so be sure to read that part of the handout.
# But you're free to modify this script however
# you need to, it's not graded.

echo "Cleaning and compiling..."
make
echo "Running tests..."
echo "Test 0, w = 1:" >> output.txt
./client -n 10000 -w 1
echo "Test 1, w = 5:" >> output.txt
./client -n 10000 -w 5
echo "Test 2, w = 10:" >> output.txt
./client -n 10000 -w 10
echo "Test 3, w = 15:" >> output.txt
./client -n 10000 -w 15
echo "Test 4, w = 20:" >> output.txt
./client -n 10000 -w 20
echo "Test 5, w = 25:" >> output.txt
./client -n 10000 -w 25
echo "Test 6, w = 30:" >> output.txt
./client -n 10000 -w 30
echo "Test 7, w = 35:" >> output.txt
./client -n 10000 -w 35
echo "Test 8, w = 40:" >> output.txt
./client -n 10000 -w 40
echo "Test 9, w = 45:" >> output.txt
./client -n 10000 -w 45
echo "Test 10, w = 50:" >> output.txt
./client -n 10000 -w 50
echo "Test 11, w = 55:" >> output.txt
./client -n 10000 -w 55
echo "Test 12, w = 60:" >> output.txt
./client -n 10000 -w 60
echo "Test 13, w = 65:" >> output.txt
./client -n 10000 -w 65
echo "Test 14, w = 70:" >> output.txt
./client -n 10000 -w 70
echo "Test 15, w = 75:" >> output.txt
./client -n 10000 -w 75
echo "Test 16, w = 80:" >> output.txt
./client -n 10000 -w 80
echo "Test 17, w = 85:" >> output.txt
./client -n 10000 -w 85
echo "Test 18, w = 90:" >> output.txt
./client -n 10000 -w 90
echo "Test 19, w = 95:" >> output.txt
./client -n 10000 -w 95
echo "Test 20, w = 100:" >> output.txt
./client -n 10000 -w 100
echo "Test 21, w = 105:" >> output.txt
./client -n 10000 -w 105
echo "Test 22, w = 110:" >> output.txt
./client -n 10000 -w 110
echo "Test 23, w = 115:" >> output.txt
./client -n 10000 -w 115
echo "Test 24, w = 120:" >> output.txt
./client -n 10000 -w 120
echo "Test 25, w = 125:" >> output.txt
./client -n 10000 -w 125
echo "Test 26, w = 130:" >> output.txt
./client -n 10000 -w 130
echo "Test 27, w = 135:" >> output.txt
./client -n 10000 -w 135
echo "Test 28, w = 140:" >> output.txt
./client -n 10000 -w 140
echo "Test 29, w = 145:" >> output.txt
./client -n 10000 -w 145
echo "Test 30, w = 150:" >> output.txt
./client -n 10000 -w 150
echo "Test 31, w = 155:" >> output.txt
./client -n 10000 -w 155
echo "Test 32, w = 160:" >> output.txt
./client -n 10000 -w 160
echo "Test 33, w = 165:" >> output.txt
./client -n 10000 -w 165
echo "Test 34, w = 170:" >> output.txt
./client -n 10000 -w 170
echo "Test 35, w = 175:" >> output.txt
./client -n 10000 -w 175
echo "Test 36, w = 180:" >> output.txt
./client -n 10000 -w 180
echo "Test 37, w = 185:" >> output.txt
./client -n 10000 -w 185
echo "Test 38, w = 190:" >> output.txt
./client -n 10000 -w 190
echo "Test 39, w = 195:" >> output.txt
./client -n 10000 -w 195
echo "Test 40, w = 200:" >> output.txt
./client -n 10000 -w 200
echo "Test 41, w = 205:" >> output.txt
./client -n 10000 -w 205
echo "Test 42, w = 210:" >> output.txt
./client -n 10000 -w 210
echo "Test 43, w = 215:" >> output.txt
./client -n 10000 -w 215
echo "Test 44, w = 220:" >> output.txt
./client -n 10000 -w 220
echo "Test 45, w = 225:" >> output.txt
./client -n 10000 -w 225
echo "Test 46, w = 230:" >> output.txt
./client -n 10000 -w 230
echo "Test 47, w = 235:" >> output.txt
./client -n 10000 -w 235
echo "Test 48, w = 240:" >> output.txt
./client -n 10000 -w 240
echo "Test 49, w = 245:" >> output.txt
./client -n 10000 -w 245
echo "Test 50, w = 250:" >> output.txt
./client -n 10000 -w 250
echo "Finished!"
