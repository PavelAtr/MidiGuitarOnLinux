#ifndef _FREQVOLMETER_H
#define _FREQVOLMETER_H
#include "main.h"
#include "jack.h"

#define PERIOD_TIMEOUT 100000
#define PERIOD_MIN 200
#define ADC_MAX 0xFFFF
#define ADC_ZERO_SIN 0
#define ADC_MAX_SIN ADC_MAX
#define ADC_MAX_RMS (ADC_MAX * 0.638)
#define COMPARATOR_TRESHOLD 400

typedef struct {
	volume_long_t volume;
	ucounter_t mecount; //in 1/samplerate points
	volume_long_t volume_tmp;
	ucounter_t mecount_tmp; //in 1/samplerate points
	period_t period; //in 1/samplerate points
	union {
		period_t period; //in 1/samplerate points
		period_t last;
	} period_tmp;
	flag_short_t comparator;
} sensor;

#define NACTUAL_PERIOD_TIMEOUT 1
#define NACTUAL_PERIOD_DIRTY 2
typedef struct {
	period_t period; //in usecs
	volume_t volume;
	ucounter_t accuracy; //in 1/samplerate points
	bool_t notactual;
	bool_t peek;
} sensor_value;

extern sensor sens;

void freqvolmeter_init();
sensor_value* read_sensor(sensor* sens, sensor_value* buf);

#endif
