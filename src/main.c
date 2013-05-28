/**
 ******************************************************************************
 * @file     main.c
 * @author  Xavier Halgand (& many other people...)
 * @version
 * @date    january 2013
 * @brief
 *
 STM32F4 Discovery board open source project.

"Ambiantiseur" ( Ambientizer in english) is a very basic synthesizer
which can produce semi-randomly generated musical sequences with a few effects (delay and phaser).
Demo here : http://www.youtube.com/watch?v=fMWcXrCg1iw

Four potentiometers and one switch allow the user to control the machine.
The two position switch allow you to modify 2 x 4 = 8 parameters :
Switch = 0 ->
	pot1 : tempo
	pot2 : notes scales
	pot3 : delay time
	pot4 : delay feedback
Switch = 1 ->
	pot1 : note duration (decay time)
	pot2 : phaser rate
	pot3 : phaser feedback
	pot4 : master volume

When switch is toggled, parameter will be adjusted only when the potentiometer is moved and reaches previous value.
The LEDS light on when the pot is at real position.
That is done for avoiding brutal value jumps. (Could be improved)

The tone generator is an anti-aliased minBLEP sawtooth oscillator with light vibrato.

Wirings :
Switch and pots are 3V board powered.
	switch : PA1
	pot1   : PB1
	pot2   : PC1
	pot3   : PC2
	pot4   : PC4
Three additional pots or switches can be wired on PA2, PA3, PB0. Their values appear in ADC1ConvertedValues[1..2..3] array.

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

/*----------------------------- Includes --------------------------------------*/

#include "main.h"

/******************************************************************************/
/* ADC CDR register base address */
#define ADC1_DR_ADDRESS               ((uint32_t)0x4001204C)

/* ===============  Global variables =========================================*/

uint16_t        audiobuff[BUFF_LEN];  // The circular audio buffer
float_t           sampleT;

/* Envelope variables  */
uint8_t			envTrigger, envAmp;
float_t			envPhase;
float_t			envTime = ENV_TIME;

/*  Sequencer variables  */
uint32_t 		seqIndex;
uint32_t		seq1Number = 6;

/*   tempo generator (timer 6) variables   */
TIM_TimeBaseInitTypeDef  	TIM_TimeBaseStructure;
uint16_t 					PrescalerValue = 349;  /* 21000000 / 60000  - 1  */
uint16_t 					reloadValue = 60000;

/*   Oscillator variables  */
float_t    pass = 0.8f;
float_t    phase2, phase2Step;
float_t   f1 = FREQ1 , f2 = FREQ2 , freq;
float_t   _p, _w, _z;
float_t   _f [FILLEN + STEP_DD_PULSE_LENGTH];
int     _j, _init;

/* panel (switch and pots) variables */
uint16_t 		ADC1ConvertedValues[8] ;
uint16_t		param1=PARAMDEF, param2=PARAMDEF, param3=PARAMDEF, param4=PARAMDEF;
uint16_t		old_param3=PARAMDEF;
uint16_t		param1b=PARAMDEF, param2b=PARAMDEF, param3b=PARAMDEF, param4b=PARAMDEF;
uint8_t			potSet;
uint8_t			accroche1, accroche2, accroche3, accroche4;
uint16_t		delay1 = DELAY_1;


/* Delay effect variables  */
float_t           delayline[DELAYLINE_LEN + 2 ];
float_t           *readpos; // output pointer of delay line
float_t           *writepos; // input pointer of delay line
float_t			coeff_a1 = 0.6f; // coeff for the one pole low-pass filter in the feedback loop
// coeff_a1 is between 0 and 1, 0 : no filtering, 1 : heavy filtering
float_t			old_dy; //previous delayed sample
float_t           fdb = FEEDB;



/* ----- function prototypes -----------------------------------------------*/

static void ADC1_start(void);
static void TIM6_Config_and_start(void);

/*=============================== MAIN ======================================
==============================================================================*/
/**
 * @brief  Main program.
 * @param  None
 * @retval None
 */
