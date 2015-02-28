#include "PiPS2.h"
#include <wiringPi.h>
#include <stdio.h>

// Initialize the I/O pins and set up the controller for the desired mode.
//	TODO:
//		This function is hard coded to configure controller for analog mode.
//		Must also implement input parameters to choose what mode to use.
// 		If you want digital mode or analog mode with all pressures then use reInitController()
//	Inputs:
// 			--!! NOTE !!-- wiringPiSetupPhys(), wiringPiSetupGpio() OR wiringPiSetup()
// 				should be called first. The following pins refer to either the gpio or the 
//				physical pins, depending on how wiring pi was set up.
// 		_commandPin - The RPi pin that is connected to the COMMAND line of PS2 remote 
//  	_dataPin	- The RPi pin that is connected to the DATA line of PS2 remote
//  	_clkPin		- The RPi pin that is connected to the CLOCK line of PS2 remote
// 		_attnPin	- The RPi pin that is connected to the ATTENTION line of PS2 remote
//	Returns:
//		1 -		Config success.
//		0 - 	Controller is not responding.
unsigned char PIPS2::initializeController(int _commandPin, int _dataPin, int _clkPin, int _attnPin)
{
	
	// INITIALIZE I/O
	commandPin = _commandPin;
	dataPin = _dataPin;
	clkPin = _clkPin;
	attnPin = _attnPin;
	readDelay = 1;
	controllerMode = ANALOGMODE;
	// Set command pin to output
	pinMode(commandPin, OUTPUT);
	// Set data pin to input
	pinMode(dataPin, INPUT);
	// Set attention pin to output
	pinMode(attnPin, OUTPUT);
	// Set clock pin to output
	pinMode(clkPin, OUTPUT);
	
	// Set command pin and clock pin high, ready to initialize a transfer.
	digitalWrite(commandPin, 1);
	digitalWrite(clkPin, 1);

	// Read controller a few times to check if it is talking.
	readPS2();	
	readPS2();
	
	// Initialize the read delay to be 1 millisecond. 
	// Increment read_delay until controller accepts commands.
	// This is a but of dynamic debugging. Read delay usually needs to be about 2.
	// But for some controllers, especially wireless ones it needs to be a bit higher.
	
	// Try up until readDelay = MAX_READ_DELAY
	while (1)
	{
		// Transmit the enter config command.
		transmitCmdString(enterConfigMode, sizeof(enterConfigMode));
		
		// Set mode to analog mode and lock it there.
		transmitCmdString(set_mode_analog_lock, sizeof(set_mode_analog_lock));
		//delay(CMD_DELAY);
		// Return all pressures
		//transmitCmdString(config_AllPressure, sizeof(config_AllPressure));
		// Exit config mode.	
		transmitCmdString(exitConfigMode, sizeof(exitConfigMode));
		
		// Attempt to read the controller.
		readPS2();
		
		// If read was successful (controller indicates it is in analog mode), break this config loop.
		if(PS2data[1] == controllerMode)
		break;
	
		// If we have tried and failed 10 times. call it quits,
		if (readDelay == MAX_READ_DELAY)
		{
			return 0;
		}
		
		// Otherwise increment the read delay and go for another loop
		readDelay++;	
	}
	return 1;
}

// Bit bang a single byte.
// Inputs:
// 		byte 	- The byte to transmit.
//
// Returns:
// 		
unsigned char PIPS2::transmitByte(char byte)
{
	// Data byte received
	unsigned char RXdata = 0;
	
	// Bit bang all 8 bits
	for (int i = 0; i < 8; i++)
	{
		// If the bit to be transmitted is 1 then set the command pin to 1
		if (CHK(byte, i)) 	digitalWrite(commandPin, 1);
		// Otherwise set it to 0.
		else				digitalWrite(commandPin, 0);
		
		// Pull clock low to transfer bit
		digitalWrite(clkPin, 0);
		
		// Wait for the clock delay before reading the received bit.
		delayMicroseconds(CLK_DELAY);
		
		// If the data pin is now high then save the input.
		if (digitalRead(dataPin)) SET(RXdata, i);
		
		// Done transferring bit. Put clock back high
		digitalWrite(clkPin, 1);
		delayMicroseconds(CLK_DELAY);
	}
	
	// Done transferring byte, set the command pin back high and wait.
	digitalWrite(commandPin, 1);
	delayMicroseconds(BYTE_DELAY);
	return RXdata;
}

