/*
 *  libser.h
 *  
 *
 *  Created by Andrew Holt on 03/06/2010.
 *
 */
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <errno.h>

#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>

//-----------------------------------------------------------------------------
// Name: u16OpenSerialPort
// 
// Description:	Opens the serial port for comminucation 
//
// Returns: 
//		fd:	The file descriptor for the serial port
//
//-----------------------------------------------------------------------------

int u16OpenSerialPort(char *acSerialPortName, speed_t tBaudRate) {	

	int iSerialPort;
    int fd=-1;

    if( tBaudRate == 0) {
        tBaudRate = B115200;
    }

	printf("serial: Opening Serial Port (%s) at %d\n", acSerialPortName,(int)tBaudRate ); 
    fd = open( acSerialPortName , O_NOCTTY | O_RDWR | O_NONBLOCK|O_EXCL);

    if( fd < 0) {
		printf("serial: Can't open serial port - %s\n", acSerialPortName);
        perror("u16OpenSerialPort");
		exit (0);
	}

	printf("serial: Opening Serial Port (%d)\n", fd); 

	//-------------------------------------
	// Then initialise UART.
	//-------------------------------------
	iConfigureSerialPort(fd, tBaudRate);

	eFlushSerialPort(fd);
	return fd ;
}

//-----------------------------------------------------------------------------
// Name: 	teCloseSerialPort()
//
// Description: This function flushes, and then closes the serial port.
//
// Parameters:
//		iSerialPort: The serial port file descriptor
//
// Returns:
//      Error value.    
//		
//-----------------------------------------------------------------------------

int teCloseSerialPort(int iSerialPort) {

	int eSerialError ;

	eFlushSerialPort(iSerialPort);
	eSerialError=close(iSerialPort);

	return eSerialError;
}


//-----------------------------------------------------------------------------
// Name: init_serial_port()
//
// Description: This function initialises the serial port.
//
// Parameters:
//	Fd: The file descriptor for the serial port we are using.
//
// Returns:
//		0
//-----------------------------------------------------------------------------

int iConfigureSerialPort(int iSerialPort, speed_t tSpeed) {
	int iBits;
	struct termios tty;

	tcflush(iSerialPort, TCIOFLUSH);
	tcgetattr(iSerialPort, &tty);

	iBits = '8';
	cfsetospeed(&tty, tSpeed);
    cfsetispeed(&tty, tSpeed);
  
	switch (iBits) {
		case '5':
		  tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS5;
		  break;

		case '6':
		  tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS6;
		  break;
		
		case '7':
		  tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS7;
		  break;
		
		case '8':
		default:
		  tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
		  break;
	}
	
	tty.c_iflag |= IGNBRK;
	tty.c_lflag = 0;
	tty.c_oflag = 0;
	tty.c_cflag |= CLOCAL | CREAD;
	tty.c_cflag &= ~CRTSCTS;
	tty.c_cc[VMIN] = 1;
	tty.c_cc[VTIME] = 5;

    tty.c_iflag &= ~(IXON|IXOFF|IXANY);

	tty.c_cflag &= ~(PARENB | PARODD);
	tty.c_cflag &= ~CSTOPB;
	
	cfmakeraw(&tty);
	
	tcsetattr(iSerialPort, TCSANOW, &tty);

	return 0;
}

int eFlushSerialPort(int iSerialPort) {
	int eSerialError=0;

	int iBytesReceived;
	char cByte;

	do {
		iBytesReceived = read(iSerialPort, &cByte, 1);
	} while (iBytesReceived > 0);

	return eSerialError;
}
