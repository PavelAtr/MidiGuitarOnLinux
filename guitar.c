#include "guitar.h"
#include "midi.h"
#include "main.h"
#include "notes.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cli.h"

struna struny[CHANNEL_NUM];

pitch_t search_pitch(note* inp, period_t period)
{
	pitch_t bend;
	if (notes[inp->index].period >= period)
	{
		bend = (notes[inp->index].period - period) * 100 /
			(notes[inp->index].period - notes[inp->index + 1].period);
	}
	else
	{
		bend = -((period - notes[inp->index].period) * 100 / (notes[inp->index - 1].period - notes[inp->index].period));

	}
	return bend;
}

note* search_note(note* inp)
{
	for (int i = 0; i < NUMNOTES - 1; i++)
	{
		if (inp->period <= notes[i].period && inp->period > notes[i + 1].period)
		{
			if (notes[i].period - inp->period <= inp->period - notes[i + 1].period)
				inp->index = i;
			else
				inp->index = i + 1;
				
			inp->bend = search_pitch(inp, inp->period);
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

pitch* normalize_pitch(pitch* input, pitch_t bend)
{
	pitch_t inpitch = (bend >= PITCH_MAX) ? PITCH_MAX : bend;
	inpitch = (bend <= PITCH_MIN) ? PITCH_MIN : bend;
	input->bendMSB = 0;
	input->bendLSB = 0;
	pitch_t realpitch = inpitch * MIDI_PITCH_HALF / PITCH_MAX + MIDI_PITCH_ZERO;
	input->realpitch = realpitch;
	input->bendMSB = (realpitch >> 7) & MIDI_PITCH_MLSB_MASK;
	input->bendLSB = realpitch & MIDI_PITCH_MLSB_MASK;
	return input;
}

byte_t normalize_velocity(struna* str, int volume)
{
	volume_t velocity = volume * MIDI_VELOCITY_MAX / VOLUME_MAX_DEFAULT;
	velocity = (velocity >= MIDI_VELOCITY_MAX) ? MIDI_VELOCITY_MAX : velocity;
	
	return velocity;
}

void perform_freqvol(sensor_value* sensvalue, struna* str)
{
	if (sensvalue->serialno == str->serialno)
	{
		return;
	}	
	else
	{
		str->serialno = sensvalue->serialno;
	}

	if (debug_raw)
	{
		if (sensvalue->volume > str->volume_max)
		{
			str->volume_max = sensvalue->volume;
		}
		printf("chn=%d ser=%d MAX=%d RMS=%d per=%d acc=%d div=%d",
			str->channel, sensvalue->serialno, str->volume_max,
			sensvalue->volume, sensvalue->period,
			sensvalue->accuracy, sensvalue->period_divider);
		if (str->curvolume > str->oldvolume + VOLUME_NEW_TRESHOLD)
		{
			printf(" LOUDER=%d\n", str->curvolume - str->oldvolume);
		}
		else
		{
			printf("\n");
		}
	}
	
	if (sensvalue->errors)
	{
		return;
	}

	str->oldvolume = str->curvolume;
	str->curvolume = sensvalue->volume;

	if (sensvalue->volume < VOLUME_NOISE)
	{
	// End note by volume if not silence
		if (debug_alg)
		{
			printf("End note by volume\n");
		}
		note_copy(&str->oldnote, &str->curnote);
		str->note_flags |= NOTE_END;
		// Nothing else
		return;
	}
	
	flag_short_t flags = 0;

	// Searching note in array frequencys
	str->newnote.period = sensvalue->period;
	search_note(&str->newnote);
	
	if (enable_tuning)
		printf("%s\t%d% (period=%d)\n", notes[str->newnote.index].name, str->newnote.bend, str->newnote.period);
	
	if (str->newnote.index == -1) return;

#ifndef EVENT_DOUBLECHECK
	str->doublecheck = CHECK_NOCHECK;
#endif
	
	// Frequency after silence
	if (!enable_tremolo)
	{
		if (str->doublecheck & CHECK_AFTERSILENCE)
		{
			flags |=  NOTE_NEW;
			str->doublecheck = 0;
			if (debug_alg)
			{
				printf("New note %d after SILENCE, volume=%d, period=%d\n",
					str->newnote.index, sensvalue->volume, str->newnote.period);
			}
		}
		else
		{
			str->doublecheck |= CHECK_AFTERSILENCE;
		}
		goto end;
	}
	
	if (str->curvolume > str->oldvolume + VOLUME_NEW_TRESHOLD)
	{
	// New note is louder
		if (enable_tremolo)
		{
#ifdef EVENT_DOUBLECHECK
			str->note_flags |= NOTE_LOUDER;
#else
			flags |= NOTE_NEW;
			if (debug_alg)
			{
				printf("New note %d as LOUDER, volume=%d period=%d\n",
					str->newnote.index, str->curvolume, str->newnote.period);
			}
#endif
		}
		goto end;
	}

	if (str->curvolume < str->oldvolume && str->note_flags & NOTE_LOUDER)
	{
	// New note is louder
		flags |= NOTE_NEW;
		if (debug_alg)
		{
			printf("New note %d as LOUDER, volume=%d period=%d\n",
				str->newnote.index, str->curvolume, str->newnote.period);
		}
		goto end;
	}
	
	if (str->newnote.index == str->curnote.index)
	{
	// Note same
		if (pitch_diff(str->curnote.bend, str->newnote.bend) >= PITCH_STEP)
		{
		// if diff >= PITCH_STEP, newpitch
			str->curnote.bend = str->newnote.bend;
			flags |= NOTE_NEWPITCH;
			if (debug_alg)
			{
				if (enable_bends)
				{
					printf("New pitch %d in same note\n", str->newnote.bend);
				}
			}
		}
		goto end;
	}
	else
	{
		if (abs(str->curnote.bend) < PITCH_TRESHOLD)
		{
		// Note not pitched, slide
			if (sensvalue->volume >= VOLUME_ACTUALFREQUENCY)
			{
				if (enable_slides)
				{
					if (str->doublecheck & CHECK_NEWFREQUENCY)
					{
						str->doublecheck = 0;
						flags |= NOTE_NEW;
						if (debug_alg)
						{
							printf("New note %d as FREQUENCY, volume=%d, period=%d\n",
								str->newnote.index, sensvalue->volume, str->newnote.period);
						}
					}
					else
					{
						str->doublecheck |= CHECK_NEWFREQUENCY;
					}
				}
				goto end;
			}
		}
		else
		{
			//Note pitched
			pitch_t newpitch = search_pitch(&str->curnote, str->newnote.period);
			if (abs(newpitch) > PITCH_FURTHER)
			{
				if (sensvalue->volume >= VOLUME_ACTUALFREQUENCY)
				{
					if (str->doublecheck & CHECK_FURTHERPITCH)
					{
						flags |= NOTE_NEW;
						if (debug_alg)
						{
							printf("New note %d as FURTHER PITCH, volume=%d\n",
								str->newnote.index + STARTMIDINOTE, sensvalue->volume);
						}
					}
					else
					{
						str->doublecheck |= CHECK_FURTHERPITCH;
					}
					goto end;
				}
			}
			else
			{
				if (pitch_diff(str->curnote.bend, newpitch) >= PITCH_STEP)
				{
					// if diff >= PITCH_STEP, newpitch
					str->curnote.bend = newpitch;
					flags |= NOTE_NEWPITCH;
					if (debug_alg)
					{
						if (enable_bends)
						{
							printf("New further pitch %d\n", newpitch);
						}
					}
				}
				goto end;
			}
		}
	}
end:
	if (flags & NOTE_NEW)
	{
		note_copy(&str->oldnote, &str->curnote);
		note_copy(&str->curnote, &str->newnote);
		str->curnote.volume = sensvalue->volume;
		str->curnote.accuracy = sensvalue->accuracy;
		str->curnote.serialno = sensvalue->serialno;
		str->note_flags |= NOTE_NEW | NOTE_NEWPITCH;
		if (!(str->note_flags & NOTE_SILENCE))
			str->note_flags |= NOTE_END;
		str->note_flags &= ~NOTE_LOUDER;
		str->doublecheck = 0;
	}
	if (flags & NOTE_NEWPITCH)
	{
		if (!(str->note_flags & NOTE_SILENCE))
		{
			str->note_flags |= NOTE_NEWPITCH;
		}
		str->doublecheck = 0;
	}
}

char tmp[30];

void perform_send(struna* str)
{
	pitch tmppitch;
	
	if (str->note_flags & NOTE_END)
	{
		if (enable_midi)
		{
			midiNoteOffOut(str->oldnote.index + STARTMIDINOTE,
				normalize_velocity(str, str->oldnote.volume), str->channel);
		}
		
		if (debug_midi)
		{
			printf("chn=%d note END=%d velocity=%d period=%d\r\n",
				str->channel, str->oldnote.index + STARTMIDINOTE,
				normalize_velocity(str, str->oldnote.volume),
				str->oldnote.period);
		}
		str->note_flags &= ~NOTE_END;
		str->note_flags |= NOTE_SILENCE;
	}
	if (str->note_flags & NOTE_NEW)
	{
		if (enable_midi)
		{
			midiNoteOnOut(str->curnote.index + STARTMIDINOTE,
				normalize_velocity(str, str->curnote.volume), str->channel);
		}
		if (debug_midi)
		{
			printf("chn=%d note NEW=%d velocity=%d period=%d accuracy=%d volume=%d cur=%d old=%d\r\n",
				str->channel, str->curnote.index  + STARTMIDINOTE,
				normalize_velocity(str, str->curnote.volume),
				str->curnote.period, str->curnote.accuracy, str->curnote.volume, str->curvolume, str->oldvolume);
		}
		str->note_flags &= ~NOTE_NEW;
		str->note_flags &= ~NOTE_SILENCE;
	}
	if (str->note_flags & NOTE_NEWPITCH)
	{
		normalize_pitch(&tmppitch, str->curnote.bend);
		
		if (enable_midi)
		{
			if (enable_bends)
			{
				midiPitchBendOut(tmppitch.bendLSB, tmppitch.bendMSB, str->channel);
			}
		}
		if (debug_midi)
		{
			if (enable_bends)
			{
				printf("chn=%d PITCHNEW=%d MSB=%d LSB=%d\r\n",
					str->channel, str->curnote.bend,
					tmppitch.bendMSB, tmppitch.bendLSB);
			}
		}
		str->note_flags &= ~NOTE_NEWPITCH;
	}
}

void guitar_init()
{
	for (ucounter_t i = 0; i < CHANNEL_NUM; i++)
	{
		struny[i].channel = i;
		struny[i].volume_max = 0;
	}
}
