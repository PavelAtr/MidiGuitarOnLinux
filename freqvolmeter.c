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
	sensor* s = &sens;
	for (jack_nframes_t i = 0; i < ports_nframes; i++)
	{
		volume_t ADC_voltage = ADC_MAX * inputbuf[i] -ADC_ZERO_SIN;
		semaphore_waitnosleep(sem);
		s->volume_tmp += abs(ADC_voltage);
		s->accuracy_tmp++;
		s->samplecounter++;

		if (ADC_voltage > ADC_ZERO_SIN)
			s->comparator_zero = 1;
		else if (sens.comparator_zero)
		{
		// From + to - trought zero, working always
			s->comparator_zero = 0;
			s->period_volume_max = SUSTAIN_FACTOR * s->period_volume_max;
			s->period_volume_min = SUSTAIN_FACTOR * s->period_volume_min;
		}
		if (ADC_voltage > sens.period_volume_max)
		{
		// Total accuracy of sinusoide max
			s->period_volume_max = ADC_voltage;
			s->comparator_min = 1;
			s->period_volume_last = s->samplecounter;
			if (s->comparator_max)
			{
			//comparator work at SUSTAIN_FACTOR(maxsin) once by period
				s->comparator_max = 0;
				s->period_divider_tmp++;
				if (s->accuracy_tmp > PERIOD_ACCURACY_MIN)
				// Counted needed measurments
					s->ready = 1;
			}
		}
		if (ADC_voltage < s->period_volume_min)
		{
			s->period_volume_min = ADC_voltage;
			s->comparator_max = 1;
			if (s->comparator_min)
			{
			//comparator work at SUSTAIN_FACTOR(minsin) once by period
				s->comparator_min = 0;
				// Write values by period
				s->prev_single = s->cur_single;
				s->cur_single = s->period_volume_last;
				if (s->ready)
				{
				// Counted needed measurments, write result walues
					s->ready = 0;
					s->serialno++;
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
		semaphore_post(sem);
	}
}

void freqvolmeter_init()
{
	extern_process = &adcprocess;
	PERIOD_ACCURACY_MIN = ports_nframes;
	memset(&sens, 0x0, sizeof(sens));
}

sensor_value* read_sensor(sensor* sens, sensor_value* buf)
{
		semaphore_wait(sem);
		buf->period =  (sens->period_divider > 0)? (sens->cur - sens->prev) / (sens->period_divider) : 0;
//		buf->period_sampl = buf->period;
		buf->period_single = sens->cur_single - sens->prev_single;
		buf->volume = (sens->accuracy != 0)? sens->volume / sens->accuracy : 0;
		buf->accuracy = sens->accuracy;
//		buf->divider = sens->period_divider;
		buf->serialno = sens->serialno;
		semaphore_post(sem);
		
		buf->errors = 0;
		if (buf->period != 0)
			if ((buf->period - buf->period_single) * 100 / buf->period >= PERIOD_ACCURACY_DIFF)	
				buf->errors =  EACCURACY;
		buf->period = buf->period * 1000000 / SAMPLERATE;
		if (sens->samplecounter - sens->prev > PERIOD_TIMEOUT)
			buf->errors =  ETIMEOUT;
		if (buf->period > PERIOD_MAX || buf->period <= PERIOD_MIN)
			buf->errors =  EDIRTY;

		return buf;
}

