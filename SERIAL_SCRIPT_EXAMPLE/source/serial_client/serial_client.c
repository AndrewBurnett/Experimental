/*
 * serial_client
 * Serial test control program. Writes a test string to an external unit 
 * and reads string back. 
 *
 * Expects serial_server to be running on remote unit, local loopback also works.
 * 
 * Implements manual control of RTS for RS485 (does not expect Auto RTS)
 * RS485/422/232 setup to be handled externally.
 *
 * The original location of this source (v0.1 9/17/01) is
 * http://www.embeddedlinuxinterfacing.com/chapters/06/querySerial.c
 * Copyright (C) 2001 by Craig Hollabaugh as set out under the GPLv2
 * Please do not contact the original author of this utility for support.
 *
 * gcc serial_client.c -o serial_client 
 *
 * Taken from queryserial. Queryserial used pthread, and gcc library -lpthread, to stop test.
 * There are some threading issues/effects noticed with serial_server, for instance serial_server
 * printf do not always appear when expected, they sometimes seem out of sync. The pthread stuff 
 * has been kept for reference. 
 * 
 * Version
 * 1.1	AB 21/11/14
 * - Initial release
*/

#include <stdio.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>
#include <stdlib.h>
//#include <pthread.h>
#include <string.h>
#include <unistd.h>

// Global variables
struct termios tio;
long int timeout;
int delay = 0;
long temp =0;
int verbose=0;
int hardware_control=0;

// Part of queryserial pthread control
void *stop_test (void *unused)
{
	usleep (timeout);
	exit(0);
	return NULL;
}

// Set RTS for RS485 control
int setRTS(int fd, int level)
{
    int status;

    if (ioctl(fd, TIOCMGET, &status) == -1) {
		perror("setRTS(): TIOCMGET");
        return 0;
    }
    if (level)
        status |= TIOCM_RTS;
    else
        status &= ~TIOCM_RTS;
    if (ioctl(fd, TIOCMSET, &status) == -1) {
        perror("setRTS(): TIOCMSET");
        return 0;
    }
    return 1;
}

// Get baud rate 
long GetBaud (char* rate) 
{
	char temp[10];
	char *baudrate=temp;
	long baud=0;
	
	strcpy(baudrate, rate);

	if (!strcmp("300", baudrate))
	{
		baud=B300;
		delay=64;
	}
	else if (!strcmp("600", baudrate))
	{
		baud=B600;
		delay=32;
	}
	else if (!strcmp("1200", baudrate))
	{
		baud=B1200;
		delay=16;
	}
	else if (!strcmp("2400", baudrate))
	{
		baud=B2400;
		delay=8;
	}
	else if (!strcmp("4800", baudrate))
	{
		baud=B4800;
		delay=4;
	}
	else if (!strcmp("9600", baudrate))
	{
		baud=B9600;
		delay=2;
	}
	else if (!strcmp("19200", baudrate))
	{
		baud=B19200;
		delay=1;
	}
	else if (!strcmp("38400", baudrate))
	{
		baud=B38400;
		delay=1;
	}
	else if (!strcmp("57600", baudrate))
	{
		baud=B57600;
		delay=1;
	}
	else if (!strcmp("115200", baudrate))
	{
		baud=B115200;
		delay=1;
	}
	else if (!strcmp("230400", baudrate))
	{
		baud=B230400;
		delay=1;
	}
	else
	{
		printf("Baud rate %s is not supported, ", baudrate);
		printf("use 300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 115200, 230400.\n");
		exit(1);
	}
	return baud;
}

// Set baud rate 
int SetBaud (int port, long baud)
{	
	/* Example : tio.c_cflag = baud | CLOCAL | CS8 | CREAD; */
	tio.c_cflag = baud | CS8 | CREAD;
	if (hardware_control)
	{
		tio.c_cflag |= HUPCL;
		tio.c_cflag |= CRTSCTS;
	}
	else
	{
		tio.c_cflag |= CLOCAL;
	}
		
	tcflush(port, TCIFLUSH);			/* flush buffer */
	tcsetattr(port, TCSANOW, &tio);		/* set attributes */
	usleep(100000);
	return 0;
}
	
