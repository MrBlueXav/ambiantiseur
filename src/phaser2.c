/**
 ******************************************************************************
 * @file     phaser2.c
 * @author  Xavier Halgand
 * @version
 * @date    december 2012
 * @brief

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


#include "main.h"
#include "phaser2.h"

/*This defines the phaser stages
that is the number of allpass filters
*/
#define PH_STAGES 6

static float old[PH_STAGES];
static float f_min, f_max;
static float swrate;
static float dry, wet ;
static float _dmin, _dmax; //range
static float _fb; //feedback
static float _lfoPhase;
static float _lfoInc;
static float _a1;
static float _zm1;

void PhaserInit(void)
{
	f_min = 200.f;
	f_max = 1700.f;
	swrate = 0.1f;
	_fb = 0.7f;
	dry = 0.7f;
	wet = 0.3f;

	_dmin = 2 * f_min / SAMPLERATE;
	_dmax = 2 * f_max / SAMPLERATE;
	_lfoInc = _2PI * swrate / SAMPLERATE;
}

void PhaserRate(float rate)
{
	swrate = rate;
	_lfoInc = _2PI * swrate / SAMPLERATE;
}

void PhaserFeedback(float fdb)
{
	_fb = fdb;
}
float allpass(float yin, int ind)
{
        float yout;

        yout = - yin * _a1 + old[ind];
        old[ind] = yout * _a1 + yin;
        return yout;
}


float PhaserProcess(float xin)
{
	float yout;
	int i;

	//calculate and update phaser sweep lfo...
	float d  = _dmin + (_dmax - _dmin) * ((sinf(_lfoPhase) + 1.f)*0.5f);
	_lfoPhase += _lfoInc;
	if( _lfoPhase >= _2PI ) 	_lfoPhase -= _2PI;

	//update filter coeffs
	_a1 = (1.f - d) / (1.f + d);

	//calculate output

	yout = allpass(xin + _zm1 * _fb, 0);

	for(i = 1; i < PH_STAGES; i++)
	{
		yout = allpass(yout, i);
	}
	_zm1 = yout;

	yout = dry * xin + wet * yout;

	return yout;
}

