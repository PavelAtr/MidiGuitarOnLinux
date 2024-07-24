#ifndef _JACK_H
#define _JACK_H

#include <jack/jack.h>
#include "main.h"

#define INPUT_BUFFER_LEN 2 // in seconds
#define MIDI_BUFFER_LEN 100 // in packets

void jack_init(void);
extern void (*extern_process)();

extern unsigned int SAMPLERATE;
extern jack_default_audio_sample_t* inputbuf;
extern jack_nframes_t ports_nframes;

void sendmidi(size_t len, char* buf);

#endif

