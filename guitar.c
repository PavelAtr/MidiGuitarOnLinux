#include "guitar.h"
#include "midi.h"
#include "main.h"
#include "notes.h"
#include <stdlib.h>
#include <string.h>

struna struna1;
struna struna2;


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
	byte_t velocity = (float)volume / VOLUME_MAX * MIDI_VELOCITY_MAX;
	velocity = (velocity >= MIDI_VELOCITY_MAX) ? MIDI_VELOCITY_MAX : velocity;
	
	return velocity;
}

void perform_freq(sensor* sens, struna* str)
{
	bool_t ret;// = is_sensor_notactual(sens);
	
	if (ret == ETIMEOUT || ret == EDIRTY)
	{
		if (!(str->flags & NOTE_SILENCE))
		{
			note_copy(&str->oldnote, &str->curnote);
			str->flags |= NOTE_END;
		}
		return;
	}

	flag_short_t flags = 0;

//	str->newnote.period = get_period(sens);
	search_note(&str->newnote);
	
	if (str->newnote.index == -1) return;
	
	flags |= (str->flags & NOTE_SILENCE) ? NOTE_NEW : 0;
	flags |= (abs(str->curnote.bend) < PITCH_TRESHOLD && str->newnote.index != str->curnote.index) ? NOTE_NEW : 0;
	
	if (flags & NOTE_NEW)
	{
		note_copy(&str->oldnote, &str->curnote);
		note_copy(&str->curnote, &str->newnote);
//		str->curnote.volume = get_volume(sens);;
		str->flags |= NOTE_NEW;
		if (!(str->flags & NOTE_SILENCE))
			str->flags |= NOTE_END;
	}
}

char tmp[30];

