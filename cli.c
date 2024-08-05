#include "cli.h"
#include <unistd.h>
#include <stdio.h>
#include <getopt.h>
#include "main.h"
#include <string.h>

bool_t enable_midi = 0;
bool_t enable_bends = 0;
bool_t enable_slides = 0;
bool_t debug_raw = 0;
bool_t debug_midi = 0;
bool_t debug_alg = 0;

void usage(void)
{
	printf("Usage: under construction\n");
}

void cli_init(int argc, char** argv)
{
	int c;

	while (1) {
		int this_option_optind = optind ? optind : 1;
		int option_index = 0;
		static struct option long_options[] = {
                   {"midi",    no_argument, 0,  0 },
                   {"bends",  no_argument, 0,  0 },
                   {"slides",  no_argument, 0,  0 },
                   {"debug-alg",  no_argument, 0,  0 },
                   {"debug-raw",  no_argument, 0,  0 },
                   {"debug-midi",  no_argument, 0,  0 },
                   {0, 0, 0, 0 }
        };

        c = getopt_long(argc, argv, "",
                         long_options, &option_index);

        if (c == -1)
             break;

        switch (c) {
			case 0:
				if (strcmp(long_options[option_index].name, "midi") == 0)
					enable_midi = 1;
				if (strcmp(long_options[option_index].name, "bends") == 0)
					enable_bends = 1;
				if (strcmp(long_options[option_index].name, "slides") == 0)
					enable_slides = 1;
				if (strcmp(long_options[option_index].name, "debug-alg") == 0)
					debug_alg = 1;
				if (strcmp(long_options[option_index].name, "debug-raw") == 0)
					debug_raw = 1;
				if (strcmp(long_options[option_index].name, "debug-midi") == 0)
					debug_midi = 1;
                break;
            default:
                break;;
        }
    }
    
    if (!enable_midi && !enable_bends && !enable_slides && !debug_raw
		&& !debug_midi && !debug_alg)
		usage();
}
