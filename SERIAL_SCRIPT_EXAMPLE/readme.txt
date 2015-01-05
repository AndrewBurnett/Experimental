Serial test example
===================

These programs provide a method to test RS232/RS485/RS422 across a number of baud rates.

They were developed on an ISIS where the port configuration is 
    port0 : RS232
    port1 : RS232/RS485/RS422.

The unit under test (UUT) communicates to an external fixture. Char sent to the external fixture is echoed back.
The external fixture is an ALUDRA, with AIM104-COM4 and AIM104-RELAY8. The COM4 has 2xRS232, 1xRS485, 1xRS422 and
provides the serial test interface. The RELAY8 switches and provides isolation between RS232 and RS485/RS422.  

These test programs have been optimized for ISIS and the ALUDRA test fixture, but porting to different hardware 
should be relatively simple, see the program descriptions below.

Currently I've named these programs,

serial_client - serial data test, this runs on the UUT, transmits and compares received data.
serial_hs - handshake test, this runs on the UUT, toggles RTS/DTR and reads CTS and DCD/DSR/RI
serial_server - this runs on the test fixture, echoes received data back to the client and toggles handshake lines
io-rmw - simple IO for controlling serial port configuration

RS232_TEST - example test script for system validation.

serial_client
=============

Control script to run on the UUT.

    run
        Setup and run data test across different baud rates on an ISIS. This requires the ALUDRA test fixture to 
        switch between serial interface types on COM2/ttyS1.

    run_rs232/run_rs422/run_rs485
        Setup and run an individual serial test on an ISIS. 
        These are for use with USB test cables, when the ALUDRA test fixture is not avaialable. 

    serialinit.sh
        Script to initialise the UUT serial ports.

    set_serial.sh
        Script to provide control for the ISIS serial ports. This script should be modified for different board types.

    baudtest.sh
        Baud rate test script.

    serial_client
        C program to control communication.    

    test{n}.txt
        Serial test data files.
        
    com2_cntrl.sh
        ISIS COM2 control utility script.


serial_server
=============

Listen for data on test ports, respond to test commands, echo data back to UUT.

    run
        Setup serial server to run on ALUDRA test fixture. This will have to be modified for different test hardware.

    serialinit.sh
        Setup serial ports on ALUDRA tst fixture, it will require modification for different hardware.

    serial_server
        C program to control serial communication.


serial_hs
=========

Handshake tst to run on UUT. This requires some of the utilities from serial_client and serial_server provides support.
The handshake test expects the input to inverted, e.g. RTS=1, CTS=0. 

    run
        Setup and run the handshake test on the ISIS.    

    serial_hs
        C program to control serial handshaking.

io-rmw
======

Simple utility to read and write to IO.

End.

	 



 
 



