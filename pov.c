/* 
file	pov.c
authors	  Diego Carregha and Rasmus Nystrom
date	05/28/2018
brief	Main persistence of vision file for the static RPi controllers.
This program will run the logic of the static RPi

*/


// Linux C libraries	
#include	<stdio.h>	//printf, fprintf, stderr, fflush, scanf, getchar
#include	<string.h>	//strncpy, strerror
#include	<errno.h>	//errno
#include	<stdlib.h>	//exit, EXIT_SUCCESS, EXIT_FAILURE
#include	<signal.h>	//signal, SIGINT, SIGQUIT, SIGTERM
#include	<wiringPi.h>	//wiringPiSetup, pinMode, digitalWrite, delay, INPUT, OUTPUT, PWM_OUTPUT
#include	<time.h>	
#include	<pthread.h>	

// Local headers	
#include	"static_pinout.h"
#include	"motor.h"	//initMotor
#include	"web_client.h" //initWebClient


//	Local function declaration void sendM(void);

void initializeGPIO();

void* responseCheck(void* arg);


//	Global variables

char message[MESSAGE_BUFFER_SIZE];


// Local variables											
int clock_face[120]={ 0x4E0000,	0x920000,  0x620000,  0x000000,  0x000000,  0x000000,  0x000000,  0x000000,  0x000000,  0x000000,  0xFE0000,
0x000000,	0x000000,	0x000000,	0x000000,	0x000000,	0x000000,	0x000000,	0x000000,	0x000000,	0xFE0000,	0x000000,	0x000000,
0x000000,	0x000000,	0x000000,	0x000000,	0x400000,	0xA00000,	0x800000,	0x400000,	0x800000,	0xA00000,	0x400000,	0x000000,
0x000000,	0x000000,	0x000000,	0x000000,	0x000000,	0xFE0000,	0x000000,	0x000000,	0x000000,	0x000000,	0x000000,	0x000000,
0x000000,	0x000000,	0x000000,	0xFE0000,	0x000000,	0x000000,	0x000000,	0x000000,	0x000000,	0x000000,	0x000000,	0x000000,
0x640000,	0x920000,	0x7C0000,	0x000000,	0x000000,	0x000000,	0x000000,	0x000000,	0x000000,	0x000000,	0x000000,	0xFE0000,
0x000000,	0x000000,	0x000000,	0x000000,	0x000000,	0x000000,	0x000000,	0x000000,	0x000000,	0xFE0000,	0x000000,	0x000000,
0x000000,	0x000000,	0x000000,	0x000000,	0xE00000,	0x200000,	0x200000,	0xE00000,	0xA00000,	0xA00000,	0xE00000,	0x000000,
0x000000,	0x000000,	0x000000,	0x000000,	0x000000,	0xFE0000,	0x000000,	0x000000,	0x000000,	0x000000,	0x000000,	0x000000,
0x000000,	0x000000,	0x000000,	0xFE0000,	0x000000,	0x000000,	0x000000,	0x000000,	0x000000,	0x000000,	0x820000,	0xFE0000,
0x020000,	};										



///	Function controlling exit or interrupt signals void control_event(int sig);

//	Threads declaration pthread_t responseCheck_t;

/**

main function - Entry point function for pov


@param argc number of arguments passed

@param *argv IP address of the rotor RPi


@return number stdlib:EXIT_SUCCESS exit code when no error found.

*/

int main(int argc, char *argv[])

