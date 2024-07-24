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
	midiNoteOnOut(0x24, 127, CHANNEL1);
	sleep(1);
	midiNoteOffOut(0x24, 127, CHANNEL1);
	midiNoteOnOut(0x28, 127, CHANNEL1);
	sleep(1);
	midiNoteOffOut(0x28, 127, CHANNEL1);
	midiNoteOnOut(0x2B, 127, CHANNEL1);
	sleep(1);
	midiNoteOffOut(0x2B, 127, CHANNEL1);
}

int main(int argc, char** argv)
{
	jack_init();
	freqvolmeter_init();
	while(1) 
	{
		sensor_value sensvalue;
		read_sensor(&sens, &sensvalue);
		
		#ifdef DEBUGRAW
		if (!sensvalue.errors && sensvalue.volume > VOLUME_NOISE)
			printf("RMS=%d\tperiod_single=%d\tperiod=%d\tperiod_sampl=%d\tdivider=%d\taccuracy=%d\n", sensvalue.volume, sensvalue.period_single, sensvalue.period, sensvalue.period_sampl, sensvalue.divider, sensvalue.accuracy);
		#endif
		
		perform_freqvol(&sensvalue, &struna1);
		perform_send(&struna1);
		
		usleep(100);
	}
	return 0;
}