// Bit bang out an entire string of specified length.
//
// Inputs:
// 		string 	- Pointer to unsigned char array to be transmitted.
// 		len 	- Number of bytes to be transmitted.
void PIPS2::transmitCmdString(unsigned char *string, int len)
{
	// Create a temporary buffer for receiving the response.
	unsigned char tempBuffer[len];
	
	// Ready to begin transmitting, pull attention low.
	digitalWrite(attnPin, 0);
	
	// Shift the data out, one byte at a time.
	for (int y = 0; y < len; y++)
	{
		tempBuffer[y] = transmitByte(string[y]);
	}
	
	// Packet finished, release attention line.
	digitalWrite(attnPin, 1);
	// Wait some time before beginning another packet.
	delay(readDelay);
	
}

// Read the PS2 Controller. Save the returned data to PS2data.
void PIPS2::readPS2(void)
{
	static unsigned int last_read = 0;
	unsigned int timeSince = millis() - last_read;
	if (timeSince > 1500) //waited to long
		reInitializeController();
	if(timeSince < readDelay)  //waited too short
		delay(readDelay - timeSince);	
	// Ensure that the command bit is high before lowering attention.
	digitalWrite(commandPin, 1);
	// Ensure that the clock is high.
	digitalWrite(clkPin, 1);
	// Drop attention pin.
	digitalWrite(attnPin, 0);
	
	// Wait for a while between transmitting bytes so that pins can stabilize.
	delayMicroseconds(BYTE_DELAY);
	
	// The TX and RX buffer used to read the controller.
	unsigned char TxRx1[9] = {0x01,0x42,0,0,0,0,0,0,0};
	unsigned char TxRx2[12] = {0,0,0,0,0,0,0,0,0,0,0,0};
	
	// Grab the first 9 bits
	for (int i = 0; i<9; i++)
	{
		PS2data[i] = transmitByte(TxRx1[i]);
	}
	
	// If controller is in full data return mode, get the rest of data
	if(PS2data[1] == 0x79)
	{  
		for (int i = 0; i<12; i++)
		{
			PS2data[i+9] = transmitByte(TxRx2[i]);
			//printf("%d: %d\t->\t%d\n", i, (int)TxRx1[i], (int)PS2data[i]);
		}
	}
	
	// Done reading packet, release attention line.
	digitalWrite(attnPin, 1);
	last_read = millis();
	   
}

// Re-Initialize the I/O pins and set up the controller for the desired mode.
//	TODO:
//		Currently this function is only coded for either analog mode without all pressures
// 		returned or analog mode with all pressures. Need to implement digital.
//	Inputs:
// 			--!! NOTE !!-- wiringPiSetupPhys(), wiringPiSetupGpio() OR wiringPiSetup()
// 				should be called first. The following pins refer to either the gpio or the 
//				physical pins, depending on how wiring pi was set up.
// 		_commandPin - The RPi pin that is connected to the COMMAND line of PS2 remote 
//  	_dataPin	- The RPi pin that is connected to the DATA line of PS2 remote
//  	_clkPin		- The RPi pin that is connected to the CLOCK line of PS2 remote
// 		_attnPin	- The RPi pin that is connected to the ATTENTION line of PS2 remote
//
// Returns:
// 		 1 			- Success!
// 		-1 			- Invalid mode!
// 		-2 			- Failed to get controller into desired mode in less than MAX_INIT_ATTEMPT attempts
int PIPS2::reInitializeController(char _controllerMode)
{
	controllerMode = _controllerMode;
	if (controllerMode != ANALOGMODE && controllerMode != ALLPRESSUREMODE)
		return -1;
	
	for (int initAttempts = 1; initAttempts < MAX_INIT_ATTEMPT; initAttempts++)	
	{
		transmitCmdString(enterConfigMode, sizeof(enterConfigMode));
		transmitCmdString(set_mode_analog_lock, sizeof(set_mode_analog_lock));
		if (controllerMode == ALLPRESSUREMODE)
			transmitCmdString(config_AllPressure, sizeof(config_AllPressure));
		transmitCmdString(exitConfigMode, sizeof(exitConfigMode));
		readPS2();
		if (PS2data[1] == controllerMode)
			return 1;
		delay(readDelay);
	}
	return -2;
}

int PIPS2::reInitializeController()
{
	return PIPS2::reInitializeController(controllerMode);
}



