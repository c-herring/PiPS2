/*
 * Sample program for PiPS2 library.
 * Compile: g++ sample.cpp PiPS2.cpp -o sample -lwiringPi
 *
 * Sets up the PS2 remote for analog mode and to return all pressure values.
 * Reads controller every 10ms and prints the buttons that are pressed to the console.
 * BTN_START uses the functionality to detect a button push or release. 
 * Holding R2 will print the pressure values of the right analog stick to console.
 * All other buttons just cause a message to be printed after every read if they are being pressed.
 *
 * You can just implement the same functionality from the Start button or from the R2 functionality 
 * for any keys.
 *
 */
 
#include <wiringPi.h> 
#include <stdio.h> 
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "PiPS2.h"

#define READDELAYMS 10

int main(int argc, char **argv) 
{
	char *progname = *argv++; argc--;

	if (wiringPiSetupPhys () == -1)
	{
		fprintf (stdout, "Unable to start wiringPi: %s\n", 
		strerror (errno)) ;
		return 1 ;
	}
	
	////////////// PiPS2 stuff ////////////////////////////
	// Create a PIPS2 object
	PIPS2 pips2;
	int nextRead = READDELAYMS;
	if (!pips2.initializeController(19, 21, 23, 24))
	{
		fprintf(stderr, "%s: Failed to configure gamepad\nController is not responding.\nExiting ...\n", progname);
			exit(EXIT_FAILURE);
	}
	int returnVal = pips2.reInitializeController(ALLPRESSUREMODE);
	if (returnVal == -1)
	{
		printf("%s: Invalid Mode\n", progname);
		exit(EXIT_FAILURE);
	} else if (returnVal == -2)
	{
		printf("%s: Took too many tries to reinit.\n", progname);
		exit(EXIT_FAILURE);
	}
	delay(50);
	printf("Control mode = %d\n", pips2.PS2data[1]);
	
	while (1)
	{
		if (millis() > nextRead)
		{
			nextRead += READDELAYMS;
			// Read the controller.
			pips2.readPS2();		
			
			// Example detecting when a button is pressed or released.
			char changedStates[2]; // Create the vector to hold changed states.
			pips2.getChangedStates(changedStates); // Populate the vector
			char btnDowns[2]; // Create the vector of buttons that have been pushed since last read.
			char btnUps[2]; 	// Create the vector of buttons that have been pushed since last read.
			// Buttons that have been pushed down are buttons that are currently down and have changed.
			btnDowns[0] = ~pips2.PS2data[3] & changedStates[0];
			// Buttons that have been released are buttons that are currently up and have changed.
			btnUps[0] = pips2.PS2data[3] & changedStates[0];
			
			// Just going to check for the START button.
			if (CHK(btnDowns[0], BTN_START))
				printf("BTN_START has been pushed DOWN\n");
			if (CHK(btnUps[0], BTN_START))
				printf("BTN_START has been RELEASED\n");
			
			
			// Example reading each button. 
			if (!CHK(pips2.PS2data[3], BTN_SELECT))
				printf("BTN_SELECT is pressed\n");
			if (!CHK(pips2.PS2data[3], BTN_RIGHT_JOY))
				printf("BTN_RIGHT_JOY is pressed\n");
			if (!CHK(pips2.PS2data[3], BTN_LEFT_JOY))
				printf("BTN_LEFT_JOY is pressed\n");
			//if (!CHK(pips2.PS2data[3], BTN_START))
			//	printf("BTN_START is pressed\n");
			if (!CHK(pips2.PS2data[3], BTN_UP))
				printf("BTN_UP is pressed\n");
			if (!CHK(pips2.PS2data[3], BTN_RIGHT))
				printf("BTN_RIGHT is pressed\n");
			if (!CHK(pips2.PS2data[3], BTN_DOWN))
				printf("BTN_DOWN is pressed\n");
			if (!CHK(pips2.PS2data[3], BTN_LEFT))
				printf("BTN_LEFT is pressed\n");
			if (!CHK(pips2.PS2data[4], BTN_L2))
				printf("BTN_L2 is pressed\n");
			if (!CHK(pips2.PS2data[4], BTN_R2))
			{
				printf("Right Joy   Horizontal = %d\tVertical = %d\n", pips2.PS2data[5], pips2.PS2data[6]);
			}
			if (!CHK(pips2.PS2data[4], BTN_L1))
				printf("BTN_L1 is pressed\n");
			if (!CHK(pips2.PS2data[4], BTN_R1))
				printf("BTN_R1 is pressed\n");
			if (!CHK(pips2.PS2data[4], BTN_TRIANGLE))
				printf("BTN_TRIANGLE is pressed\n");
			if (!CHK(pips2.PS2data[4], BTN_CIRCLE))
				printf("BTN_CIRCLE is pressed\n");
			if (!CHK(pips2.PS2data[4], BTN_X))
				printf("X is pressed\n");
			if (!CHK(pips2.PS2data[4], BTN_SQUARE))
				printf("BTN_SQUARE is pressed\n"); 
		}
	}
	
	return 0;
}