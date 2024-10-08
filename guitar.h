#ifndef _GUITAR_H
#define _GUITAR_H

#include "freqvolmeter.h"


typedef struct {
	volume_t volume;
	period_t period;
	ucounter_t accuracy;
	ucounter_t serialno;
	int index;
	pitch_t bend;
	char* search;
	} note;

typedef struct {
	volume_t volume_max;
	volume_t curvolume;
	volume_t oldvolume;
	byte_t channel;
	ucounter_t serialno;
	flag_short_t note_flags;
	flag_short_t doublecheck;
	note oldnote;
	note curnote;
	note newnote;
	} struna;

// note flags:
#define NOTE_NEW 0x01
#define NOTE_END 0x02
#define NOTE_NEWPITCH 0x04
#define NOTE_SILENCE 0x08
#define NOTE_LOUDER 0x10
// doublecheck flags:
#define CHECK_AFTERSILENCE	0x01
#define CHECK_NEWFREQUENCY 0x02
#define CHECK_FURTHERPITCH 0x04
#define CHECK_PITCH 0x08
#define CHECK_NOCHECK  0xFF


typedef struct {
	pitch_t realpitch;
	byte_t bendLSB;
	byte_t bendMSB;
	} pitch;

// Constatnts:
#define PITCH_MAX 200//in %, 0.5 tone = 100%
#define PITCH_MIN -200 //in %
#define PITCH_TRESHOLD 35 // in %, note pitched and not be slided
#define PITCH_STEP 10 // in %, to reduce event count
#define PITCH_FURTHER 300 //in %, after this pitch new note occur
#define VOLUME_MAX_DEFAULT 14000 //in ADC points
#define VOLUME_NEW_TRESHOLD VOLUME_MAX_DEFAULT * 5 / 100
#define VOLUME_NOISE VOLUME_MAX_DEFAULT * 1 / 100
#define VOLUME_ACTUALFREQUENCY VOLUME_MAX_DEFAULT * 10 / 100

extern struna struny[CHANNEL_NUM];

void guitar_init();
void perform_freqvol(sensor_value* sensvalue, struna* str);
void perform_send(struna* str);
pitch* normalize_pitch(pitch* input, pitch_t bend);
#define pitch_diff(pitch1, pitch2) (pitch2 > pitch1)? pitch2 - pitch1: pitch1 - pitch2

#endif

