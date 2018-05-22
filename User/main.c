/**
  ******************************************************************************
  * @file    Templates/Src/main.c 
  * @author  MCD Application Team
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2016 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
extern uint8_t aRxBuffer[RXBUFFERSIZE];								// Buffer used for reception
/** @addtogroup STM32L0xx_HAL_Examples
  * @{
  */

/** @addtogroup Templates
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
unsigned char Temp;
uint32_t step = 0;
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/*===========================================================================
* ���� : main() => ���������������                                       	*
* ˵�� : ����һ�����ݣ�������Ӧ������																				*
============================================================================*/
int main(void)
{
	Delay(900000);
	/* STM32L0xx HAL library initialization:
       - Configure the Flash prefetch, Flash preread and Buffer caches
       - Systick timer is configured by default as source of time base, but user 
             can eventually implement his proper time base source (a general purpose 
             timer for example or other time source), keeping in mind that Time base 
             duration should be kept 1ms since PPP_TIMEOUT_VALUEs are defined and 
             handled in milliseconds basis.
       - Low Level Initialization
	*/
	HAL_Init();

	/* Disable Prefetch Buffer */
  __HAL_FLASH_PREFETCH_BUFFER_DISABLE();

  /* Configure the system clock @ 32 KHz */
  SystemClock_Config();

  /* Configure the system Power */
//  SystemPower_Config();
	
	System_Initial();

	Show_Message();
	
	/* Enter LP RUN mode */
	HAL_PWREx_EnableLowPowerRunMode();

	/* Wait until the system enters LP RUN and the Regulator is in LP mode */
	while(__HAL_PWR_GET_FLAG(PWR_FLAG_REGLP) == RESET){}

//	/* Exit LP RUN mode */
//	HAL_PWREx_DisableLowPowerRunMode();

//	/* Wait until the system exits LP RUN and the Regulator is in main mode */
//	while(__HAL_PWR_GET_FLAG(PWR_FLAG_REGLP) != RESET){}
		
//	__set_FAULTMASK(1);
//	NVIC_SystemReset();
		
	while(1)
		{
			index = RF_RecvHandler();
			if(index != 0)   // �������ݽ��մ���
			{
				RF_SendPacket(index);
			}
		}
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @param  htim: TIM handle
  * @retval None
  */
//void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
//{
////	if(RecvWaitTime != 0 && RecvWaitTime != 1)	// ���ݽ��ռ�ʱ
////		{	RecvWaitTime--;}
////	else if(RecvWaitTime == 1)
////		{	RecvFlag=1;}
////	else
////		{	RecvFlag=0;}
//	if(ADXL362_INT1_READ() == 1)
//		{
//			i++;
//			printf("i is %d\n",i);
//		}
//	Temp = ADXL362RegisterRead(XL362_STATUS);
//	printf("Status is %x\n",Temp);

//	LED_GREEN_TOG();
//}

#ifdef UART_PROG
/**
  * @brief  Tx Transfer completed callback
  * @param  UartHandle: UART handle. 
  * @note   This example shows a simple way to report end of IT Tx transfer, and 
  *         you can add your own implementation. 
  * @retval None
  */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *UartHandle)
{
  /* Set transmission flag: trasfer complete*/
//  printf("trasfer complete\n");
}

/**
  * @brief  Rx Transfer completed callback
  * @param  UartHandle: UART handle
  * @note   This example shows a simple way to report end of IT Rx transfer, and 
  *         you can add your own implementation.
  * @retval None
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle)
{
  /* Set transmission flag: trasfer complete*/
		uint32_t data;
		uint8_t uart_addr_eeprom;// ��eeprom�ж�������
		uint16_t uart_sync_eeprom;
		uint32_t uart_rfid_eeprom;
