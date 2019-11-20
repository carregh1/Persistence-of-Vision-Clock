/* 
\file	pov.c	
\author	Diego Carregha and Rasmus Nystrom
\date	05/28/2018	
\brief	Main persistence of vision file for the static RPi controllers.
This program will run the logic of the static RPi
*/		
// Linux C libraries	
#include <stdio.h>	//printf, fprintf, stderr, fflush, scanf, getchar
#include <string.h>	//strncpy, strerror
#include <errno.h>	//errno
#include <stdlib.h>	//exit, EXIT_SUCCESS, EXIT_FAILURE
#include <signal.h>	//signal, SIGINT, SIGQUIT, SIGTERM
#include <wiringPi.h>	//wiringPiSetup, pinMode, digitalWrite, delay, INPUT, OUTPUT, PWM_OUTPUT
#include <pthread.h>	
#include <unistd.h>	


// Local headers
#include "web_client.h" //initWebClient_new_port

// Local variables
const int pin_table[] = {28, 27, 26, 24, 23, 22, 21, 16,
						 15, 14, 13, 12, 11, 10, 9, 8, 7,
						 6, 5, 4, 3, 2, 1, 0};

int imgArr[120];
char msg[MESSAGE_BUFFER_SIZE];
volatile int arrIndex = 0;
const int lockCode = 10;

// Local function declaration

///	Function controlling exit or interrupt signals 
void control_event(int sig);

void receiveMessage(void); void ISR(void);

void* displayThread(void* arg); 

// Threads
pthread_t LED;


/**

main function - Entry point function for pov

@param argc number of arguments passed

@param *argv IP address of the rotor RPi

@return number stdlib:EXIT_SUCCESS exit code when no error found.

*/

int main (int argc, char *argv[]) {

	//	Inform OS that control_event() function will be handling kill signals (void)signal(SIGINT, control_event);
	(void)signal(SIGQUIT, control_event); (void)signal(SIGTERM, control_event);

	//Local variable definition

	int mStatus;

	//Set IP

	char server_ip[20];
	//	Parse 1st argument: Server IP 
	if(argc < 2) {
		printf("Enter you server IP >"); fflush(stdout);
		scanf(" %s",server_ip); getchar(); fflush(stdin);
	} else {
		strncpy(server_ip, argv[1], sizeof server_ip - 1);
	}
	printf("	Server IP	: %s\n", server_ip);
	printf("	My wireless IP is: %s\n", getMyIP("wlan0"));

	// Server Startup
	printf("Initialize Connection To Server...\n");
	initWebClient(server_ip);


	//	Initialize the Wiring Pi facility printf("Initialize Wiring Pi facility... ");
	if (wiringPiSetup() != 0) {
		// Handles error Wiring Pi initialization
		fprintf(stderr, "Unable to setup wiringPi: %s\n", strerror(errno));
		fflush(stderr);
		exit(EXIT_FAILURE);

	}

	printf("Done\n");

	//	Initialize GPIO pins (LED pins) printf("Initialize GPIO pins... "); 
	for(int i = 0; i < 24; i++)	{
		pinMode(pin_table[i], OUTPUT);
	}

	printf("Done\n");
	wiringPiISR(29, INT_EDGE_FALLING, &ISR);

	mStatus = pthread_create(&LED, NULL, displayThread, NULL);
	if (mStatus != 0) {
		printf("Error creating thread displayThread\n");
		exit(-1);
	} else {
		while(1) {
			for (int k = 0; k < 24; k++) {
				digitalWrite(pin_table[k], imgArr[arrIndex] & (1 << k));
			}
			delayMicroseconds(242);
			arrIndex++;
			arrIndex %= 120;
		}
 
	}
	printf("Start Main loop\n");
	return 0;
}

void control_event(int sig) {
	printf("\b\b	\nExiting pov... ");
	delay(200);
	printf("Done\n");
	exit(EXIT_SUCCESS);
}

//	function receiving message 
void receiveMessage(void) {
	printf("Waiting for a command...\n"); sprintf(msg,"%s",getMessage());
}

//	ISR function - resets clock position when IR receiver is triggered 
void ISR(void) {
	arrIndex = 67;
}


//	Thread handling message 
void* displayThread(void* arg) {
	char *segment; int ledArrIndex; char static_ip[20];
	char response[MESSAGE_BUFFER_SIZE]; const char endSeg[] = ",\n";
	char *s; while(1){
	receiveMessage(); // Receive message from server, stored in msg.
	segment = strtok(msg, endSeg);
	sprintf(static_ip,"%s",segment);
	segment = strtok(NULL, endSeg); //ignore rotor IP
	segment = strtok(NULL, endSeg);
	// message conditions
	if(strcmp(segment, "display") == 0){
		for (int j = 0; j < 120; j++){
			imgArr[j] = 0;
		}
		//update LED's
		segment = strtok(NULL, endSeg);
		while( segment != NULL ) {
			ledArrIndex = strtol(segment, NULL, 10);
			segment = strtok(NULL, endSeg);
			imgArr[ledArrIndex] = strtol(segment, NULL, 16);
			segment = strtok(NULL, endSeg);
		}
		//send response validation
		sprintf(response, "%s,%s,response,display,valid\n",getMyIP("wlan0"), static_ip);
		printf(response);
		sendMessage(response);
	} else if(strcmp(segment, "lock") == 0) {
		//Code for lock
	} else if(strcmp(segment, “unlock”) == 0) {
		//Code for unlock
	} else {
		//Code for test

	}
}
