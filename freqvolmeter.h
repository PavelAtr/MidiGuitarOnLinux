#ifndef _FREQVOLMETER_H
#define _FREQVOLMETER_H
#include "main.h"
#include "jack.h"

#define PERIOD_MAX 40000 // in usecs
#define PERIOD_MIN 200 // in usecs
#define PERIOD_TIMEOUT PERIOD_MAX * SAMPLERATE / 1000000 //in samples
#define ADC_MAX 0xFFFF //in ADC points
#define ADC_ZERO_SIN 0 //in ADC points
#define ADC_MAX_SIN ADC_MAX - ADC_ZERO_SIN //in ADC points
#define ADC_MAX_RMS (ADC_MAX * 0.638) //in ADC points
#define VOLUME_NOISE 100 //in ADC points
#define SUSTAIN_FACTOR 0.8 //0 ... 1 float
#define PERIOD_ACCURACY_DIFF 10 // period accuracy error %


typedef struct {
	volume_long_t volume;
	ucounter_t accuracy; //in 1/samplerate points
	ulongcounter_t prev;
	ulongcounter_t cur;
	ulongcounter_t prev_single;
	ulongcounter_t cur_single;
	ucounter_t period_divider;
	ucounter_t serialno;
	flag_short_t ready;
	
	ulongcounter_t samplecounter;
	volume_t period_volume_max;
	volume_t period_volume_min;
	volume_long_t volume_tmp;
	ucounter_t accuracy_tmp;
	ucounter_t period_divider_tmp;
	ulongcounter_t period_volume_last;
	flag_short_t comparator_zero;
	flag_short_t comparator_max;
	flag_short_t comparator_min;
} sensor;

typedef struct {
	period_t period; //in usecs
	period_t period_sampl; //in sampl
	ucounter_t divider;
	period_t period_single;
	volume_t volume;
	ucounter_t serialno;
	ucounter_t accuracy; //in 1/samplerate points
	errno_t errors;
} sensor_value;

//errors values:
#define ETIMEOUT 1
#define EDIRTY 2
#define EACCURACY 3

extern sensor sens;

void freqvolmeter_init();
sensor_value* read_sensor(sensor* sens, sensor_value* buf);

#endif
