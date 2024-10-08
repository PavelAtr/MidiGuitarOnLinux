#ifndef _FREQVOLMETER_H
#define _FREQVOLMETER_H
#include "main.h"
#include "jack.h"
#include "simplsemaphore.h"

#define PERIOD_MAX 80000 // in usecs
#define PERIOD_MIN 200 // in usecs
#define PERIOD_TIMEOUT PERIOD_MAX * SAMPLERATE / 1000000 //in samples
#define ADC_MAX 0xFFFF //in ADC points
#define ADC_ZERO_SIN 0 //in ADC points
#define ADC_ZERO_TRESHOLD 10 //in ASDC points
#define ADC_MAX_RMS (ADC_MAX * 0.638) //in ADC points
#define COMPARATOR_TRESOLD 80 // %


typedef struct {
	volume_long_t volume;
	ucounter_t accuracy; //in 1/samplerate points
	volume_long_t volume_approx;
	ucounter_t accuracy_approx;
	volume_t volume_max_redy;
	volume_t approx_redy;
	ulongcounter_t prev;
	ulongcounter_t cur;
	ucounter_t period_divider;
	ucounter_t serialno;
	bool_t ready;
	semaphore_t sem;
	
	ulongcounter_t samplecounter;
	volume_t volume_max;
	volume_t volume_min;
	volume_t volume_max_prev;
	volume_t volume_min_prev;
	volume_long_t volume_tmp;
	ucounter_t accuracy_tmp;
	ucounter_t period_divider_tmp;
	ulongcounter_t cur_tmp;
	ulongcounter_t prev_tmp;
	bool_t comparator_max;
	bool_t comparator_min;
	bool_t comparator_zero;
	bool_t measure;
} sensor;

typedef struct {
	sensor* sens;
	period_t period; //in usecs
	volume_t volume;
	volume_t approx;
	volume_t volumemax;
	ucounter_t serialno;
	ucounter_t accuracy; //in samplerate points
	ucounter_t period_divider;
	errno_t errors;
} sensor_value;

//errors values:
#define ETIMEOUT 1
#define EDIRTY 2
#define EACCURACY 3

extern sensor sensors[CHANNEL_NUM];

void freqvolmeter_init();
sensor_value* read_sensor(sensor* sens, sensor_value* buf);

#endif
