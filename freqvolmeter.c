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
ucounter_t PERIOD_ACCURACY_MIN;

void adcprocess()
{
	semaphore_waitnosleep(sem);
	sensor* s = &sens;
	for (jack_nframes_t i = 0; i < ports_nframes; i++)
	{
		volume_t ADC_voltage = ADC_MAX * inputbuf[i];
		s->volume_tmp += (ADC_voltage > ADC_ZERO_SIN)? ADC_voltage - ADC_ZERO_SIN : ADC_ZERO_SIN - ADC_voltage;
		s->accuracy_tmp++;
		s->samplecounter++;

		if (ADC_voltage > ADC_ZERO_SIN)
			s->comparator_zero = 1;
		else if (sens.comparator_zero)
		{
			s->comparator_zero = 0;
			s->period_volume_max = SUSTAIN_FACTOR * s->period_volume_max;
			s->period_volume_min = SUSTAIN_FACTOR * s->period_volume_min;
		}
		if (ADC_voltage > sens.period_volume_max)
		{
			s->period_volume_max = ADC_voltage;
			s->comparator_min = 1;
			s->period_volume_last = s->samplecounter;
			if (s->comparator_max)
			{
				s->comparator_max = 0;
				s->period_divider_tmp++;
				if (s->accuracy_tmp > PERIOD_ACCURACY_MIN)
					s->ready = 1;
			}
		}
		if (ADC_voltage < s->period_volume_min)
		{
			s->period_volume_min = ADC_voltage;
			s->comparator_max = 1;
			if (s->comparator_min)
			{
				s->comparator_min = 0;
				s->prev_single = s->cur_single;
				s->cur_single = s->period_volume_last;
				if (s->ready)
				{
					s->ready = 0;
					s->prev = s->cur;
					s->cur = s->period_volume_last;
					s->volume = s->volume_tmp;
					s->accuracy = s->accuracy_tmp;
					s->period_divider = s->period_divider_tmp;
					s->volume_tmp = 0;
					s->accuracy_tmp = 0;
					s->period_divider_tmp = 0;
				}
			}
		}
	}
	semaphore_post(sem);
}

void freqvolmeter_init()
{
	extern_process = &adcprocess;
	PERIOD_ACCURACY_MIN = ports_nframes / 2;
	memset(&sens, 0x0, sizeof(sens));
}

sensor_value* read_sensor(sensor* sens, sensor_value* buf)
{
		semaphore_wait(sem);
		buf->period =  (sens->period_divider > 1)? (sens->cur - sens->prev) / (sens->period_divider - 1) : 0;
		buf->period = buf->period * 1000000 / samplerate;
		buf->accuracy = sens->accuracy;
		buf->volume = (sens->accuracy != 0)? sens->volume / sens->accuracy : 0;
		buf->notactual = 0;
		if (sens->samplecounter - sens->prev > PERIOD_TIMEOUT)
			buf->notactual =  ETIMEOUT;
		if (buf->period > PERIOD_MAX || buf->period <= PERIOD_MIN)
			buf->notactual =  EDIRTY;
		semaphore_post(sem);
		
		return buf;
}

