#!/bin/bash

killall serial_server

#./serial_server /dev/ttyS7 115200 -fixture -hfcnt -v & 
./serial_server /dev/ttyS7 115200 -fixture -v & 
./serial_server /dev/ttyS6 115200 -fixture -v & 
./serial_server /dev/ttyS5 115200 -fixture -v & 
./serial_server /dev/ttyS4 115200 -rs485 -fixture -v &

./serial_server /dev/ttyUSB0 115200 -0 &		
./serial_server /dev/ttyUSB1 115200 -0 &		
./serial_server /dev/ttyUSB2 115200 -0 &		
#EOF
