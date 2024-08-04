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
	midiNoteOnOut(0x24, 127, 0);
	sleep(1);
	midiNoteOffOut(0x24, 127, 0);
	midiNoteOnOut(0x28, 127, 0);
	sleep(1);
	midiNoteOffOut(0x28, 127, 0);
	midiNoteOnOut(0x2B, 127, 0);
	sleep(1);
	midiNoteOffOut(0x2B, 127, 0);
}

int main(int argc, char** argv)
{
	jack_init();
	freqvolmeter_init();
	guitar_init();
	while(1) 
	{
		sensor_value sensvalue[CHANNEL_NUM];
		read_sensor(&sensors[0], &sensvalue[0]);
		
		#ifdef DEBUGRAW
		if (!sensvalue[0].errors && sensvalue[0].volume > VOLUME_NOISE)
			printf("RMS=%d per=%d acc=%d div=%d\n",
				sensvalue[0].volume, sensvalue[0].period,
				sensvalue[0].accuracy, sensvalue[0].period_divider);
		else reset_sensor(sensvalue[0].sens);
		#endif
		
		perform_freqvol(&sensvalue[0], &struny[0]);
		perform_send(&struny[0]);
	}
	return 0;
}
