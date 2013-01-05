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

#include "stm32f4xx.h"
#include "stm32f4xx_conf.h"
#include "stm32f4_discovery.h"
#include "stm32f4_discovery_audio_codec.h"

#include <stdio.h>
#include "stm32f4xx_it.h"
    

 /*--------------------- Defines for the Audio process------------------------ */

#define _2PI                    6.283185307f
#define _PI						3.14159265f
#define _INVPI					0.3183098861f
#define SAMPLERATE              48000
#define FREQ1                   440.0f   // default carrier frequency
#define FREQ2                   6.0f     // default modulation frequency
#define VOL                     70
#define MAXVOL					80
#define BUFF_LEN_DIV4           160 // XX ms latency at 48kHz
#define BUFF_LEN_DIV2           320
#define BUFF_LEN                640  /* Audio buffer length : count in 16bits half-words */
#define DELAYLINE_LEN           24000  // max delay in samples (0.5 seconds)
#define DELAY                   13000  // init delay (in samples)
#define MIN_DELAY				250		// min delay, in samples
#define DELAY_VOLUME            0.1f   // 0.3f
#define FEEDB                   0.4f   //0.4f
#define ON                      1
#define OFF                     0
#define ADCMAX					63.f
#define DELTA					3
#define DELAY_1					3  // time to wait between controls reading (in ticks)
#define PARAMDEF				50
#define MIN_ARR					2000 // for max tempo
#define MAX_ARR					34000 // for min tempo
#define ADC_MAX					63.f
#define ENV_TIME				0.3f


/* Exported functions ------------------------------------------------------- */

void 		TimingDelay_Decrement(void);
void 		AudioBuffer_Init(void);
void 		AudioBuffer_LoadWave(void);
uint32_t 	Codec_TIMEOUT_UserCallback(void);
float 		randomNum(void);


#endif /* __MAIN_H */

/************************END OF FILE****/
