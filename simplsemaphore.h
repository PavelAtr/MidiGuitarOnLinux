#ifndef _SIMPLSEMAPHORE_H
#define _SIMPLSEMAPHORE_H
#include <unistd.h>

typedef volatile unsigned char semaphore_t;

#define semaphore_waitnosleep(sem) while(sem); sem = 1
#define semaphore_wait(sem) while(sem) usleep(1); sem = 1
#define semaphore_post(sem) sem = 0

#endif
