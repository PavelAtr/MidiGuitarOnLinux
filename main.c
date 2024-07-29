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
	guitar_init();
	while(1) 
	{
		sensor_value sensvalue1;
		read_sensor(&sens, &sensvalue1);
		
		#ifdef DEBUGRAW
		if (!sensvalue1.errors && sensvalue1.volume > VOLUME_NOISE)
			printf("RMS=%d\t\tperiod=%d\t\taccuracy=%d\n",
				sensvalue1.volume, sensvalue1.period, sensvalue1.accuracy);
		#endif
		
		perform_freqvol(&sensvalue1, &struna1);
		perform_send(&struna1);
		
//		pitch tmp;
//		normalize_pitch(&tmp, -200);
//		printf("MSB=%d LSB=%d\n", tmp.bendMSB, tmp.bendLSB);
		
		usleep(100);
	}
	return 0;
}