int main(void)
{
	/* Initialize LEDS */
	STM_EVAL_LEDInit(LED3); // orange LED
	STM_EVAL_LEDInit(LED4); // green LED
	STM_EVAL_LEDInit(LED5); // red LED
	STM_EVAL_LEDInit(LED6); // blue LED


	/* Initialize User Button */
	STM_EVAL_PBInit(BUTTON_USER, BUTTON_MODE_GPIO);

	/* Initialise the on-chip random number generator ! */
	RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_RNG, ENABLE);
	RNG_Cmd(ENABLE);

	sampleT = 1.f / SAMPLERATE;
	envPhase = 0.0f;

	ADC1_start();

	if (ADC1ConvertedValues[0] < (ADCMAX / 2) ) potSet = 0; else potSet = 1;

	TIM6_Config_and_start();

	if (SysTick_Config(SystemCoreClock / 50))  // 20 ms tick (50Hz)
	{
		/* Capture error */
		while (1);
	}

	/* initialize pointers positions for delay effect */
	readpos = delayline;
	writepos = delayline + DELAY;

	Synth_Init();

	PhaserInit();

	EVAL_AUDIO_Init( OUTPUT_DEVICE_AUTO, VOL, SAMPLERATE);
	EVAL_AUDIO_Play((uint16_t*)audiobuff, BUFF_LEN);


	while (1)
	{
		//Interrupts handlers are doing their job...;

	}
}
/*============================== End of main ===================================
==============================================================================*/


/**
 * @brief  Basic management of the timeout situation.
 * @param  None
 * @retval None
 */
uint32_t Codec_TIMEOUT_UserCallback(void)
{
	//STM_EVAL_LEDOn(LED5); /*  alert : red LED !  */
	return (0);
}
//---------------------------------------------------------------------------
/**
 * @brief  Manages the DMA Half Transfer complete interrupt.
 * @param  None
 * @retval None
 */
void EVAL_AUDIO_HalfTransfer_CallBack(uint32_t pBuffer, uint32_t Size)
{
	/* Generally this interrupt routine is used to load the buffer when
  a streaming scheme is used: When first Half buffer is already transferred load
  the new data to the first half of buffer while DMA is transferring data from
  the second half. And when Transfer complete occurs, load the second half of
  the buffer while the DMA is transferring from the first half ... */

	//STM_EVAL_LEDToggle(LED6);
	make_sound(0, BUFF_LEN_DIV4);
	//STM_EVAL_LEDToggle(LED6);

}
//---------------------------------------------------------------------------
/**
 * @brief  Manages the DMA Complete Transfer complete interrupt.
 * @param  None
 * @retval None
 */
void EVAL_AUDIO_TransferComplete_CallBack(uint32_t pBuffer, uint32_t Size)
{

	//STM_EVAL_LEDToggle(LED6);
	make_sound(BUFF_LEN_DIV2, BUFF_LEN_DIV4);
	//STM_EVAL_LEDToggle(LED6);
}


//---------------------------------------------------------------------------
/** NOT USED !
 * @brief  Get next data sample callback
 * @param  None
 * @retval Next data sample to be sent
 */
uint16_t EVAL_AUDIO_GetSampleCallBack(void)
{
	return 0;
}


//---------------------------------------------------------------------------
/**************
 * returns a random float between 0 and 1
 *****************/
float_t randomNum(void)
{
	float_t random = 1.0f;
	if (RNG_GetFlagStatus(RNG_FLAG_DRDY) == SET)
	{
		random = (float_t)(RNG_GetRandomNumber()/4294967294.0f);
	}
	return random;
}


//---------------------------------------------------------------------------
/**
 * @brief  Configure and start ADC1.
 * @param  None
 * @retval None
 */