//		if(ADC_IN1_READ() == 1)
		#ifdef DEBUG
		printf("data = %x %x %x %x %x %x %x %x %x %x %x %x \n",aRxBuffer[0],aRxBuffer[1],aRxBuffer[2],aRxBuffer[3],aRxBuffer[4],aRxBuffer[5],aRxBuffer[6],aRxBuffer[7],aRxBuffer[8],aRxBuffer[9],aRxBuffer[10],aRxBuffer[11]);
		#endif
		/*##-1- Check UART receive data whether is ��ABCD�� begin or not ###########################*/
		if(aRxBuffer[0] == 0x41 && aRxBuffer[1] == 0x42 && aRxBuffer[2] == 0x43 && aRxBuffer[3] == 0x44)//���롮ABCD��
			{
				data = ((uint32_t)(0xFF000000 & aRxBuffer[4]<<24)+(uint32_t)(0x00FF0000 & aRxBuffer[5]<<16)+(uint32_t)(0x0000FF00 & aRxBuffer[6]<<8)+(uint32_t)(0x000000FF & aRxBuffer[7]));
				DATAEEPROM_Program(EEPROM_START_ADDR, data);
				data = ((uint32_t)(0xFF000000 & aRxBuffer[8]<<24)+(uint32_t)(0x00FF0000 & aRxBuffer[9]<<16)+(uint32_t)(0x0000FF00 & aRxBuffer[10]<<8)+(uint32_t)(0x000000FF & aRxBuffer[11]));
				DATAEEPROM_Program(EEPROM_START_ADDR+4, data);
				uart_addr_eeprom = (uint8_t)(0xff & DATAEEPROM_Read(EEPROM_START_ADDR)>>16);
				uart_sync_eeprom = (uint16_t)(0xffff & DATAEEPROM_Read(EEPROM_START_ADDR));
				uart_rfid_eeprom	= DATAEEPROM_Read(EEPROM_START_ADDR+4);
//				if(ADC_IN1_READ() == 1)
				#ifdef DEBUG
					printf("eeprom program end\n");
					printf("addr_eeprom = %x\n",uart_addr_eeprom);
					printf("sync_eeprom = %x\n",uart_sync_eeprom);
					printf("rfid_eeprom = %x\n",uart_rfid_eeprom);
				#endif
			}
}

/**
  * @brief  UART error callbacks
  * @param  UartHandle: UART handle
  * @note   This example shows a simple way to report transfer error, and you can
  *         add your own implementation.
  * @retval None
  */
 void HAL_UART_ErrorCallback(UART_HandleTypeDef *UartHandle)
{
		while(1)
    {
			LED_GREEN_ON();
    }
}
#endif

/**
  * @brief EXTI line detection callback.
  * @param GPIO_Pin: Specifies the pins connected EXTI line
  * @retval None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if(GPIO_Pin == ADXL362_INT1_PIN)
  {
//		LED_GREEN_TOG();
		step++;
//		if(ADC_IN1_READ() == 1)
		#ifdef DEBUG
			Temp = ADXL362RegisterRead(XL362_STATUS);
			printf("Status is %x\n",Temp);
			printf("Step is %d\n",step);
		#endif
  }
  
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSI)
  *            SYSCLK(Hz)                     = 32000000
  *            HCLK(Hz)                       = 32000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 1
  *            APB2 Prescaler                 = 1
  *            HSI Frequency(Hz)              = 16000000
  *            PLL_MUL                        = 4
  *            PLL_DIV                        = 2
  *            Flash Latency(WS)              = 1
  *            Main regulator output voltage  = Scale1 mode
  * @param  None
  * @retval None
  */
//static void SystemClock_Config(void)
//{
//  RCC_ClkInitTypeDef RCC_ClkInitStruct;
//  RCC_OscInitTypeDef RCC_OscInitStruct;
//  
//  /* Enable Power Control clock */
//  __HAL_RCC_PWR_CLK_ENABLE();
//  
//  /* The voltage scaling allows optimizing the power consumption when the device is 
//     clocked below the maximum system frequency, to update the voltage scaling value 
//     regarding system frequency refer to product datasheet.  */
//  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

//  /* Enable HSI Oscillator and activate PLL with HSI as source */
//  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
//  RCC_OscInitStruct.HSEState = RCC_HSE_OFF;
//  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
//  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
//  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
//  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL4;
//  RCC_OscInitStruct.PLL.PLLDIV = RCC_PLL_DIV2;
//  RCC_OscInitStruct.HSICalibrationValue = 0x10;
//  HAL_RCC_OscConfig(&RCC_OscInitStruct);
//  
//  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
//     clocks dividers */
//  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
//  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
//  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
//  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;  
//  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;  
//  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1);
//}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = (MSI)
  *            MSI Range                      = 2
  *            SYSCLK(Hz)                     = 32000
  *            HCLK(Hz)                       = 32000
  *            AHB Prescaler                  = 2
  *            APB1 Prescaler                 = 1
  *            APB2 Prescaler                 = 1
  *            Main regulator output voltage  = Scale2 mode
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;

  /* Enable Power Control clock */
  __HAL_RCC_PWR_CLK_ENABLE();

  /* The voltage scaling allows optimizing the power consumption when the device is 
     clocked below the maximum system frequency, to update the voltage scaling value 
     regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /* Enable MSI Oscillator */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_3;
  RCC_OscInitStruct.MSICalibrationValue = 0x00;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }

  /* Select MSI as system clock source and configure the HCLK, PCLK1 and PCLK2 
     clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }

  /* Set MSI range to 0 */
