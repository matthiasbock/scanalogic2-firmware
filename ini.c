// INI

#include "header.h"

void setup_timers()
{
	//setup timer 0 for USB pooling
	TCCR0A = (1<<WGM01);
	TCCR0B = (1<<CS00)|(1<<CS02); //Clock source = CLK_IO / 1024
	OCR0A = 24;//194; //Configuration for 50 Hz interrupt source
	TIMSK0 = (1<<OCIE0A);

	//Setup timer 1 for timestamping
	TCCR1A = 0;	//no waveform generation
	STOP_TIMER1	//Timer is started by "START_TIMER1" at the right moment.
	TIMSK1 = (1<<TOIE1);

	//SETUP TIMER 2 for clock generation
	STOP_TIMER2;//Timer is started by "START_TIMER2" when needed
}


void setup_trigger(uchar mask)
{
	PCICR  = (1<<PCIE1);	//enable Pin change interrupt on pins 
	PCMSK1 = mask; //Set the mask on the four sampling inputs according to user request
}

