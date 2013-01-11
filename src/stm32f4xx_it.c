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

#include "stm32f4xx_it.h"

/* ---------------------------------------------------------------------------*/



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

//===============================================================================================
static void update_tempo(void)
{
	/* update tempo */
	param1 = ADC1ConvertedValues[4];
	reloadValue = (uint16_t)(MAX_ARR - (float_t)(param1) * (MAX_ARR - MIN_ARR)/ ADCMAX) ; //
	TIM_SetAutoreload(TIM6, reloadValue);
}

static void update_scale(void)
{
	param2 = ADC1ConvertedValues[5];
}

static void update_delaytime(void)
{
	/* update delay time */
	uint32_t	shift;
	float_t 	*pos;

	param3 = ADC1ConvertedValues[6];
	if (fabsf(param3 - old_param3) > DELTA) old_param3 = param3; // ultra basic noise blocker
	shift = MIN_DELAY + (uint32_t)(old_param3 * (DELAYLINE_LEN - MIN_DELAY) / ADCMAX); //
	pos = writepos - shift;
	if (pos >= delayline)
		readpos = pos;
	else
		readpos = pos + DELAYLINE_LEN - 1;
}

static void update_delayfeedback(void)
{
	/* update feedback delay */
	param4 = ADC1ConvertedValues[7];
	fdb = (float_t)(param4 / ADCMAX); //
}

static void update_envelope(void)
{
	param1b = ADC1ConvertedValues[4];
	envTime = (float_t)(param1b / ADCMAX);
}

static void update_phaserRate(void)
{
	param2b = ADC1ConvertedValues[5];
	PhaserRate(8 * param2b/ADCMAX);
}

static void update_phaserFdb(void)
{
	param3b = ADC1ConvertedValues[6];
	PhaserFeedback(param3b/ADCMAX);
}

static void update_volume(void)
{
	param4b = ADC1ConvertedValues[7];
	EVAL_AUDIO_VolumeCtl((uint8_t)(MAXVOL * param4b / ADCMAX));
}
//=============================================================================================
/**
 * @brief  This function handles SysTick Handler.
 * Every DELAY_1 ticks this function reads the panel potentiometers and switches
 * and updates the parameters.
 * @param  None
 * @retval None
 */
void SysTick_Handler(void)
{

	uint16_t 	switchVal;
	uint8_t		val;

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
		if (switchVal < ADCMAX / 2 ) val = 0; else val = 1;
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
			if (accroche1 == ON)
			{
				STM_EVAL_LEDOn(LED5);
				//param1 = ADC1ConvertedValues[4];
				update_tempo();
			}
			else
			{
				if (fabsf(ADC1ConvertedValues[4] - param1) <= DELTA)
				{
					accroche1 = ON;
					STM_EVAL_LEDOn(LED5);
					//param1 = ADC1ConvertedValues[4];
					update_tempo();
				}
			}
			/*-------------------------------------------------------*/
			if (accroche2 == ON)
			{
				STM_EVAL_LEDOn(LED4);
				//param2 = ADC1ConvertedValues[5];
				update_scale();
			}
			else
			{
				if (fabsf(ADC1ConvertedValues[5] - param2) <= DELTA)
				{
					accroche2 = ON;
					STM_EVAL_LEDOn(LED4);
					//param2 = ADC1ConvertedValues[5];
					update_scale();
				}
			}
			/*-------------------------------------------------------*/
			if (accroche3 == ON)
			{
				STM_EVAL_LEDOn(LED3);
				//param3 = ADC1ConvertedValues[6];
				update_delaytime();
			}
			else
			{
				if (fabsf(ADC1ConvertedValues[6] - param3) <= DELTA)
				{
					accroche3 = ON;
					STM_EVAL_LEDOn(LED3);
					//param3 = ADC1ConvertedValues[6];
					update_delaytime();
				}
			}
			/*-------------------------------------------------------*/
			if (accroche4 == ON)
			{
				STM_EVAL_LEDOn(LED6);
				//param4 = ADC1ConvertedValues[7];
				update_delayfeedback();
			}
			else
			{
				if (fabsf(ADC1ConvertedValues[7] - param4) <= DELTA)
				{
					accroche4 = ON;
					STM_EVAL_LEDOn(LED6);
					//param4 = ADC1ConvertedValues[7];
					update_delayfeedback();
				}
			}
		}
		else // second functions for pots
		{
			/*-------------------------------------------------------*/
			if (accroche1 == ON)
			{
				STM_EVAL_LEDOn(LED5);
				//param1b = ADC1ConvertedValues[4];
				update_envelope();
			}
			else
			{
				if (fabsf(ADC1ConvertedValues[4] - param1b) <= DELTA)
				{
					accroche1 = ON;
					STM_EVAL_LEDOn(LED5);
					//param1b = ADC1ConvertedValues[4];
					update_envelope();
				}
			}
			/*-------------------------------------------------------*/
			if (accroche2 == ON)
			{
				STM_EVAL_LEDOn(LED4);
				//param2b = ADC1ConvertedValues[5];
				update_phaserRate();
			}
			else
			{
				if (fabsf(ADC1ConvertedValues[5] - param2b) <= DELTA)
				{
					accroche2 = ON;
					STM_EVAL_LEDOn(LED4);
					//param2 = ADC1ConvertedValues[5];
					update_phaserRate();
				}
			}
			/*-------------------------------------------------------*/
			if (accroche3 == ON)
			{
				STM_EVAL_LEDOn(LED3);
				//param3b = ADC1ConvertedValues[6];
				update_phaserFdb();
			}
			else
			{
				if (fabsf(ADC1ConvertedValues[6] - param3b) <= DELTA)
				{
					accroche3 = ON;
					STM_EVAL_LEDOn(LED3);
					//param3b = ADC1ConvertedValues[6];
					update_phaserFdb();
				}
			}
			/*-------------------------------------------------------*/
			if (accroche4 == ON)
			{
				STM_EVAL_LEDOn(LED6);
				//param4b = ADC1ConvertedValues[7];
				update_volume();
			}
			else
			{
				if (fabsf(ADC1ConvertedValues[7] - param4b) <= DELTA)
				{
					accroche4 = ON;
					STM_EVAL_LEDOn(LED6);
					//param4b = ADC1ConvertedValues[7];
					update_volume();
				}
			}

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

			if 	(knobValue < LIM1)
				f1 = (float_t)(scale7[(uint16_t)(randomNum() * 10)]);
			else if (knobValue < LIM2)
				f1 = (float_t)(scale8[(uint16_t)(randomNum() * 11)]);
			else if (knobValue < LIM3)
				f1 = (float_t)(scale4[(uint16_t)(randomNum() * 11)]);
			else if (knobValue < LIM4)
				f1 = (float_t)(scale3[(uint16_t)(randomNum() * 17)]);
			else if (knobValue < LIM5)
				f1 = (float_t)(scale1[(uint16_t)(randomNum() * 38)]);
			else if (knobValue < LIM6)
				f1 = (float_t)(scale5[(uint16_t)(randomNum() * 41)]);
			else if (knobValue < LIM7)
				f1 = (float_t)(scale6[(uint16_t)(randomNum() * 41)]);
			else
				f1 = (float_t)(scale2[(uint16_t)(randomNum() * 46)]);
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