{

	//	Inform OS that control_event() function will be handling kill signals (void)signal(SIGINT, control_event);

	(void)signal(SIGQUIT, control_event); (void)signal(SIGTERM, control_event);	

	//Local variable definition

	char localServer_ip[20];	// IP of server


	//	Parse 1st argument: rotor RPi IP 
	
	if (argc < 2){
		printf("Enter Server IP: > "); fflush(stdout);
		scanf(" %s", localServer_ip); getchar();
		fflush(stdin);

    } else {

		strncpy(localServer_ip, argv[1], sizeof localServer_ip - 1);

	}


	printf("	Local Server IP	: %s\n", localServer_ip);
	printf("	My wireless IP is: %s\n", getMyIP("wlan0"));

	//	Open communication with local server 
	printf("Connecting to server...\n");
	initWebClient(localServer_ip);

	//	Call tonitialize wiring pi facility and GPIO pins 
	initializeGPIO();



	//	LOCK

	sprintf(message, "%s,10.133.0.15,lock", getMyIP("wlan0"));
	sprintf(message,"%s,%d,%X", message, 1;
	sendM();
	delay(200);

	//	Start encoder counter ISRs, setting actual motor RPM 
	printf("Initialize Motor... ");
	initMotor();
	printf("Done\n");


	// Create thread for checking for response
	pthread_create(&responseCheck_t, NULL, responseCheck, NULL);
	printf("Start Main loop\n");

	//	Initialize loop variables int minute;

	int hour;
	int prevMin = -1;	// update condition
	int currentClockFace[120];
	int minHand;
	int hourHand;
	struct tm *tTime;
	time_t currentTime;
	int minCount = 0;
	int sound = 0;
	
	while(1){
		//	Aquire current hour and minute time(&currentTime);
		tTime = localtime(&currentTime); hour = (tTime->tm_hour);
		minute = tTime->tm_min;
		// update clock 
		if (prevMin != minute) {
			// set constant lights
			for (int i = 0; i < 120; i++){
				currentClockFace[i] = clock_face[i];
			}
			minHand = minute * 2;
			hourHand = hour * 10;
			hourHand = (hourHand + minute/6) % 120;


			// update min hand
			currentClockFace [minHand] |= 0x03FFFF;
	 
			// update hour hand
			currentClockFace [hourHand]|= 0x002FFF;
	 
			sprintf(message, "%s,10.133.0.15,display",getMyIP("wlan0"));

			for (int i = 0; i < 120; i++) {
				if (currentClockFace[i] != 0){
				sprintf(message, "%s,%d,%X", message, i, currentClockFace[i]);
			}
		}

		sprintf(message, "%s\n", message);
		sendM();
		prevMin = minute;
		minCount++;

		//	play a sample every 5 min, tick every min and horn on the hour 
		if(minCount % 5 == 0 && minute != 0) {
			if (sound == 0) {
				system("omxplayer -o local s2.mp3");
				sound++;

			} else if (sound == 1) {
				sound++;
				system("omxplayer -o local s1.mp3");
			} else if (sound == 2) {
				sound++;
				system("omxplayer -o local s2.mp3");
			} else if (sound == 3) {
				sound++;
				system("omxplayer -o local s3.mp3");
			} else if (sound == 4) {
				sound++;
				system("omxplayer -o local s4.mp3");
			} else {
				system("omxplayer -o local s3.mp3");
				sound %= 4;
			}
			} else if (minute == 0){
				system("omxplayer -o local horn.mp3");
			} else {
				system("omxplayer -o local tick.mp3");
			}
		}

		delay(500);	// Delay in infinite loop

	}

		return 0;

}


//	Local function that initializes wiring pi facility and GPIO pins 

void initializeGPIO() {

	//	Initialize the Wiring Pi facility printf("Initialize Wiring Pi facility... ");
	if (wiringPiSetup() != 0) {
		// Handles error Wiring Pi initialization
		fprintf(stderr, "Unable to setup wiringPi: %s\n", strerror(errno));
		fflush(stderr);
		exit(EXIT_FAILURE);
	}

	printf("Done\n");
	//	Initialize GPIO pins 
	printf("Initialize GPIO pins... "); 
	pinMode(PWM_PIN, OUTPUT); 
	pinMode(DIS_PIN, OUTPUT); 
	pinMode(DIR_PIN, OUTPUT); 
	pinMode(ENC_PIN, INPUT);
	digitalWrite(DIS_PIN, 0);  // Set disable pin to low
	digitalWrite(DIR_PIN, 1);  // set direction, 1: spin clockwise, 0: spin counter-clockwise
	pinMode(4, OUTPUT);
	pinMode(5, OUTPUT);
	pinMode(27, OUTPUT);
	digitalWrite(4,0);
	digitalWrite(5,0);
	digitalWrite(27,1);
	pinMode(27,OUTPUT);
	digitalWrite(27, HIGH);
	system("gpio mode 23 alt0");
	system("gpio mode 26 alt0");
	printf("Done\n");

}


//	Local function that sends message to server 
void sendM(void) {
printf("Sending message: \"%s\"\n", message); sendMessage(message);
}


//	Thread that checks for response from rotorPi

void* responseCheck(void* arg) {

	char response[MESSAGE_BUFFER_SIZE];
	char rotor_ip[20];
	char status[] = "valid";
	char returnCommand[20];
	char *segment;
	const char endSegment[] = ",\n";

	while (1) {

		sprintf(response, "%s", getMessage()); 
		segment = strtok(response, endSegment); 
		sprintf(rotor_ip,"%s", segment); 
		segment = strtok(NULL, endSegment); 
		segment = strtok(NULL, endSegment);
		// Process response

		if (strcmp(segment, "response") == 0) { 
			segment = strtok(NULL, endSegment); 
			sprintf(returnCommand,"%s", segment); 
			segment = strtok(NULL, endSegment); // Status
 			if (strcmp(segment, status) != 0) { // error check
				printf("ERROR... \"%s\", recieved %s\n", returnCommand, segment);
			} else {
				printf("Clock updated!\n");
			}

		} else {
			printf("ERROR... Unknown command: %s\n", response);
		}

	}
}

//	Control event function, stops the rotor and exits pov 
void control_event(int sig) {
	printf("\b\b \nExiting pov... "); //stop the motor
	stopMotor();
	delay(200);
	printf("Done\n"); exit(EXIT_SUCCESS);

}