#ifndef _FREQVOLMETER_H
#define _FREQVOLMETER_H
#include "main.h"
#include "jack.h"

#define PERIOD_MAX 40000
#define PERIOD_MIN 200
#define PERIOD_TIMEOUT PERIOD_MAX * samplerate / 1000000
#define ADC_MAX 0xFFFF
#define ADC_ZERO_SIN 0
#define ADC_MAX_SIN ADC_MAX
#define ADC_MAX_RMS (ADC_MAX * 0.638)
#define VOLUME_NOISE 100 	

#define MEASURMENT_TIME 2 //In signal periods

typedef struct {
	volume_long_t volume;
	ucounter_t accuracy; //in 1/samplerate points
	samplecount_t prev;
	samplecount_t cur;
	ucounter_t measurment_time;
	
	volume_t period_volume_max;
	volume_t period_volume_min;
	volume_long_t volume_tmp;
	ucounter_t accuracy_tmp;
	samplecount_t period_volume_max_last;
	flag_short_t comparator_zero;
	flag_short_t comparator_max;
	flag_short_t comparator_min;
} sensor;

#define ETIMEOUT 1
#define EDIRTY 2
typedef struct {
	period_t period; //in usecs
	volume_t volume;
	ucounter_t volume_accuracy; //in 1/samplerate points
	period_t period_accuracy; //in 1/samplerate points
	errno_t notactual;
} sensor_value;

extern sensor sens;

void freqvolmeter_init();
sensor_value* read_sensor(sensor* sens, sensor_value* buf);

#endif
