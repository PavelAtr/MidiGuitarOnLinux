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

	semaphore_wait(sem);
	for (jack_nframes_t i = 0; i < ports_nframes; i++)
	{
		volume_t ADC_voltage = ADC_MAX * inputbuf[i];
		sens.volume_tmp += (ADC_voltage > ADC_ZERO_SIN)? ADC_voltage - ADC_ZERO_SIN : ADC_ZERO_SIN - ADC_voltage;
		sens.accuracy_tmp++;
		samplecounter++;

		if (ADC_voltage > sens.period_volume_max)
		{
			sens.period_volume_max_last = samplecounter;
			sens.period_volume_max = ADC_voltage;
			sens.comparator2 = 1;
			if (sens.comparator1)
			{
				sens.comparator1 = 0;
				sens.period_volume_min = 0.8 * sens.period_volume_min;
			}
		}
		if (ADC_voltage < sens.period_volume_min)
		{
			sens.period_volume_min = ADC_voltage;
			sens.comparator1 = 1;
			if (sens.comparator2)
			{
				sens.comparator2 = 0;
				sens.period_volume_max = 0.8 * sens.period_volume_max;

				sens.volume = sens.volume_tmp;
				sens.accuracy = sens.accuracy_tmp;
				sens.volume_tmp = 0;
				sens.accuracy_tmp = 0;
				sens.prev = sens.cur;
				sens.cur = sens.period_volume_max_last;

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
		buf->period = sens->cur - sens->prev;
		buf->period_accuracy = buf->period;
		buf->period = buf->period * 1000000 / samplerate;
		buf->volume_accuracy = sens->accuracy;
		buf->volume = (sens->accuracy != 0)? sens->volume / sens->accuracy : 0;
		buf->notactual = 0;
//		if (samplecounter - sens->prev > PERIOD_TIMEOUT)
//			buf->notactual =  ETIMEOUT;
		if (buf->period > PERIOD_MAX || buf->period <= PERIOD_MIN)
			buf->notactual =  EDIRTY;
		semaphore_post(sem);
		
		return buf;
}

