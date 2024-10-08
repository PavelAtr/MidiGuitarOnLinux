#ifndef _SIMPLSEMAPHORE_H
#define _SIMPLSEMAPHORE_H
#include <unistd.h>

typedef volatile unsigned char semaphore_t;

#define semaphore_wait(sem) while(sem); sem = 1;
#define semaphore_post(sem) sem = 0;

#endif