//  __HAL_RCC_MSI_RANGE_CONFIG(RCC_MSIRANGE_0);

}

/**
  * @brief  System Power Configuration
  *         The system Power is configured as follow : 
  *            + System Running at MSI (~32KHz)
  *            + Flash 0 wait state  
  *            + Voltage Range 2
  *            + Code running from Internal FLASH
  *            + Wakeup using Key Button PC.13
  * @param  None
  * @retval None
  */
//static void SystemPower_Config(void)
//{
//  GPIO_InitTypeDef GPIO_InitStructure;

//  /* Enable GPIOs clock */
//  __HAL_RCC_GPIOA_CLK_ENABLE();
//  __HAL_RCC_GPIOB_CLK_ENABLE();
//  __HAL_RCC_GPIOC_CLK_ENABLE();
//  __HAL_RCC_GPIOD_CLK_ENABLE();
//  __HAL_RCC_GPIOH_CLK_ENABLE();

//  /* Configure all GPIO port pins in Analog Input mode (floating input trigger OFF) */
//  GPIO_InitStructure.Pin = GPIO_PIN_All;
//  GPIO_InitStructure.Mode = GPIO_MODE_ANALOG;
//  GPIO_InitStructure.Pull = GPIO_NOPULL;
//  HAL_GPIO_Init(GPIOA, &GPIO_InitStructure); 
//  HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);
//  HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);
//  HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);
//  HAL_GPIO_Init(GPIOH, &GPIO_InitStructure);

//  /* Disable GPIOs clock */
//  __HAL_RCC_GPIOA_CLK_DISABLE();
//  __HAL_RCC_GPIOB_CLK_DISABLE();
//  __HAL_RCC_GPIOC_CLK_DISABLE();
//  __HAL_RCC_GPIOD_CLK_DISABLE();
//  __HAL_RCC_GPIOH_CLK_DISABLE();

//}

/**
  * @brief  Delay Function.
  * @param  nCount:specifies the Delay time length.
  * @retval None
  */
void Delay(__IO uint32_t nCount)
{
  while(nCount--)
  {
  }
}

/**
  * @brief  ��ӡָ��������ʾ��Ϣ
  * @param  none
  * @retval none
  */
static void Show_Message(void)
{
	unsigned int  ReadValueTemp;
	#ifdef DEBUG
		printf("\r\n CC1101 chip transfer program \n");
		printf(" using USART2,configuration:%d 8-N-1 \n",DEBUG_USART_BAUDRATE);
		printf(" when in transfer mode,the data must not exceed 60 bytes!!\r\n");  
	#endif
	ReadValueTemp = ADXL362RegisterRead(XL362_DEVID_AD);     	//Analog Devices device ID, 0xAD
	if(ReadValueTemp == 0xAD)
	{
		LED_GREEN_TOG();
		Delay(2000);
	}
	#ifdef DEBUG
		printf("Analog Devices device ID: %x\n",ReadValueTemp);	 	//send via UART
	#endif
	ReadValueTemp = ADXL362RegisterRead(XL362_DEVID_MST);    	//Analog Devices MEMS device ID, 0x1D
	if(ReadValueTemp == 0x1D)
	{
		LED_GREEN_TOG();
		Delay(2000);
	}
	#ifdef DEBUG
		printf("Analog Devices MEMS device ID: %x\n",ReadValueTemp);	//send via UART
	#endif	
	ReadValueTemp = ADXL362RegisterRead(XL362_PARTID);       	//part ID, 0xF2
	if(ReadValueTemp == 0xF2)
	{
		LED_GREEN_TOG();
		Delay(2000);
	}
	#ifdef DEBUG
		printf("Part ID: %x\n",ReadValueTemp);										//send via UART
	#endif	
	ReadValueTemp = ADXL362RegisterRead(XL362_REVID);       	//version ID, 0x02
	if(ReadValueTemp == 0x02)
	{
		LED_GREEN_TOG();
	}
	#ifdef DEBUG
		printf("Version ID: %x\n",ReadValueTemp);									//send via UART
	#endif	
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void Error_Handler(void)
{
  /* User may add here some code to deal with this error */
  LED_GREEN_ON();
	printf("error\n");
  while(1)
  {
  }
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/**
  * @}
  */ 

/**
  * @}
  */ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
