#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "main.h"
#include "jack.h"
#include "midi.h"
#include "freqvolmeter.h"
#include "guitar.h"


void guitar_baner(void)
{
	midiNoteOnOut(0x24, 127, CHANNEL);
	sleep(1);
	midiNoteOffOut(0x24, 127, CHANNEL);
	midiNoteOnOut(0x28, 127, CHANNEL);
	sleep(1);
	midiNoteOffOut(0x28, 127, CHANNEL);
	midiNoteOnOut(0x2B, 127, CHANNEL);
	sleep(1);
	midiNoteOffOut(0x2B, 127, CHANNEL);
}

int main(int argc, char** argv)
{
	jack_init();
	freqvolmeter_init();
	while(1) 
	{
		sensor_value sensvalue;
		read_sensor(&sens, &sensvalue);
		if (!sensvalue.notactual && sensvalue.volume > VOLUME_NOISE)
			printf("RMS=%d\t\tperiod=%d\t\taccuracy=%d\n", sensvalue.volume, sensvalue.period, sensvalue.accuracy);
		usleep(100);
	}
	return 0;
}
