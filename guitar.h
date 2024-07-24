#ifndef _GUITAR_H
#define _GUITAR_H

#include "freqvolmeter.h"


typedef struct {
	volume_t volume;
	period_t period;
	ucounter_t accuracy;
	int index;
	pitch_t bend;
	} note;

typedef struct {
	volume_t curvolume;
	volume_t oldvolume;
	byte_t channel;
	flag_short_t flags;
	note oldnote;
	note curnote;
	note newnote;
	} struna;

#define NOTE_NEW 0x01
#define NOTE_END 0x02
#define NOTE_NEWPITCH 0x04
#define NOTE_SILENCE 0x08

typedef struct {
	pitch_t realpitch;
	byte_t bendLSB;
	byte_t bendMSB;
	} pitch;

#define PITCH_MAX 200 //in percent
#define PITCH_MIN -200 //in percent
#define PITCH_TRESHOLD 70 //in percent 200% = 2 semitone
#define PITCH_STEP 10 // in percent 200% = 2 semitone
#define VOLUME_MAX 10000
#define VOLUME_NEW_TRESHOLD VOLUME_MAX * 0.1



extern struna struna1;

void perform_freqvol(sensor_value* sensvalue, struna* str);
void perform_send();

extern struna struna1;

#endif

