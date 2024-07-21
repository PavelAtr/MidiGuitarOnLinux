#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <error.h>
#include <unistd.h>
#include "freqvolmeter.h"
#include "jack.h"
#include "main.h"
#include "simplsemaphore.h"
#include <string.h>

semaphore_t sem = 0;

sensor sens;
samplecount_t samplecounter = 0;

void adcprocess()
{
	volume_long_t volume_tmp;
	ucounter_t measurments_tmp;
	samplecount_t period_volume_max_last;
	volume_t period_volume_max;
	volume_t period_volume_min;
	volume_t period_volume_max_prev;
	volume_t period_volume_min_prev;


	semaphore_wait(sem);
	for (jack_nframes_t i = 0; i < ports_nframes; i++)
	{
		volume_t ADC_voltage = ADC_MAX * inputbuf[i];
		volume_tmp += (ADC_voltage > ADC_ZERO_SIN)? ADC_voltage - ADC_ZERO_SIN : ADC_ZERO_SIN - ADC_voltage;
		if (ADC_voltage > period_volume_max)
		{
			period_volume_max_last = samplecounter;
			period_volume_max = ADC_voltage;
			if (ADC_voltage > period_volume_max_prev * 0.9)
			{
				period_volume_min_prev = period_volume_min;
				period_volume_min = 0;
			}
		}

		if (ADC_voltage < period_volume_min)
		{
			period_volume_min = ADC_voltage;
			if (ADC_voltage < period_volume_min_prev * 0.9)
			{
				period_volume_max_prev = period_volume_max;
				period_volume_max = 0;
				sens.period = samplecounter - period_volume_max_last;
			}
		}

		measurments_tmp++;
		samplecounter++;
		if (ADC_voltage > ADC_ZERO_SIN)
		{
			sens.comparator = 1;
		} else 
		{
			if (sens.comparator)
			{
				sens.comparator = 0;
				sens.volume = volume_tmp;
				sens.measurments = measurments_tmp;
				volume_tmp = 0;
				measurments_tmp = 0;
			}
		}
	}
	semaphore_post(sem);
}

void freqvolmeter_init()
{
	extern_process = &adcprocess;
	memset(&sens, 0x0, sizeof(sens));
}

sensor_value* read_sensor(sensor* sens, sensor_value* buf)
{
		semaphore_wait(sem);
//		buf->period = abs(sens->period_volume_max_last - sens->period_volume_min_last);
//		buf->period = sens->period * 1000000 / samplerate;
		buf->accuracy = sens->measurments;
		buf->volume = (sens->measurments != 0)? sens->volume / sens->measurments : 0;
		buf->notactual = 0;
//		if (sens->period_tmp.last * 1000000 / samplerate > PERIOD_TIMEOUT)
//			buf->notactual = ETIMEOUT;
		if (buf->period > PERIOD_TIMEOUT || buf->period <= PERIOD_MIN)
			buf->notactual =  EDIRTY;
		semaphore_post(sem);
		
		return buf;
}