//Write data to port
int WriteData (int port, char *data)
{
	int result = 0;
	char *buffer;

	if (verbose)
	{
		printf("\n Tx : size :");
	}
	buffer = (char *)malloc(2048);
	if (buffer == 0)
	{
		printf("unable to allocate malloc : buffer");
		exit(1);
	}
	memset(buffer, 0, 2048);

	strcpy(buffer, data);
	result = write(port, buffer, strlen(buffer));
	tcdrain(port);
	if (verbose)
	{
		printf("  %d", result);
	}
	if (result < 0)
	{
		fputs("write failed\n", stderr);
		close(port);
		exit(2);
	}
	free(buffer);
	return result;
}

// read data from port
int ReadData (int port, char **data)
{
	char *buffer;
	char *store;
	int result=0;
	int start=1;
	int retry=10;
	int timeout=100;

	buffer = (char *)malloc(2048);

	if (buffer == 0)
	{
		printf("\n\n unable to allocate buffer");
		exit (1);
	}
	store = (char *)malloc(2048);
	if (store == 0)
	{
		printf("\n\n unable to allocate store");
		exit (1);
	}
	memset(buffer, 0, 2048);
	memset(store, 0, 2048);
	while (start || result || retry)	
	{
		if (timeout-- == 0)
		{
			break;
		}
		result = read(port,buffer,1024);
		buffer[result] = 0;		/* zero terminate so printf works */ 
		if (result)
		{
			start=0;
			timeout=100;
			strcat(store, buffer);
		}
		if ( !start && !result) 
		{
			retry-=1;
		}
		usleep(50000);
	}
	strcpy(*data, store);
	result=strlen(store);
	if (verbose)
	{
		printf("\n Rx : size : %d", result);
		printf("\n Rx : data : %s", store);
	}
	free (buffer);
	free (store);
	return result;
}

