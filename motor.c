#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <wiringPi.h>
#include <pthread.h>
#include <softPwm.h>
#include "motor.h"
#include "static_pinout.h"
#include "utils.h"

#define PWM_PIN	4
#define DIS_PIN	5
#define	ENC_PIN	6
#define	DIR_PIN	7
#define	PULSE_SAMPLING_MS	250
#define	MAX_PWM	1024

// Local function declaration
void* rpmCalculatorThrFunc (void* null_ptr);
pthread_t rpmCalculatorThr;


static int init_motors_done = 0;
int count, iter, diff, value;
float KP, rpm, dutyCycle, timer;
volatile int pulseCounter, currentPulseCounter;

//	ISR function for the encoder counter 
void counterA_ISR (void) {
	++pulseCounter;
}

//	Thread function to calculate RPM
void* rpmCalculatorThrFunc (void* null_ptr) {
	const int expected_cnt = (int)(rpm * 211.2 * (float)PULSE_SAMPLING_MS / (4000.0 * 60.0)); timer = 0.0;
	value = 0;
	diff = expected_cnt;
	dutyCycle = (float)value * 100.0 / (float)MAX_PWM;
	rpm = 1500;
	KP = 1;
	while(1) {
		pulseCounter = 0;
		delay(PULSE_SAMPLING_MS);
		currentPulseCounter = pulseCounter;
		value += (int)((float)(expected_cnt - currentPulseCounter) * KP);
		value = value > MAX_PWM ? MAX_PWM : (value < 0 ? 0 : value);	// Bounds 0 <= value <= MAX_PWM
		softPwmWrite(PWM_PIN, value);
		timer = ++iter * PULSE_SAMPLING_MS;
		diff = expected_cnt - currentPulseCounter;
		dutyCycle = (float)value * 100.0 / (float)MAX_PWM;
	}

	rpm = (float)currentPulseCounter * 4000.0 * 60.0 / (211.2 * (float)PULSE_SAMPLING_MS);
	printf("\nFinal RPM: %.2f\n", rpm);
	printf("Exiting...\n");
	softPwmWrite(PWM_PIN, 0);
	delay(100); //wait a little for the pwm to finish write

}



//	init_motor: initializes motor pin (GPIO allocation and initial values of output)
//	and initialize the elements of all motor control data structure

int initMotor(void) {
	rpm = 1500;
	count = 0;
	//	Set all GPIO pin modes and states 
	pinMode(PWM_PIN, OUTPUT); 
	pinMode(DIS_PIN, OUTPUT); 
	pinMode(DIR_PIN, OUTPUT); 
	pinMode(ENC_PIN, INPUT);
	digitalWrite(DIS_PIN, 0);  // Set disable pin to low
	digitalWrite(DIR_PIN, 1); // set direction, 1: spin clockwise, 0: spin counter-clockwise 
	if(!init_motors_done) {
		if (softPwmCreate(PWM_PIN, 0, MAX_PWM) )	//Create a SW PWM, value from 0 to MAX_PWM (=100% duty cycle)
		{
			fprintf(stderr,"Error creating software PWM: %s\n", strerror(errno));
			fflush(stderr);
			return -1;
		}
		pulseCounter = 0;
		//	Start counter ISR and RPM calculator wiringPiISR(ENC_PIN, INT_EDGE_FALLING, &counterA_ISR);
		int ret = pthread_create( &(rpmCalculatorThr), NULL, rpmCalculatorThrFunc, NULL); 
		if( ret ) { // ret = 1 on error
			fprintf(stderr,"Error creating rpmCalculatorThr - pthread_create() return code: %d\n",ret); 
			fflush(stderr);
		return ret;
		}
	}
	init_motors_done = 1;
	return 0;
}


//	stopMotor: stop the motor 
void stopMotor(void) {
	printf("\nExiting...\n");
	printf("\nFinal RPM: %.2f\n", (float)currentPulseCounter * 4000.0 * 60.0 / (211.2 * (float)PULSE_SAMPLING_MS)); 
	softPwmWrite(PWM_PIN, 0);
	delay(100); //wait a little for the pwm to finish write exit(0);
}

//	getCount: accessor funtion of a motor encoder counter
int getCount(void) {
	return count;
}



//	getRPM: accessor funtion of a motor encoder counter 
int getRPM(void) {
	return rpm;
}