/**
 ******************************************************************************
 * @file     pitchTables.c
 * @author  Xavier Halgand
 * @version
 * @date    december 2012
 * @brief
 *
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



#include "pitchTables.h"

//This array contains the note frequencies corresponding to MIDI note values.


const uint16_t MIDINOTES_FREQ_LUT[128] =
{
		8,9,9,10,10,11,12,12,13,14,15,15,
		16,17,18,19,21,22,23,24,26,28,29,31,
/*		C		C#		D		D#		E		F		F#		G		G#		A		A#		B	*/
		33,		35,		37,		39,		41,		44,		46,		49,		52,		55,		58,		62,
		65,		69,		73,		78,		82,		87,		92,		98,		104,	110,	117,	123,
		131,	139,	147,	156,	165,	175,	185,	196,	208,	220,	233,	247,
		262,	277,	294,	311,	330,	349,	370,	392,	415,	440,	466,	494,
		523,	554,	587,	622,	659,	698,	740,	784,	831,	880,	932,	988,
		1047,	1109,	1175,	1245,	1319,	1397,	1480,	1568,	1661,	1760,	1865,	1976,
		2093,	2217,	2349,	2489,	2637,	2794,	2960,	3136,	3322,	3520,	3729,	3951,
		4186,	4435,	4699,	4978,	5274,	5588,	5920,	6272,	6645,	7040,	7459,	7902,
		8372,	8870,	9397,	9956,	10548,	11175,	11840,	12544
};



/*  do r� mi sol la  */
const uint16_t scale1[39] =
{
		65,				73,				82,				98,			110,
		131,			147,			165,			196,		220,
		262,			294,			330,			392,		440,
		523,			587,			659,			784,		880,
		1047,			1175,			1319,			1568,		1760,
		2093,			2349,			2637,			3136,		3520,
		4186,			4699,			5274,			6272,		7040,
		8372,			9397,			10548,			12544
};

/*  do r� mib solb sol la  */
const uint16_t scale2[47] =
{
		65,				73,		78,						92,		98,			110,
		131,			147,	156,					185,	196,		220,
		262,			294,	311,					370,	392,		440,
		523,			587,	622,					740,	784,		880,
		1047,			1175,	1245,					1480,	1568,		1760,
		2093,			2349,	2489,					2960,	3136,		3520,
		4186,			4699,	4978,					5920,	6272,		7040,
		8372,			9397,	9956,					11840,	12544
};
/*   do r� mi fa sol la  */
const uint16_t scale3[18] =
{
		262,			294,			330,	349,		392,		440,
		523,			587,			659,	698,		784,		880,
		1047,			1175,			1319,	1397,		1568,		1760
};

/* do r� fa la  */
const uint16_t scale4[12] =
{
		262,		294,				349,					440,
		523,		587,				698,					880,
		1047,		1175,				1397,					1760
};

/* Gamme par ton : do# r�# fa sol la si */
const uint16_t scale5[42] =
{
/*		C		C#		D		D#		E		F		F#		G		G#		A		A#		B	*/
				69,				78,				87,				98,				110,			123,
				139,			156,			175,			196,			220,			247,
				277,			311,			349,			392,			440,			494,
				554,			622,			698,			784,			880,			988,
				1109,			1245,			1397,			1568,			1760,			1976,
				2217,			2489,			2794,			3136,			3520,			3951,
				4435,			4978,			5588,			6272,			7040,			7902
};

/* la mineur m�lodique */
const uint16_t scale6[42] =
{
/*		C		C#		D		D#		E		F		F#		G		G#		A		A#		B	*/
		131,			147,			165,	175,					208,	220,			247,
		262,			294,			330,	349,					415,	440,			494,
		523,			587,			659,	698,					831,	880,			988,
		1047,			1175,			1319,	1397,					1661,	1760,			1976,
		2093,			2349,			2637,	2794,					3322,	3520,			3951,
		4186,			4699,			5274,	5588,					6645,	7040,			7902
};


/*  do et sol */
const uint16_t scale7[11] =
{
		98, 131, 196, 262, 392, 523, 784, 1047, 1568, 2093, 3136
};

/*  do et la */
const uint16_t scale8[12] =
{
		65, 110, 131, 220, 262, 440, 523, 880, 1047, 1760, 2093, 3520
};
