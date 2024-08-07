#ifndef _CLI_H
#define _CLI_H
#include "main.h"

int cli_init(int argc, char** argv);

extern bool_t enable_midi;
extern bool_t enable_bends;
extern bool_t enable_slides;
extern bool_t enable_tremolo;
extern bool_t debug_raw;
extern bool_t debug_midi;
extern bool_t debug_alg;
extern bool_t enable_tuning;

#endif
