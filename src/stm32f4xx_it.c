/**
 ******************************************************************************
 * @file    stm32f4xx_it.c
 * @author  Xavier Halgand & MCD Application Team
 * @version
 * @date    december 2012
 * @brief   Main Interrupt Service Routines.
 *          This file provides template for all exceptions handler and
 *          peripherals interrupt service routine.
 ******************************************************************************
 * @attention
 *
 * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
 * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
 * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
 * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
 * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
 * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *
 * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "phaser2.h"

/* ---------------------------------------------------------------------------*/

extern float 			f1 ;
extern uint16_t 		ADC1ConvertedValues[8] ;
//extern __IO uint32_t 	TimingDelay ;
extern float         	fdb;
extern float           *readpos; // output pointer of delay line
extern float           *writepos; // input pointer of delay line
extern float           delayline[DELAYLINE_LEN + 10];
extern uint16_t			delay1;

//extern const uint16_t 	MIDINOTES_FREQ_LUT[128];
extern const uint16_t 	scale1[39];
extern const uint16_t 	scale2[47];
extern const uint16_t 	scale3[18];
extern const uint16_t 	scale4[12];
extern const uint16_t 	scale5[42];
extern const uint16_t 	scale6[42];

extern uint8_t			envTrigger, envAmp;
extern float			envTime;

/*  Extern Sequencer variables  */
extern const uint8_t 	seq1[64][16];
extern uint32_t 		seqIndex;
extern uint32_t			seq1Number;

extern uint8_t			potSet;
extern uint8_t			accroche1, accroche2, accroche3, accroche4;
extern uint16_t			param1, param2, param3, param4;
extern uint16_t			param1b, param2b, param3b, param4b;

extern uint16_t 		reloadValue;


/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
 * @brief   This function handles NMI exception.
 * @param  None
 * @retval None
 */
void NMI_Handler(void)
{
}

/**
 * @brief  This function handles Hard Fault exception.
 * @param  None
 * @retval None
 */
void HardFault_Handler(void)
{
	/* Go to infinite loop when Hard Fault exception occurs */
	while (1)
	{
	}
}

/**
 * @brief  This function handles Memory Manage exception.
 * @param  None
 * @retval None
 */
void MemManage_Handler(void)
{
	/* Go to infinite loop when Memory Manage exception occurs */
	while (1)
	{
	}
}

/**
 * @brief  This function handles Bus Fault exception.
 * @param  None
 * @retval None
 */
void BusFault_Handler(void)
{
	/* Go to infinite loop when Bus Fault exception occurs */
	while (1)
	{
	}
}

/**
 * @brief  This function handles Usage Fault exception.
 * @param  None
 * @retval None
 */
void UsageFault_Handler(void)
{
	/* Go to infinite loop when Usage Fault exception occurs */
	while (1)
	{
	}
}

/**
 * @brief  This function handles SVCall exception.
 * @param  None
 * @retval None
 */
void SVC_Handler(void)
{
}

/**
 * @brief  This function handles Debug Monitor exception.
 * @param  None
 * @retval None
 */
void DebugMon_Handler(void)
{
}

/**
 * @brief  This function handles PendSVC exception.
 * @param  None
 * @retval None
 */
void PendSV_Handler(void)
{
}

/**
 * @brief  This function handles SysTick Handler.
 * Every DELAY_1 ticks this function reads the panel potentiometers and switches
 * and updates the parameters.
 * @param  None
 * @retval None
 */
