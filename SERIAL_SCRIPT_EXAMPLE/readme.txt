Serial test example
===================

These programs provide a method to test RS232/RS485/RS422 across a number of baud rates.

They were developed on an ISIS where the port configuration is port0 : RS232 and port1 : RS232/RS485/RS422.

The unit under test (UUT) communicates to an external fixture. Char sent to the external fixture is echoed back.

Currently I've named these programs,

	serial_client - serial data test, this runs on the UUT, transmits and compares received data
	serial_hs - handshake test, this runs on the UUT, toggles RTS/DTR and reads CTS and DCD/DSR/RI
	serial_server - this runs on the test fixture, echoes received data back to the client and toggles handshake lines
	io-rmw - simple IO for controlling serial port configuration

End.

	 



 
 



