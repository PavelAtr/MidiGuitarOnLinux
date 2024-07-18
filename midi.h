#ifndef _MIDI_H
#define _MIDI_H

#define MIDI_NOTE_ON 	0x90
#define MIDI_NOTE_OFF 	0x80
#define MIDI_POLY_TOUCH 	0xA0
#define MIDI_CONTROL_CHANGE	0xB0
#define MIDI_PROGRAM_CHANGE	0xC0
#define MIDI_CHANNEL_TOUCH	0xD0
#define MIDI_PITCH_BEND	        0xE0

#define MIDI_DATA_MASK			0x7F
#define MIDI_STATUS_MASK		0xF0
#define MIDI_CHANNEL_MASK		0x0F

typedef struct {
	unsigned int len;
	char data[4];
} midi_packet;


void midiNoteOnOut(char note, char  vel, char channel);
void midiNoteOffOut(char note, char vel, char channel);
void midiControlChangeOut(char controller, char value, char channel);
void midiProgramChangeOut(char program, char channel);
void midiPolyTouchOut(char note, char pressure, char channel);
void midiChannelTouchOut(char pressure, char channel);
void midiPitchBendOut(char bendLSB, char bendMSB, char channel);

#endif
