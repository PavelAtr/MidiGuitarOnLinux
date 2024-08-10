#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "main.h"
#include "jack.h"
#include "midi.h"
#include "freqvolmeter.h"
#include "guitar.h"
#include "cli.h"


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
	if (cli_init(argc, argv)) return 1;
	jack_init();
	freqvolmeter_init();
	guitar_init();
	
	while(1) 
	{
		sensor_value sensvalue[CHANNEL_NUM];
		for (ucounter_t i = 0; i < CHANNEL_NUM; i++)
		{
			read_sensor(&sensors[i], &sensvalue[i]);
			perform_freqvol(&sensvalue[i], &struny[i]);
		}
		for (ucounter_t i = 0; i < CHANNEL_NUM; i++)
		{
			perform_send(&struny[i]);
		}
		usleep(10);
	}
	return 0;
}