void SysTick_Handler(void)
{
	uint32_t	shift;
	uint16_t 	switchVal;
	uint8_t		val;
	float 		*pos;

	delay1--;

	if (delay1 <= 0) // read now the panel
	{
		delay1 = DELAY_1;

		/* Let's poll the switch 2 to hear the sounds */
		/*
		if (ADC1ConvertedValues[1] > 40)
		{
			pass = 0.5f;
		}
		else
		{
			pass = 0.0f;
		}
		*/

		switchVal = ADC1ConvertedValues[0]; // read switch 1
		if (switchVal < 31 ) val = 0; else val = 1;
		if (val != potSet)
		{
			accroche1 = OFF; STM_EVAL_LEDOff(LED5);
			accroche2 = OFF; STM_EVAL_LEDOff(LED4);
			accroche3 = OFF; STM_EVAL_LEDOff(LED3);
			accroche4 = OFF; STM_EVAL_LEDOff(LED6);
		}
		potSet = val;

		if (potSet == 0) // first functions for pots
		{
			/*-------------------------------------------------------*/
			if (fabsf(ADC1ConvertedValues[4] - param1) <= DELTA)
			{
				accroche1 = ON;
				STM_EVAL_LEDOn(LED5);
			}
			else
			{
				if (accroche1 == ON) param1 = ADC1ConvertedValues[4];
			}
			/*-------------------------------------------------------*/
			if (fabsf(ADC1ConvertedValues[5] - param2) <= DELTA)
			{
				accroche2 = ON;
				STM_EVAL_LEDOn(LED4);
			}
			else
			{
				if (accroche2 == ON) param2 = ADC1ConvertedValues[5];
			}
			/*-------------------------------------------------------*/
			if (fabsf(ADC1ConvertedValues[6] - param3) <= DELTA)
			{
				accroche3 = ON;
				STM_EVAL_LEDOn(LED3);
			}
			else
			{
				if (accroche3 == ON) param3 = ADC1ConvertedValues[6];
			}
			/*-------------------------------------------------------*/
			if (fabsf(ADC1ConvertedValues[7] - param4) <= DELTA)
			{
				accroche4 = ON;
				STM_EVAL_LEDOn(LED6);
			}
			else
			{
				if (accroche4 == ON) param4 = ADC1ConvertedValues[7];
			}
			/*-------------------------------------------------------*/

			/* update tempo */
			reloadValue = (uint16_t)(MAX_ARR - (float)(param1) * (MAX_ARR - MIN_ARR)/ ADCMAX) ; //
			TIM_SetAutoreload(TIM6, reloadValue);

			/* update delay time */
			shift = MIN_DELAY + (uint32_t)(param3 * 23500.f / ADCMAX); //
			pos = writepos - shift;
			if (pos >= delayline)
				readpos = pos;
			else
				readpos = pos + DELAYLINE_LEN - 1;

			/* update feedback delay */
			fdb = (float)(param4 / ADCMAX); //


		}
		else // second functions for pots
		{
			/*-------------------------------------------------------*/
			if (fabsf(ADC1ConvertedValues[4] - param1b) <= DELTA)
			{
				accroche1 = ON;
				STM_EVAL_LEDOn(LED5);
			}
			else
			{
				if (accroche1 == ON) param1b = ADC1ConvertedValues[4];
			}
			/*-------------------------------------------------------*/
			if (fabsf(ADC1ConvertedValues[5] - param2b) <= DELTA)
			{
				accroche2 = ON;
				STM_EVAL_LEDOn(LED4);
			}
			else
			{
				if (accroche2 == ON) param2b = ADC1ConvertedValues[5];
			}
			/*-------------------------------------------------------*/
			if (fabsf(ADC1ConvertedValues[6] - param3b) <= DELTA)
			{
				accroche3 = ON;
				STM_EVAL_LEDOn(LED3);
			}
			else
			{
				if (accroche3 == ON) param3b = ADC1ConvertedValues[6];
			}
			/*-------------------------------------------------------*/
			if (fabsf(ADC1ConvertedValues[7] - param4b) <= DELTA)
			{
				accroche4 = ON;
				STM_EVAL_LEDOn(LED6);
			}
			else
			{
				if (accroche4 == ON) param4b = ADC1ConvertedValues[7];
			}
			/*-------------------------------------------------------*/

			envTime = (float)(param1b / ADCMAX);
			PhaserRate(10 * param2b/ADCMAX);
			PhaserFeedback(param3b/ADCMAX);
			EVAL_AUDIO_VolumeCtl((uint8_t)(MAXVOL * param4b / ADCMAX));
		}
	}
}


/******************************************************************************/
/*                 STM32F4xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f4xx.s).                                               */
/******************************************************************************/

/**
 * @brief  This function handles TIM6 global interrupt request.
 * The interrupt provides the sequencer clock
 * @param  None
 * @retval None
 */
void TIM6_DAC_IRQHandler(void)
{
	uint16_t	knobValue;

	if (TIM_GetITStatus(TIM6, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM6, TIM_IT_Update);

		/* LED4 toggling with frequency =  */
		//STM_EVAL_LEDToggle(LED3);

		if (seq1[seq1Number][seqIndex] != 0) // if there is a note in the next sequence step...
		{
			knobValue = param2; // read knob 2
			/*  update note */

			/*if (ADC1ConvertedValues[3] >= 50) // if automatic generation selected...
			{*/

				if (knobValue < 10)
					f1 = (float)(scale4[(uint16_t)(randomNum() * 11.f)]);
				else if (knobValue < 20)
					f1 = (float)(scale3[(uint16_t)(randomNum() * 17.f)]);
				else if (knobValue < 30)
					f1 = (float)(scale1[(uint16_t)(randomNum() * 38.f)]);
				else if (knobValue < 40)
					f1 = (float)(scale5[(uint16_t)(randomNum() * 41.f)]);
				else if (knobValue < 50)
					f1 = (float)(scale6[(uint16_t)(randomNum() * 41.f)]);
				else
					f1 = (float)(scale2[(uint16_t)(randomNum() * 46.f)]);
			/*}
			else	// manual note generation
				f1 = (float) (MIDINOTES_FREQ_LUT[knobValue + 45]);*/

			envAmp = seq1[seq1Number][seqIndex];
			envTrigger = ON;

		} else
		{
			envTrigger = OFF;
		}
		if (seqIndex >= 15) // wrap sequence index
		{
			seqIndex = 0;
			seq1Number = (uint32_t)(randomNum() * 10.f); // choose a new random sequence
		}
		else seqIndex++;


	}
}


/**
 * @brief  This function handles External line 0 interrupt request.
 * @param  None
 * @retval None
 */
void EXTI0_IRQHandler(void)
{

}

/**
 * @brief  This function handles TIM4 global interrupt request.
 * @param  None
 * @retval None
 */
void TIM4_IRQHandler(void)
{

}


/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
