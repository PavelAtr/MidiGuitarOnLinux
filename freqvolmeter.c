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
bool_t overload;

void adcperform(sensor* s, volume_t ADC)
{
		volume_t ADC_abs = abs(ADC);
		s->samplecounter++;
		
		s->volume_approx = (ADC > s->volume_approx)? ADC : s->volume_approx;
		s->accuracy_approx++;

		if (ADC > 0)
		{
			s->comparator_zero = 1;
		}
		else if (s->comparator_zero)
		{
			s->comparator_zero = 0;
			if (s->accuracy_approx >= PERIOD_ACCURACY_MIN * 4)
			{
				s->volume_max_prev = (s->volume_approx < s->volume_max_prev)? s->volume_approx : s->volume_max_prev;
				s->volume_min_prev = (s->volume_approx > s->volume_min_prev)? s->volume_approx : s->volume_min_prev;
				s->volume_approx = 0;
				s->accuracy_approx = 0;
			}
		}
		if (ADC >= s->volume_max_prev * COMPARATOR_TRESOLD)
		{
		
			s->volume_min_prev = (s->volume_min <= s->volume_min_prev * COMPARATOR_TRESOLD) ?
				s->volume_min : s->volume_min_prev;
			s->comparator_min = 1;
			s->volume_min = 0;
			if (s->comparator_max)
			{
				s->comparator_max = 0;

				s->period_divider_tmp++;
				if (s->period_divider_tmp < 2)
				{
					s->volume_tmp = 0;
					s->accuracy_tmp = 0;
					s->measure = 1;
				}
			}
			if (ADC >= s->volume_max)
			{
				s->volume_max = ADC;
				s->cur_tmp = s->samplecounter;
				if (s->period_divider_tmp == 1)
				{
					s->prev_tmp = s->samplecounter;
				}
				if (s->accuracy_tmp >= PERIOD_ACCURACY_MIN)
				{
					s->ready = 1;
					s->measure = 0;
				}
			}
		}
		if (ADC <= s->volume_min_prev * COMPARATOR_TRESOLD)
		{
			s->comparator_max = 1;
			s->volume_max_prev = (s->volume_max >= s->volume_max_prev * COMPARATOR_TRESOLD) ?
				s->volume_max : s->volume_max_prev;
			s->volume_max = 0;

			if (s->comparator_min)
			{
				s->comparator_min = 0;
			}
			if (ADC <= s->volume_min)
			{
				s->volume_min = ADC;
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
				s->volume_max_redy = s->volume_max;
				s->approx_redy = s->volume_max_prev;
				s->period_divider_tmp = 0;
				semaphore_post(s->sem);
			}
		}
		if (s->measure)
		{
			s->volume_tmp += ADC_abs;
			s->accuracy_tmp++;
		}
}

void adcprocess()
{
	if (overload)
		printf("OVERLOAD!\r\n");
	overload = 1;

	for (jack_nframes_t j = 0; j < inputs[0].ports_nframes; j++)
		for (ucounter_t i = 0; i < CHANNEL_NUM; i++)
			adcperform(&sensors[i], ADC_MAX * inputs[i].inputbuf[j] - ADC_ZERO_SIN);
	
	overload = 0;
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
		buf->approx = s->approx_redy;
		buf->volumemax = s->volume_max_redy;
		semaphore_post(s->sem);
		buf->sens = s;
		buf->errors = 0;
		if (s->samplecounter - s->prev > PERIOD_TIMEOUT)
			buf->errors =  ETIMEOUT;
		if (buf->period > PERIOD_MAX || buf->period <= PERIOD_MIN)
			buf->errors =  EDIRTY;

		return buf;
}