int main(int argc, char *argv[])
{
	int fd;
	long cmd_baud = 0;
	long test_baud = 0;
	char cmd[20] = {0};
	char *buf;
	int RS485=0;
	int write_count = 0;
	int read_count = 0;
	int test_count = 0;
	int retry=3;
	int n=0;

	// allocate buf space
	buf = (char *)malloc(2048);
	if (buf == 0)
	{
		printf("\n\n error allocating buf");
		exit(1);
	}

	// help arguments
	if ( (argc != 6 && argc != 7 && argc != 8))
	{
		printf("Usage: serial_server [port] [command baud] [string] [test baud] [serial type]\n");
		printf("       port        Port you want to use, e.g. /dev/ttyS4\n");
		printf("       cmd baud    Default baud rate for test commandds to server\n");
		printf("       string      Test data, can be string, or a file\n");
		printf("       test baud   Test baud rate\n");
		printf("       serial      Serial test type, RS232, RS485, RS422\n");
		printf("       -hfcnt      Hardware flow control\n"); 
		printf("       -v          Verbose output\n");
		exit( 1 );
	}

	// parse arguments
	for (n=0; n<argc; n++)
	{
		if (strstr(argv[n], "-v"))
		{
			verbose=1;
		}
		if (strstr(argv[n], "-hfcnt"))
		{
			hardware_control = 1;
		}
	}

	// default baud for test command communications
	cmd_baud=GetBaud(argv[2]);

	// baud rate to test
	test_baud=GetBaud(argv[4]);

	// length of test file/string
	test_count=strlen(argv[3]);
	
	// RS485 RTS control
	if (strstr(argv[5], "RS485"))
	{
		RS485=1;
	}
  
	/* pthread example from queryserial */
	//if(argc == 6 && !strcmp(argv[5], "-hfcnt"))
	//{
		//pthread_create (&thread_id, NULL, &stop_test, NULL);
	//}

	/* open the serial port device file
	* O_NDELAY - tells port to operate and ignore the DCD line
	* O_NOCTTY - this process is not to become the controlling 
	*            process for the port. The driver will not send
	*            this process signals due to keyboard aborts, etc.
	*/
	/* Example : if ((fd = open(argv[1],O_RDWR | O_NDELAY | O_NOCTTY)) < 0) */
	if ((fd = open(argv[1],O_RDWR | O_NONBLOCK )) < 0)
	{
		printf("Couldn't open %s\n",argv[1]);
		exit(1);
	}

	/* we are not concerned about preserving the old serial port configuration
	* CS8, 8 data bits
	* CREAD, receiver enabled
	* CLOCAL, don't change the port's owner 
	*/
	/* Example : tio.c_cflag = cmd_baud | CLOCAL | CS8 | CREAD; */
	tio.c_cflag = cmd_baud | CS8 | CREAD;

	if (hardware_control)
	{
		tio.c_cflag |= HUPCL;
		tio.c_cflag |= CRTSCTS;
	}
	else
	{
		tio.c_cflag |= CLOCAL;
	}
		

	//	if( (argc == 6 && strcmp(argv[5], "-hfcnt")) )
	//		tio.c_cflag |= CRTSCTS;
	// tio.c_cflag &= ~CLOCAL; /* clear the CLOCAL bit so modem status does not get ignored */

	//tio.c_cflag &= ~PARENB;
	tio.c_lflag = 0;       /* set input flag non-canonical, no processing */
	tio.c_iflag = IGNPAR;  /* ignore parity errors */
	tio.c_oflag = 0;       /* set output flag non-canonical, no processing */
	tio.c_cc[VTIME] = 0;   /* no time delay */
	tio.c_cc[VMIN]  = 0;   /* no char delay */
	tcflush(fd, TCIFLUSH); /* flush the buffer */
	tcsetattr(fd, TCSANOW, &tio); /* set the attributes */

	/* Set up for no delay, ie non-blocking reads will occur. 
	*   When we read, we'll get what's in the input buffer or nothing 
	*/
	//fcntl(fd, F_SETFL, FNDELAY);
	fcntl(fd, F_SETFL, 0);

	strcat(cmd, "BAUD");
	strcat(cmd, argv[4]);
	
	/* SEND COMMAND */

	// set RTS for RS485 Tx
	if (RS485)
	{	
		setRTS(fd, 1);
		usleep(1000*delay*1);
	}
	
	//Write cmd to slave, allow retries in case slave not responive
	while (retry-- > 0)
	{
		write_count = WriteData(fd, cmd);
		if (write_count != 0)
		{
			break;
		}
	}	
	// command retry failure
	if (retry == 0)
	{
		exit (3);
	}

	// De-assert RTS for RS485 Rx
	if (RS485)
	{
		setRTS(fd, 0);
		usleep(1000*delay*1);
	}

	// read the input buffer and print it
	read_count = ReadData(fd, &buf);

	// Set test baud if passed
	if (write_count == read_count)	
	{
		SetBaud(fd, test_baud);
	}
	else
	{	
		close(fd);
		exit(4);
	}
	
	// Assert RTS for RS485 Tx
	if (RS485)
	{
		setRTS(fd, 1);
		usleep(1*delay*1000);
	}

	/* TEST DATA */

	// write test data
	if (!strstr(argv[3], "NULL"))
	{
		write_count = WriteData( fd, argv[3]);
		
		// De-asser RTS for RS485 Rx
		if (RS485)
		{
			setRTS(fd, 0);
			usleep(1*delay*1000);
		}	

		// simple write bufer count compare wih read buffer count 
		read_count = ReadData(fd, &buf);
		
		// buffer comapre failure
		if (strcmp(buf, argv[3]) != 0)
		{
			close(fd);
			free(buf);
			exit(5);
		}
		// buffer count failure
		if (read_count != test_count)
		{
			close(fd);
			free(buf);
			exit (6);
		}
	
		// set baud to default
		SetBaud(fd, cmd_baud);
		close(fd);
		//printf("\n\n");
	}
	free(buf);
	exit (0);
}

/* EOF */
