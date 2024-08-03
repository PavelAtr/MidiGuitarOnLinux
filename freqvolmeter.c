#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <error.h>
#include <unistd.h>
#include "freqvolmeter.h"
#include "jack.h"
#include "main.h"
#include <string.h>

sensor sensors[CHANNEL_NUM];
ucounter_t PERIOD_ACCURACY_MIN;

void adcprocess()
{
	sensor* s = &sensors[0];
	
	if (!s->overload)
	{
		//semaphore_waitnosleep(s->sem);
	} else
		printf("OVERLOAD!\r\n");

	s->overload = 1;
		
	for (jack_nframes_t i = 0; i < ports_nframes; i++)
	{
		volume_t ADC_voltage = ADC_MAX * inputbuf[i] -ADC_ZERO_SIN;
		s->volume_tmp += abs(ADC_voltage);
		s->accuracy_tmp++;
		s->samplecounter++;

		if (ADC_voltage >= s->volume_max)
		{
		// Total accuracy of sinusoide max
			//Store peak of voltage
			s->volume_max = ADC_voltage;
			if (s->volume_max > s->volume_max_prev && s->comparator_max)
			{
			//Comaprator positive halfwave
				s->comparator_max = 0;
				//Set parameters of negative halfvawe
				s->volume_min_prev  = s->volume_min * SUSTAIN_FACTOR;
				s->volume_min = 0;
				//Increment period divider
				s->period_divider_tmp++;
			}
			// Store current peak of period
			s->cur_tmp = s->samplecounter;
			if (s->period_divider_tmp < 2)
			{
			//Start measurment at sinusoide max
				//Store start peak of period and reset values
				s->prev_tmp = s->samplecounter;
				s->volume_tmp = 0;
				s->accuracy_tmp = 0;
			}
			if (s->accuracy_tmp > PERIOD_ACCURACY_MIN)
			{
			// Needed measurments counted
				s->ready = 1;
			}
			//Enable negative comparator
			s->comparator_min = 1;
		}
		if (ADC_voltage <= s->volume_min)
		{
			//Store peak of voltage
			s->volume_min = ADC_voltage;
			if (s->volume_min < s->volume_min_prev && s->comparator_min)
			{
			//Comparator negative halfvawe
				s->comparator_min = 0;
				//Set parameters of positive halfwave
				s->volume_max_prev = s->volume_max * SUSTAIN_FACTOR;
				s->volume_max = 0;
				if (s->ready)
				{
				// Needed measurments counted, write result values
					s->ready = 0;
					s->serialno++;
					s->prev = s->prev_tmp;
					s->cur = s->cur_tmp;
					s->volume = s->volume_tmp;
					s->accuracy = s->accuracy_tmp;
					s->period_divider = s->period_divider_tmp;
					s->period_divider_tmp = 0;
				}
			}
			//Enable positive comparator
			s->comparator_max = 1;
		}
	}
	semaphore_post(s->sem);
	s->overload = 0;
}

void freqvolmeter_init()
{
	extern_process = &adcprocess;
	PERIOD_ACCURACY_MIN = ports_nframes;
	memset(&sensors[0], 0x0, sizeof(sensor));
}

sensor_value* read_sensor(sensor* sens, sensor_value* buf)
{
		semaphore_wait(sens->sem);
		buf->period =  (sens->period_divider > 1)?
			(sens->cur - sens->prev) * 1000000 /
			((sens->period_divider - 1) * SAMPLERATE) 
			: 0;
		buf->volume = (sens->accuracy != 0)?
			sens->volume / sens->accuracy : 0;
		buf->accuracy = sens->accuracy;
		buf->serialno = sens->serialno;
		buf->period_divider = sens->period_divider;
		semaphore_post(sens->sem);
		buf->errors = 0;
		if (sens->samplecounter - sens->prev > PERIOD_TIMEOUT)
			buf->errors =  ETIMEOUT;
		if (buf->period > PERIOD_MAX || buf->period <= PERIOD_MIN)
			buf->errors =  EDIRTY;

		return buf;
}