void ADC1_start(void)
{
	ADC_InitTypeDef       ADC_InitStructure;
	ADC_CommonInitTypeDef ADC_CommonInitStructure;
	DMA_InitTypeDef       DMA_InitStructure;
	GPIO_InitTypeDef      GPIO_A_InitStructure;
	GPIO_InitTypeDef      GPIO_B_InitStructure;
	GPIO_InitTypeDef      GPIO_C_InitStructure;

	/* Deinitializes the ADC peripheral registers */
	ADC_DeInit();

	/* Enable the GPIOs' Clock , ADC1 Periph Clock and DMA1 clock   */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2 | RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);


	/* DMA1 Stream0 channel0 configuration  */
	DMA_InitStructure.DMA_Channel = DMA_Channel_0;
	DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_ADDRESS;
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&ADC1ConvertedValues;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
	DMA_InitStructure.DMA_BufferSize = 8;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(DMA2_Stream0, &DMA_InitStructure);
	DMA_Cmd(DMA2_Stream0, ENABLE);

	/* Configure ADC1 Channels 1, 2, 3 pins as analog input : PA1, PA2, PA3 ***********************/
	GPIO_A_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
	GPIO_A_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_A_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
	GPIO_Init(GPIOA, &GPIO_A_InitStructure);

	/* Configure ADC1 Channels 8, 9 pins as analog input : PB0, PB1 ***********************/
	GPIO_B_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
	GPIO_B_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_B_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
	GPIO_Init(GPIOB, &GPIO_B_InitStructure);

	/* Configure ADC1 Channels 11, 12, 14 pins as analog input : PC1, PC2, PC4 ***********************/
	GPIO_C_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_4;
	GPIO_C_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_C_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
	GPIO_Init(GPIOC, &GPIO_C_InitStructure);

	/* ADC Common Init **********************************************************/
	ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
	ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
	ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_20Cycles;
	ADC_CommonInit(&ADC_CommonInitStructure);

	/* ADC1 Init ****************************************************************/
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_10b;
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfConversion = 8;

	ADC_InitStructure.ADC_ExternalTrigConv = 0;

	ADC_Init(ADC1, &ADC_InitStructure);

	/* ADC1 regular channels configuration -- ADCx->SMPR1,SMPR2 et ADCx->SQR1,SQR2,SQR3*/

	ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_15Cycles );
	ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 2, ADC_SampleTime_15Cycles );
	ADC_RegularChannelConfig(ADC1, ADC_Channel_3, 3, ADC_SampleTime_15Cycles );
	ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 4, ADC_SampleTime_15Cycles );
	ADC_RegularChannelConfig(ADC1, ADC_Channel_9, 5, ADC_SampleTime_15Cycles );
	ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 6, ADC_SampleTime_15Cycles );
	ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 7, ADC_SampleTime_15Cycles );
	ADC_RegularChannelConfig(ADC1, ADC_Channel_14, 8, ADC_SampleTime_15Cycles );

	/* Enable DMA request after last transfer (Single-ADC mode) */
	ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);

	/* Enable ADC1 DMA */
	ADC_DMACmd(ADC1, ENABLE);

	/* Enable ADC1 */
	ADC_Cmd(ADC1, ENABLE);

	ADC_ContinuousModeCmd(ADC1, ENABLE);
	/* Start ADC1 Software Conversion */
	ADC_SoftwareStartConv(ADC1);
}
/*****************************************************************************************/

/**
 * @brief  Configure and start the TIMER 6.
 * @param  None
 * @retval None
 */
void TIM6_Config_and_start(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	/* TIM6 clock enable */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);

	TIM_DeInit(TIM6);

	/* Enable the TIM6 global Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = TIM6_DAC_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Time base configuration */
	TIM_TimeBaseStructure.TIM_Period = 65535;
	TIM_TimeBaseStructure.TIM_Prescaler = 0;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; // NA
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

	TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);

	/* Prescaler configuration */
	TIM_PrescalerConfig(TIM6, PrescalerValue, TIM_PSCReloadMode_Immediate);

	TIM_SetAutoreload(TIM6, reloadValue);

	TIM_UpdateDisableConfig(TIM6, DISABLE);

	TIM_UpdateRequestConfig(TIM6, TIM_UpdateSource_Regular);

	TIM_ARRPreloadConfig(TIM6, ENABLE);

	TIM_SelectOnePulseMode(TIM6, TIM_OPMode_Repetitive);

	/* TIM Interrupts enable */
	TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE);

	/* TIM6 enable counter */
	TIM_Cmd(TIM6, ENABLE);
}

/***********************END OF FILE**********************************************/
