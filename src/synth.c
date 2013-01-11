/**
 ******************************************************************************
 * @file     synth.c
 * @author  Xavier Halgand
 * @version
 * @date    december 2012
 * @brief A sawtooth oscillator with light fixed vibrato
 * goes through a very basic envelope (just Decay)
 * then an echo (feedback delay) and a phaser.
 * The oscillator is an alias-free minBLEP based generator :
 * Thanks to Sean Bolton (for his blepvco), Fons Adriaensen, Eli Brandt, ... for their work !
 ******************************************************************************
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307, USA.
 */



#include "synth.h"


//-------------------------Decay Envelope-------------------------------------
static float_t nextSawEnv(uint8_t *trigger, uint8_t amp, float *t, float envTime)
{
	float val;

	if (*trigger == ON) // envelope is trigged !
	{
		*trigger = OFF;
		*t = 0.f;
		(*t) += sampleT;
		return val = (float)amp / 255.f;
	}
	else // no trigger
	{
		if ((*t) >= envTime)
		{
			return val = 0.f;
		}
		else
		{
			val = (float)amp / 255.f * (1.f - (*t)/envTime);
			(*t) += sampleT;
			return val;
		}
	}
}

//-------------------------this is magic...---------------------------------------

static void  place_step_dd(float_t *buffer, int index, float_t phase, float_t w, float_t scale)
{
	float_t r;
	int i;

	r = MINBLEP_PHASES * phase / w;
	i = lrintf(r - 0.5f);
	r -= (float_t)i;
	i &= MINBLEP_PHASE_MASK;  /* extreme modulation can cause i to be out-of-range */
	/* this would be better than the above, but more expensive:
	 *  while (i < 0) {
	 *    i += MINBLEP_PHASES;
	 *    index++;
	 *  }
	 */

	while (i < MINBLEP_PHASES * STEP_DD_PULSE_LENGTH) {
		buffer[index] += scale * (step_dd_table[i].value + r * step_dd_table[i].delta);
		i += MINBLEP_PHASES;
		index++;
	}
}
//--------------------------Prepare the oscillator----------------------------------------

void
Synth_Init (void)
{
	_init = 1;
	_z = 0.0f;
	_j = 0;
	memset (_f, 0, (FILLEN + STEP_DD_PULSE_LENGTH) * sizeof (float_t));
}

//---------------------------Compute the samples---------------------------------
void
make_sound (uint16_t offset, uint16_t len)
{
	uint16_t	j, n;
	uint16_t	*outp;
	float_t  		a, p, t, w, dw, z, ze, y, dy, freq;
	uint16_t	value;

	outp    = audiobuff + offset;
	freq = f1;
	p = _p;  /* phase [0, 1) */
	w = _w;  /* phase increment */
	z = _z;  /* low pass filter state */
	j = _j;  /* index into buffer _f */

	if (_init)
	{
		p = 0.5f;
		w = freq / SAMPLERATE;
		if (w < 1e-5) w = 1e-5;
		if (w > 0.5) w = 0.5;
		/* if we valued alias-free startup over low startup time, we could do:
		 *   p -= w;
		 *   place_slope_dd(_f, j, 0.0f, w, -1.0f); */
		_init = 0;
	}

	//a = 0.2 + 0.8 * _port [FILT][0];
	// adjust lowpass filter :
	a = 0.9f;

	do
	{
		n = 16;  //  the osc freq control is undersampled 16 times

		/***** insert a vibrato  ******/
		phase2Step = _2PI * 16.f * f2 / SAMPLERATE;
		phase2 += phase2Step;
		phase2 = (phase2 > _2PI) ? phase2 - _2PI : phase2;
		   /* modulate freq by a sine   */
		freq = f1 * (1 + VIB_AMP * sinf(phase2));

		len -= n;

		t = freq / SAMPLERATE;
		if (t < 1e-5) t = 1e-5;
		if (t > 0.5) t = 0.5;
		dw = (t - w) / n;

		while (n--) // for each sample...
		{
			w += dw;
			p += w;

			if (p >= 1.0f)
			{  /* normal phase reset */
				p -= 1.0f;
				place_step_dd(_f, j, p, w, 1.0f);
			}

			_f[j + DD_SAMPLE_DELAY] += 0.5f - p;
			z += a * (_f[j] - z);

			/****   insert enveloppe  ****/
			ze = z * nextSawEnv(&envTrigger, envAmp, &envPhase, envTime);

			/**** insert delay effect  ****/
			// (*readpos) : delayed sample read at the output of the delay line
			dy = (1.f - coeff_a1)*(*readpos) + coeff_a1 * old_dy; // apply lowpass filter in the loop
			old_dy = dy;
			y = pass *  ze + fdb*dy;
			y = (y > 1.0f) ? 1.0f : y ; //clip too loud samples
			y = (y < -1.0f) ? -1.0f : y ;
			*writepos = y; // write new computed sample at the input of the delay line
				/* update the delay line pointers : */
			writepos++;
			readpos++;
			if ((writepos - delayline) >= DELAYLINE_LEN)
				writepos = delayline; // wrap pointer

			if ((readpos - delayline) >= DELAYLINE_LEN)
				readpos = delayline;  // wrap pointer

			/**** phaser effect *****/
			y = PhaserProcess(y);

			/**** prevent overflow ****/
			y = (y > 1.0f) ? 1.0f : y ; //clip too loud samples
			y = (y < -1.0f) ? -1.0f : y ;

			/****** let's hear the new sample *******/
			value = (uint16_t)((int16_t)((32767.0f) * y ));
			*outp++ = value; // left channel sample
			*outp++ = value; // right channel sample

			if (++j == FILLEN)
			{
				j = 0;
				memcpy (_f, _f + FILLEN, STEP_DD_PULSE_LENGTH * sizeof (float_t));
				memset (_f + STEP_DD_PULSE_LENGTH, 0,  FILLEN * sizeof (float_t));
			}
		}
	}
	while (len);

	_p = p;
	_w = w;
	_z = z;
	_j = j;
}
//---------------------------------------------------------------------------
