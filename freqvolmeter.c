#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <error.h>
#include <unistd.h>
#include "freqvolmeter.h"
#include "jack.h"
#include "main.h"
#include "simplsemaphore.h"

semaphore_t sem = 0;

sensor sens;

void adcprocess()
{
	semaphore_wait(sem);
	for (jack_nframes_t i = 0; i < ports_nframes; i++)
	{
		volume_t ADC_voltage = ADC_MAX * inputbuf[i];
		sens.volume_tmp += (ADC_voltage > ADC_ZERO_SIN)? ADC_voltage - ADC_ZERO_SIN : ADC_ZERO_SIN - ADC_voltage;
		sens.measurments_tmp++;
		sens.period_tmp.period++;
		if (ADC_voltage > COMPARATOR_TRESHOLD)
		{
			sens.comparator = 1;
		} else if (sens.comparator)
		{
			sens.comparator = 0;
			sens.volume = sens.volume_tmp;
			sens.measurments = sens.measurments_tmp;
			sens.volume_tmp = 0;
			sens.measurments_tmp = 0;
			sens.period = sens.period_tmp.period;
			sens.period_tmp.period = 0;
		}
	}
	semaphore_post(sem);
}

void freqvolmeter_init()
{
	extern_process = &adcprocess;
}

sensor_value* read_sensor(sensor* sens, sensor_value* buf)
{
		semaphore_wait(sem);
		buf->period = sens->period * 1000000 / samplerate;
		buf->accuracy = sens->measurments;
		buf->volume = (sens->measurments != 0)? sens->volume / sens->measurments : 0;
		buf->peek = buf->volume >= ADC_MAX_RMS;
		buf->notactual = 0;
		if (sens->period_tmp.last * 1000000 / samplerate > PERIOD_TIMEOUT)
			buf->notactual = ETIMEOUT;
		if (buf->period > PERIOD_TIMEOUT || buf->period <= PERIOD_MIN)
			buf->notactual =  EDIRTY;
		semaphore_post(sem);
		
		return buf;
}
