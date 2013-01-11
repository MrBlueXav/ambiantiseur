/**
  ******************************************************************************
  * @file    main.h 
  * @author  
  * @version 
  * @date    
  * @brief   Header for main.c
  ******************************************************************************
*/
  
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "stm32f4xx.h"
#include "stm32f4xx_conf.h"
#include "stm32f4_discovery.h"
#include "stm32f4_discovery_audio_codec.h"
#include "stm32f4xx_it.h"
#include "minblep_tables.h"
#include "synth.h"
#include "phaser2.h"

 /*--------------------- Global Defines ------------------------ */

#define _2PI                    6.283185307f
#define _PI						3.14159265f
#define _INVPI					0.3183098861f

#define SAMPLERATE              48000
#define BUFF_LEN_DIV4           160 // number of samples <==> XX ms latency at 48kHz
#define BUFF_LEN_DIV2           320
#define BUFF_LEN                640  // Audio buffer length : count in 16bits half-words

#define FREQ1                   440.0f   // default carrier frequency
#define FREQ2                   4.0f     // vibrato frequency
#define VIB_AMP					0.01f // vibrato amplitude
#define VOL                     70
#define MAXVOL					80


#define DELAYLINE_LEN           30000  // max delay in samples (0.625 seconds)
#define DELAY                   13000  // init delay (in samples)
#define MIN_DELAY				250		// min delay, in samples
#define FEEDB                   0.4f

#define ON                      1
#define OFF                     0

#define ADCMAX					1023.f
#define DELTA					20 // marge d'accrochage des pots et marge de bruit
#define PARAMDEF				920
#define DELAY_1					4  // time to wait between controls reading (in ticks)
#define LIM1					10 // limites des plages de contrôle pour la sélection des gammes (pot2)
#define LIM2					20 // limites des plages de contrôle pour la sélection des gammes (pot2)
#define LIM3					50 // limites des plages de contrôle pour la sélection des gammes (pot2)
#define LIM4					100 // limites des plages de contrôle pour la sélection des gammes (pot2)
#define LIM5					300 // limites des plages de contrôle pour la sélection des gammes (pot2)
#define LIM6					500 // limites des plages de contrôle pour la sélection des gammes (pot2)
#define LIM7					800 // ne pas dépasser ADCMAX !!!

#define MIN_ARR					2000 // for max tempo
#define MAX_ARR					34000 // for min tempo
#define ENV_TIME				0.3f
#define FILLEN 					256

/* -------------Global variables ---------------------------------------------*/

extern uint16_t        audiobuff[];  // The circular audio buffer
extern float_t           sampleT;

/* Envelope variables  */
extern uint8_t			envTrigger, envAmp;
extern float_t			envPhase;
extern float_t			envTime;

/*  Sequencer variables  */
extern uint32_t 		seqIndex;
extern uint32_t			seq1Number;

/*   tempo generator (timer 6) variables   */
extern TIM_TimeBaseInitTypeDef  	TIM_TimeBaseStructure;
extern uint16_t 					PrescalerValue;
extern uint16_t 					reloadValue;

/*   Oscillator variables  */
extern float_t    pass;
extern float_t    phase2, phase2Step;
extern float_t   f1, f2, freq;
extern float_t   _p, _w, _z;
extern float_t   _f [];
extern int     _j, _init;

/* panel (switch and pots) variables */
extern uint16_t		ADC1ConvertedValues[] ;
extern uint16_t		param1, param2, param3, param4;
extern uint16_t		old_param3;
extern uint16_t		param1b, param2b, param3b, param4b;
extern uint8_t		potSet;
extern uint8_t		accroche1, accroche2, accroche3, accroche4;
extern uint16_t		delay1;

/* Delay effect variables  */
extern float_t           delayline[];
extern float_t           delayVol;
extern float_t           *readpos; // output pointer of delay line
extern float_t           *writepos; // input pointer of delay line
extern float_t			coeff_a1; // coeff for the one pole low-pass filter in the feedback loop
// coeff_a1 is between 0 and 1, 0 : no filtering, 1 : heavy filtering
extern float_t			old_dy; //previous delayed sample
extern float_t           fdb;


/* Exported functions ------------------------------------------------------- */

uint32_t 	Codec_TIMEOUT_UserCallback(void);
float 		randomNum(void);


#endif /* __MAIN_H */

/************************END OF FILE****/
