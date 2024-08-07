#ifndef _NOTES_H
#define _NOTES_H
#include "main.h"

#define NUMNOTES 88
#define STARTMIDINOTE 22

typedef struct
{
	period_t period;
	char* name;
} key;

extern const key notes[NUMNOTES]; //in usecs

#endif
