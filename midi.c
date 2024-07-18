#include "midi.h"
#include "jack.h"


midi_packet packet;


void midiNoteOnOut(char note, char vel, char channel) {
	packet.len = 3;
	packet.data[0] = MIDI_NOTE_ON | channel;
	packet.data[1] = MIDI_DATA_MASK & note;
	packet.data[2] = MIDI_DATA_MASK & vel;
	sendmidi(packet.len, packet.data);
}

void midiNoteOffOut(char note, char vel, char channel) {
	packet.len = 3;
	packet.data[0] = MIDI_NOTE_OFF | (channel & MIDI_CHANNEL_MASK);
	packet.data[1] = MIDI_DATA_MASK & note;
	packet.data[2] = MIDI_DATA_MASK & vel;
	sendmidi(packet.len, packet.data);
}

void midiControlChangeOut(char controller, char value, char channel) {
	packet.len = 3;
	packet.data[0] = MIDI_CONTROL_CHANGE | (channel & MIDI_CHANNEL_MASK);
	packet.data[1] = MIDI_DATA_MASK & controller;
	packet.data[2] = MIDI_DATA_MASK & value;
	sendmidi(packet.len, packet.data);
}

void midiProgramChangeOut(char program, char channel) {
	packet.len = 2;
	packet.data[0] = MIDI_PROGRAM_CHANGE | (channel & MIDI_CHANNEL_MASK);
	packet.data[1] = MIDI_DATA_MASK & program;
	sendmidi(packet.len, packet.data);
}

void midiPolyTouchOut(char note, char pressure, char channel) {
	packet.len = 3;
	packet.data[0] = MIDI_POLY_TOUCH | (channel & MIDI_CHANNEL_MASK);
	packet.data[1] = MIDI_DATA_MASK & note;
	packet.data[2] = MIDI_DATA_MASK & pressure;
	sendmidi(packet.len, packet.data);
}

void midiChannelTouchOut(char pressure, char channel) {
	packet.len = 2;
	packet.data[0] = MIDI_CHANNEL_TOUCH | (channel & MIDI_CHANNEL_MASK);
	packet.data[1] = MIDI_DATA_MASK & pressure;
	sendmidi(packet.len, packet.data);
}

void midiPitchBendOut(char bendLSB, char bendMSB, char channel) {
	packet.len = 3;
	packet.data[0] = MIDI_PITCH_BEND | (channel & MIDI_CHANNEL_MASK);
	packet.data[1] = MIDI_DATA_MASK & bendLSB;
	packet.data[2] = MIDI_DATA_MASK & bendMSB;
	sendmidi(packet.len, packet.data);
}
