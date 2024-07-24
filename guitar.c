#include "guitar.h"
#include "midi.h"
#include "main.h"
#include "notes.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struna struna1;


note* search_pitch(note* inp)
{
	period_t a = notes[inp->index] - inp->period;
	period_t b = inp->period - notes[inp->index + 1];
	period_t c = notes[inp->index] - notes[inp->index + 1];

	if (a < b)
	{
		inp->bend = a * 100 / c;
	} else
	{
		inp->bend = - (b * 100 / c);
		inp->index = inp->index + 1;
	}
	
	return inp;
}

note* search_note(note* inp)
{
	for (int i = 0; i < NUMNOTES - 1; i++)
	{
		if (inp->period <= notes[i] && inp->period > notes[i + 1])
		{
			inp->index = i;
			search_pitch(inp);
			return inp;
		}
	}
	
	inp->index = -1;
	
	return inp;
}

note* note_copy(note* copyto, note* copyfrom)
{
	memcpy(copyto, copyfrom, sizeof(note));
	
	return copyto;
}

pitch* normalize_pitch(pitch* input, pitch_t inpitch)
{
	input->bendMSB = 0;
	input->bendLSB = 0;
	pitch_t realpitch = (float)inpitch / PITCH_MAX * MIDI_PITCH_HALF + MIDI_PITCH_ZERO;
	input->realpitch = realpitch;
	input->bendMSB = (realpitch >> 7) & MIDI_PITCH_MLSB_MASK;
	input->bendLSB = realpitch & MIDI_PITCH_MLSB_MASK;
	return input;
}

byte_t normalize_velocity(int volume)
{
	volume_t velocity = volume * MIDI_VELOCITY_MAX / VOLUME_MAX ;
	velocity = (velocity >= MIDI_VELOCITY_MAX) ? MIDI_VELOCITY_MAX : velocity;
	
	return velocity;
}

void perform_freqvol(sensor_value* sensvalue, struna* str)
{
	if (sensvalue->serialno == str->serialno)
		return;
	else str->serialno = sensvalue->serialno;
	
	str->oldvolume = str->curvolume;
	str->curvolume = sensvalue->volume;
	if (sensvalue->errors) return;
	
	if (sensvalue->volume < VOLUME_NOISE)
	{
		str->curvolume = sensvalue->volume;
		if (!(str->flags & NOTE_SILENCE))
		{
			note_copy(&str->oldnote, &str->curnote);
			str->flags |= NOTE_END;
		}
		return;
	}
	
	flag_short_t flags = 0;

	str->newnote.period = sensvalue->period;
	search_note(&str->newnote);
	
	if (str->newnote.index == -1) return;
	
	flags |= (str->oldvolume + VOLUME_NEW_TRESHOLD < str->curvolume) ? NOTE_NEW : 0;
	flags |= (str->flags & NOTE_SILENCE) ? NOTE_NEW : 0;
	flags |= (abs(str->curnote.bend) < PITCH_TRESHOLD && str->newnote.index != str->curnote.index) ? NOTE_NEW : 0;
	
	if (flags & NOTE_NEW)
	{
		note_copy(&str->oldnote, &str->curnote);
		note_copy(&str->curnote, &str->newnote);
		str->curnote.volume = sensvalue->volume;
		str->curnote.accuracy = sensvalue->accuracy;
		str->flags |= NOTE_NEW | NOTE_NEWPITCH;
		if (!(str->flags & NOTE_SILENCE))
			str->flags |= NOTE_END;
	}
}

char tmp[30];

void perform_send(struna* str)
{
	pitch tmppitch;
	
	if (str->flags & NOTE_END)
	{
		#ifdef REALMIDI				
		midiNoteOffOut(str->oldnote.index + STARTNOTE, normalize_velocity(str->oldnote.volume), str->channel);
		#endif
		
		#ifdef DEBUGMIDI
		printf("chn=%d note END=%d vel=%d per=%d\r\n", str->channel, str->oldnote.index, normalize_velocity(str->oldnote.volume), str->oldnote.period);
		#endif
		
		str->flags &= ~NOTE_END;
		str->flags |= NOTE_SILENCE;
	}
	if (str->flags & NOTE_NEW)
	{
		#ifdef REALMIDI
		midiNoteOnOut(str->curnote.index  + STARTNOTE, normalize_velocity(str->curnote.volume), str->channel);
		#endif
		
		#ifdef DEBUGMIDI
		printf("chn%d note NEW=%d vel=%d per=%d acc=%d vol=%d\r\n", str->channel, str->curnote.index, normalize_velocity(str->curnote.volume), str->curnote.period, str->curnote.accuracy, str->curnote.volume);
		#endif

		str->flags &= ~NOTE_NEW;
		str->flags &= ~NOTE_SILENCE;
	}
	if (str->flags & NOTE_NEWPITCH)
	{
		normalize_pitch(&tmppitch, str->curnote.bend);
		
		#ifdef REALMIDI		
		midiPitchBendOut(tmppitch.bendLSB, tmppitch.bendMSB, str->channel);
		#endif
		
		#ifdef DEBUGMIDI
		printf("chn=%d PITCHNEW=%d MSB=%d LSB=%d\r\n", str->channel, str->curnote.bend, tmppitch.bendMSB, tmppitch.bendLSB);
		#endif
		
		str->flags &= ~NOTE_NEWPITCH;
	}
}

void guitar_init()
{
	struna1.channel = CHANNEL1;
}