void perform_send()
{
	pitch tmppitch;
	
	if (struna1.flags & NOTE_END)
	{
		#ifdef REALMIDI				
		midiNoteOffOut(struna1.oldnote.index, normalize_velocity(struna1.oldnote.volume), CHANNEL1);
		#endif
		
		#ifdef DEBUGMIDI
		debug("STR1 note END=");
		debug(my_itoa(struna1.oldnote.index, tmp));
		debug(" vel=");
		debug(my_itoa(normalize_velocity(struna1.oldnote.volume), tmp));
		debug(" per=");
		debug(my_itoa(struna1.oldnote.period, tmp));
		debug("\r\n");
		#endif
		
		struna1.flags &= ~NOTE_END;
		struna1.flags |= NOTE_SILENCE;
	}
	if (struna1.flags & NOTE_NEW)
	{
		#ifdef REALMIDI
		midiNoteOnOut(struna1.curnote.index, normalize_velocity(struna1.curnote.volume), CHANNEL1);
		#endif
		
		#ifdef DEBUGMIDI
		debug("STR1 note NEW=");
		debug(my_itoa(struna1.curnote.index, tmp));
		debug(" vel=");
		debug(my_itoa(normalize_velocity(struna1.curnote.volume), tmp));
		debug(" vol=");
		debug(my_itoa(struna1.curnote.volume, tmp));
		debug(" per=");
		debug(my_itoa(struna1.curnote.period, tmp));
		debug("\r\n");
		#endif

		normalize_pitch(&tmppitch, 0);
		
		#ifdef REALMIDI
		midiPitchBendOut(tmppitch.bendLSB, tmppitch.bendMSB, CHANNEL1);
		#endif
		
		#ifdef DEBUGMIDI
		debug("STR1 PITCHNEW=");
		debug(my_itoa(0, tmp));
		debug("\r\n");
		#endif

		
		struna1.flags &= ~NOTE_NEW;
		struna1.flags &= ~NOTE_SILENCE;
	}
	if (struna1.flags & NOTE_NEWPITCH)
	{
		normalize_pitch(&tmppitch, struna1.curnote.bend);
		
		#ifdef REALMIDI		
		midiPitchBendOut(tmppitch.bendLSB, tmppitch.bendMSB, CHANNEL1);
		#endif
		
		#ifdef DEBUGMIDI
		debug("STR1 PITCHNEW=");
		debug(my_itoa(struna1.curnote.bend, tmp));
		debug("\r\n");
		#endif
		
		struna1.flags &= ~NOTE_NEWPITCH;
	}


	if (struna2.flags & NOTE_END)
	{
		#ifdef REALMIDI
		midiNoteOffOut(struna2.oldnote.index, normalize_velocity(struna2.oldnote.volume), CHANNEL2);
		#endif
		
		#ifdef DEBUGMIDI
		debug("STR2 note END=");
		debug(my_itoa(struna2.oldnote.index, tmp));
		debug(" vel=");
		debug(my_itoa(normalize_velocity(struna2.oldnote.volume), tmp));
		debug(" per=");
		debug(my_itoa(struna2.oldnote.period, tmp));
		debug("\r\n");
		#endif
		
		struna2.flags &= ~NOTE_END;
		struna2.flags |= NOTE_SILENCE;
	}
	if (struna2.flags & NOTE_NEW)
	{
		#ifdef REALMIDI
		midiNoteOnOut(struna2.curnote.index, normalize_velocity(struna2.curnote.volume), CHANNEL2);
		#endif
		
		#ifdef DEBUGMIDI
		debug("STR2 note NEW=");
		debug(my_itoa(struna2.curnote.index, tmp));
		debug(" vel=");
		debug(my_itoa(normalize_velocity(struna2.curnote.volume), tmp));
		debug(" vol=");
		debug(my_itoa(struna2.curnote.volume, tmp));
		debug(" per=");
		debug(my_itoa(struna2.curnote.period, tmp));
		debug("\r\n");
		#endif

		normalize_pitch(&tmppitch, 0);
		
		#ifdef REALMIDI
		midiPitchBendOut(tmppitch.bendLSB, tmppitch.bendMSB, CHANNEL2);
		#endif
		
		#ifdef DEBUGMIDI
		debug("STR2 PITCHNEW=");
		debug(my_itoa(0, tmp));
		debug("\r\n");
		#endif
		
		struna2.flags &= ~NOTE_NEW;
		struna2.flags &= ~NOTE_SILENCE;
	}
	if (struna2.flags & NOTE_NEWPITCH)
	{
		normalize_pitch(&tmppitch, struna2.curnote.bend);
		
		#ifdef REALMIDI
		midiPitchBendOut(tmppitch.bendLSB, tmppitch.bendMSB, CHANNEL2);
		#endif

		#ifdef DEBUGMIDI
		debug("STR2 PITCHNEW=");
		debug(my_itoa(struna2.curnote.bend, tmp));
		debug("\r\n");
		#endif	

		struna2.flags &= ~NOTE_NEWPITCH;
	}
	
	#ifdef DEBUGMIDI
	//normalize_pitch(&tmppitch, struna1.newnote.bend);
	//debug(my_itoa(struna1.newnote.index, tmp));
	//debug(" STR1 PITCH=");
	//debug(my_itoa(struna1.newnote.bend, tmp));
	//debug("  REA=");
	//debug(my_itoa(tmppitch.realpitch, tmp));
	//debug("  MSB=");
	//debug(my_itoa(tmppitch.bendMSB, tmp));
	//debug("  LSB=");
	//debug(my_itoa(tmppitch.bendLSB, tmp));
	//debug("\r\n");
	
	//normalize_pitch(&tmppitch, struna2.newnote.bend);
	//debug(my_itoa(struna2.newnote.index, tmp));
	//debug(" STR2 PITCH=");
	//debug(my_itoa(struna2.newnote.bend, tmp));
	//debug("  REA=");
	//debug(my_itoa(tmppitch.realpitch, tmp));
	//debug("  MSB=");
	//debug(my_itoa(tmppitch.bendMSB, tmp));
	//debug("  LSB=");
	//debug(my_itoa(tmppitch.bendLSB, tmp));
	//debug("\r\n");
	#endif
}
