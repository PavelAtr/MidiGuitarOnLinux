#include <error.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include "jack.h"
#include <assert.h>
#include <string.h>
#include <jack/jack.h>
#include <jack/midiport.h>
#include "midi.h"
#include "main.h"
#include "simplsemaphore.h"

jack_port_t* midi_port;
jack_client_t* client;

unsigned int SAMPLERATE;
input inputs[CHANNEL_NUM];semaphore_t midisem = 0;
midi_packet* midibuf;
size_t midindx = 0;

void dummy(){}

void (*extern_process)() = &dummy;

int process (jack_nframes_t nframes, void *arg)
{
	for (ucounter_t i = 0; i < CHANNEL_NUM; i++)
		inputs[i].inputbuf  = jack_port_get_buffer(inputs[i].input_port, nframes);

	extern_process();
	
	semaphore_wait(midisem);
	void* out = jack_port_get_buffer(midi_port, nframes);
	jack_midi_clear_buffer(out);

	unsigned char* buffer;
	jack_nframes_t ind = 0;
	for (size_t i = 0; i < midindx; i++)
	{
		buffer = jack_midi_event_reserve(out, ind, midibuf[i].len);
		ind += midibuf[i].len;
		if (!buffer) continue;
		for (int j = 0; j < midibuf[i].len; j++)
			buffer[j] = midibuf[i].data[j];
	}
	midindx = 0;
	semaphore_post(midisem);
	
	return 0;      
}

void sendmidi(size_t len, char* buf)
{
	semaphore_wait(midisem);
	if (midindx < MIDI_BUFFER_LEN)
	{
		for (size_t i = 0; i < len; i++)
			midibuf[midindx].data[i] = buf[i];
		midibuf[midindx].len = len;
		midindx++;
	} else
	{
		perror("midi buffer overflow");
	}
	semaphore_post(midisem);
}

void jack_init(void)
{
	const char *client_name = "MidiGuitar";
	const char *server_name = NULL;
	jack_options_t options = JackNullOption;
	jack_status_t status;
	
	client = jack_client_open(client_name, options, &status, server_name);
	if (client == NULL)
		error(EBUSY, EBUSY, "jack connection fail\n");
	
	for (ucounter_t	i = 0; i < CHANNEL_NUM; i++)
	{
		sprintf(inputs[i].name, "input%d", i);
		inputs[i].input_port = jack_port_register (client, inputs[i].name,
						JACK_DEFAULT_AUDIO_TYPE,
						JackPortIsInput, 0);
	 	if (inputs[i].input_port == NULL)
			error(EBUSY, EBUSY, "jack input port fail\n");
	}

	midi_port = jack_port_register (client, "midioutput",
					  JACK_DEFAULT_MIDI_TYPE,
					  JackPortIsOutput, 0);
		
	if (midi_port == NULL)
		error(EBUSY, EBUSY, "jack midi port fail\n");

	jack_set_process_callback (client, process, 0);
	
	if (jack_activate (client))
		error(EBUSY, EBUSY, "jack activate fail\n");
	
	SAMPLERATE = jack_get_sample_rate (client);
	jack_nframes_t ports_nframes = jack_get_buffer_size(client);
	for (ucounter_t i = 0; i < CHANNEL_NUM; i++)
		inputs[i].ports_nframes = ports_nframes;
	
	midibuf = malloc(sizeof(char) * ports_nframes);
	assert(midibuf);
}



