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
ucounter_t PERIOD_ACCURACY_MIN = 256;

void adcprocess()
{
	sensor* s = &sensors[0];
	
	if (s->overload)
		printf("OVERLOAD!\r\n");
	s->overload = 1;
		
	for (jack_nframes_t i = 0; i < ports_nframes; i++)
	{
		volume_t ADC = ADC_MAX * inputbuf[i] - ADC_ZERO_SIN;
		s->samplecounter++;
		if (s->measure)
		{
			s->volume_tmp += abs(ADC);
			s->accuracy_tmp++;
		}
/*		if (ADC > ADC_ZERO_TRESHOLD)
			s->comparator_zero = 1;
		
		if (ADC < - ADC_ZERO_TRESHOLD && s->comparator_zero)
		{
			s->comparator_zero = 0;
		}
*/
		if (ADC > s->volume_max)
		{
			s->volume_max = ADC;
			s->cur_tmp = s->samplecounter;
			s->comparator_min = 1;
			if (s->volume_max > s->volume_max_prev)
			{
				if (s->comparator_max)
				{
					s->comparator_max = 0;
					if (s->volume_min <= s->volume_min_prev)
						s->volume_min_prev = s->volume_min * SUSTAIN_FACTOR;
					s->volume_min = s->volume_min_prev;
					s->period_divider_tmp++;
				}
			}
			if (s->period_divider_tmp < 2)
			{
				s->prev_tmp = s->samplecounter;
				s->volume_tmp = 0;
				s->accuracy_tmp = 0;
				s->measure = 1;
			}
			if (s->accuracy_tmp > PERIOD_ACCURACY_MIN)
			{
				s->ready = 1;
				s->measure = 0;
			}
		}
		
		if (ADC < s->volume_min)
		{
			s->volume_min = ADC;
			if (s->volume_min < s->volume_min_prev)
			{
				s->comparator_max = 1;
				if (s->comparator_min)
				{
					s->comparator_min = 0;
					if (s->volume_max >= s->volume_max_prev)
						s->volume_max_prev = s->volume_max * SUSTAIN_FACTOR;
					s->volume_max = s->volume_max_prev;
				}
				if (s->ready)
				{
					semaphore_wait(s->sem);
					s->ready = 0;
					s->serialno++;
					s->prev = s->prev_tmp;
					s->cur = s->cur_tmp;
					s->volume = s->volume_tmp;
					s->accuracy = s->accuracy_tmp;
					s->period_divider = s->period_divider_tmp;
					s->period_divider_tmp = 0;
					semaphore_post(s->sem);
				}
			}
		}
	}
	s->overload = 0;
}

void freqvolmeter_init()
{
	extern_process = &adcprocess;
	memset(&sensors[0], 0x0, sizeof(sensor));
	if (SAMPLERATE == 96000) PERIOD_ACCURACY_MIN = 512;
	if (SAMPLERATE == 192000) PERIOD_ACCURACY_MIN = 1024;
}

sensor_value* read_sensor(sensor* s, sensor_value* buf)
{
		semaphore_wait(s->sem);
		buf->period =  (s->period_divider > 1)?
			(s->cur - s->prev) * 1000000 /
			((s->period_divider - 1) * SAMPLERATE) 
			: 0;
		buf->volume = (s->accuracy != 0)?
			s->volume / s->accuracy : 0;
		buf->accuracy = s->accuracy;
		buf->serialno = s->serialno;
		buf->period_divider = s->period_divider;
		semaphore_post(s->sem);
		buf->sens = s;
		buf->errors = 0;
		if (s->samplecounter - s->prev > PERIOD_TIMEOUT)
			buf->errors =  ETIMEOUT;
		if (buf->period > PERIOD_MAX || buf->period <= PERIOD_MIN)
			buf->errors =  EDIRTY;

		return buf;
}

void reset_sensor(sensor* s)
{
	s->volume_max_prev = 0;
	s->volume_min_prev = 0;
	s->volume_max = 0;
	s->volume_min = 0;
}

