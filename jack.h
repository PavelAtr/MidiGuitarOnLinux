#ifndef _JACK_H
#define _JACK_H

#include <jack/jack.h>
#include "main.h"

#define INPUT_BUFFER_LEN 2 // in seconds
#define MIDI_BUFFER_LEN 100 // in packets

void jack_init(void);
extern void (*extern_process)();

extern unsigned int SAMPLERATE;

typedef struct
{
	char name[20];
	jack_port_t* input_port;
	jack_default_audio_sample_t* inputbuf;
	jack_nframes_t ports_nframes;
} input;

extern input inputs[CHANNEL_NUM];

void sendmidi(size_t len, char* buf);

#endif

