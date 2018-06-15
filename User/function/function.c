/* Includes ------------------------------------------------------------------*/
#include "./function/function.h"

/* Private typedef -----------------------------------------------------------*/
/* UART handler declared in "bsp_debug_usart.c" file */
extern UART_HandleTypeDef UartHandle;
extern uint32_t step;
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint8_t	Chip_Addr	= 0;					// cc1101��ַ
uint8_t	RSSI = 0;								// RSSIֵ
uint8_t addr_eeprom;
uint16_t sync_eeprom;
uint32_t rfid_eeprom;
uint8_t SendBuffer[SEND_LENGTH] = {0};// �������ݰ�
uint8_t RecvBuffer[RECV_LENGTH] = {0};// �������ݰ�
uint8_t aRxBuffer[RXBUFFERSIZE];			// Buffer used for reception
/* Private function prototypes -----------------------------------------------*/
extern void Delay(uint32_t nCount);
/* Private functions ---------------------------------------------------------*/

/*===========================================================================
* ���� :MCU_Initial() => ��ʼ��CPU����Ӳ��																	*
* ˵�� ��������Ӳ���ĳ�ʼ���������Ѿ�������C��,��bsp.c�ļ�									*
============================================================================*/
void MCU_Initial(void)
{
    GPIO_Config();
		if(ADC_IN1_READ() == 1)
		{
//		#ifdef DEBUG
			Debug_USART_Config();
//		#endif
		}
		Delay(0x100);
		SPI_Config();
		Delay(0x100);
		ADXL362_Init();
		Delay(0x100);
		INT_GPIO_Config();
}

/*===========================================================================
* ���� :RF_Initial() => ��ʼ��RFоƬ																					*
* ���� :mode, =0,����ģʽ, else,����ģʽ																			*
* ˵�� :CC1101�Ĳ���,�Ѿ�������C��,��CC1101.c�ļ�,�ṩSPI��CSN����						*
				���ɵ������ڲ����к����û������ٹ���CC1101�ļĴ�����������						*
============================================================================*/
void RF_Initial(uint8_t addr, uint16_t sync, uint8_t mode)
{
	CC1101Init(addr, sync);                       			// ��ʼ��CC1101�Ĵ���
	if(mode == RX)				{CC1101SetTRMode(RX_MODE);}		// ����ģʽ
	else if(mode == TX)		{CC1101SetTRMode(TX_MODE);}   // ����ģʽ
	else
	{
		CC1101SetIdle();																	// ����ģʽ����ת��sleep״̬
		CC1101WORInit();																	// ��ʼ����Ų������
		CC1101SetWORMode();
	}
}

/*===========================================================================
* ����: System_Initial() => ��ʼ��ϵͳ��������                             	*
============================================================================*/
void System_Initial(void)
{
		/*##-1- initial all peripheral ###########################*/
    MCU_Initial(); 
		/*##-2- initial CC1101 peripheral,configure it's address and sync code ###########################*/
		addr_eeprom = (uint8_t)(0xff & DATAEEPROM_Read(EEPROM_START_ADDR)>>16); 
		sync_eeprom = (uint16_t)(0xffff & DATAEEPROM_Read(EEPROM_START_ADDR)); 
		rfid_eeprom	= DATAEEPROM_Read(EEPROM_START_ADDR+4);
		if(ADC_IN1_READ() == 1)
		{
//		#ifdef DEBUG
			printf("addr_eeprom = %x\n",addr_eeprom);
			printf("sync_eeprom = %x\n",sync_eeprom);
			printf("rfid_eeprom = %x\n",rfid_eeprom);
//		#endif
		}
    RF_Initial(addr_eeprom, sync_eeprom, IDLE);     // ��ʼ������оƬ������ģʽ
		#ifdef UART_PROG
		/*##-3- Put UART peripheral in reception process ###########################*/
		if(HAL_UART_Receive_IT(&UartHandle, (uint8_t *)aRxBuffer, RXBUFFERSIZE) != HAL_OK)
		{
			Error_Handler();
		}
		#endif
}

/*===========================================================================
* ���� :RF_RecvHandler() => �������ݽ��մ���                               	* 
============================================================================*/
uint8_t RF_RecvHandler(void)
{
	uint8_t i=0, length=0;
	int16_t rssi_dBm;
	uint32_t timeout;
	
	if(CC1101_IRQ_READ() == 0)         // �������ģ���Ƿ���������ж� 
		{
			HAL_NVIC_DisableIRQ(EXTI4_15_IRQn);
			__disable_irq();
			if(ADC_IN1_READ() == 1)
			{
//				#ifdef DEBUG
				printf("interrupt occur\n");
//				#endif
			}
			timeout = Delay_TimeOut;
			while (CC1101_IRQ_READ() == 0 && timeout != 0)
			{
				timeout--;
			}
			for (i=0; i<RECV_LENGTH; i++)   { RecvBuffer[i] = 0; } // clear array
			length = CC1101RecPacket(RecvBuffer, &Chip_Addr, &RSSI);	// ��ȡ���յ������ݳ��Ⱥ���������
			
			if(ADC_IN1_READ() == 1)
			{
//				#ifdef DEBUG
				rssi_dBm = CC1101CalcRSSI_dBm(RSSI);
				printf("RSSI = %ddBm, length = %d, address = %d\n",rssi_dBm,length,Chip_Addr);
				for(i=0; i<RECV_LENGTH; i++)
				{	
					printf("%x ",RecvBuffer[i]);
				}
//				#endif
			}
			if(length == 0)
				{
					if(ADC_IN1_READ() == 1)
					{
//						#ifdef DEBUG
						printf("receive error or Address Filtering fail\n");
//						#endif
					}
					return 1;
				}
			else
				{
					if(RecvBuffer[0] == 0xAB && RecvBuffer[1] == 0xCD)
					{
						if(RecvBuffer[4] == 0xC0 && RecvBuffer[5] == 0xC0)
						{return 4;}
						else if(RecvBuffer[4] == 0xC3 && RecvBuffer[5] == 0xC3)
						{return 5;}
						else if(RecvBuffer[4] == 0xC4 && RecvBuffer[5] == 0xC4)
						{return 6;}
						else if(RecvBuffer[4] == 0xC5 && RecvBuffer[5] == 0xC5)
						{return 7;}
						else if(RecvBuffer[4] == 0xC6 && RecvBuffer[5] == 0xC6)
						{return 8;}
						else
						{	
							if(ADC_IN1_READ() == 1)
							{
//							#ifdef DEBUG
								printf("receive function order error\r\n");
//							#endif
							}
							return 3;}
					}
					else
					{
						if(ADC_IN1_READ() == 1)
						{
//						#ifdef DEBUG
							printf("receive package beginning error\r\n");
//						#endif
						}
						return 2;}
				}
		}
	else	{return 0;}
}

/*===========================================================================
* ���� : RF_SendPacket() => ���߷������ݺ���                            			*
* ���� :Sendbufferָ������͵����ݣ�length���������ֽ���                     		*
* ��� :0,����ʧ��;1,���ͳɹ�                                       					*
============================================================================*/
void RF_SendPacket(uint8_t index)
{
	uint8_t i=0;
	uint32_t data;
	uint32_t dataeeprom;// ��eeprom�ж�������
	
	LED_GREEN_OFF();
	
	if(index == 4)
	{
		SendBuffer[0] = 0xAB;
		SendBuffer[1] = 0xCD;
		SendBuffer[2] = RecvBuffer[2];
		SendBuffer[3] = RecvBuffer[3];
		SendBuffer[4] = 0xD0;
		SendBuffer[5] = 0x01;
		SendBuffer[6] = RecvBuffer[6];
		SendBuffer[7] = RecvBuffer[7];
		SendBuffer[8] = RecvBuffer[8];
		SendBuffer[9] = RecvBuffer[9];
		SendBuffer[10] = (uint8_t)(0x000000FF & step>>24);
		SendBuffer[11] = (uint8_t)(0x000000FF & step>>16);
		SendBuffer[12] = (uint8_t)(0x000000FF & step>>8);
		SendBuffer[13] = (uint8_t)(0x000000FF & step);
//		for(i=10; i<SEND_LENGTH; i++)
//		{
//			SendBuffer[i] = i-10;
//		}
//		printf("\r\n");
//		for(i=0; i<SEND_LENGTH; i++)
//		{
//			printf("%x ",SendBuffer[i]);
//		}
		SendBuffer[18] = RSSI;
		for(i=0; i<SEND_PACKAGE_NUM; i++)
		{
			Delay(0x10);
			CC1101SendPacket(SendBuffer, SEND_LENGTH, ADDRESS_CHECK);
		}
	}
	else if(index == 5)
	{
		SendBuffer[0] = 0xAB;
		SendBuffer[1] = 0xCD;
		SendBuffer[2] = RecvBuffer[2];
		SendBuffer[3] = RecvBuffer[3];		
		SendBuffer[4] = 0xD3;
		SendBuffer[5] = 0x01;
		SendBuffer[6] = RecvBuffer[6];
		SendBuffer[7] = RecvBuffer[7];
		SendBuffer[8] = RecvBuffer[8];
		SendBuffer[9] = RecvBuffer[9];
		SendBuffer[18] = RSSI;
		for(i=0; i<SEND_PACKAGE_NUM; i++)
		{
			Delay(0x10);
			CC1101SendPacket(SendBuffer, SEND_LENGTH, ADDRESS_CHECK);
		}
	}
	else if(index == 6)
	{
		SendBuffer[0] = 0xAB;
		SendBuffer[1] = 0xCD;
		SendBuffer[2] = RecvBuffer[2];
		SendBuffer[3] = RecvBuffer[3];		
		SendBuffer[4] = 0xD4;
		SendBuffer[5] = 0x01;
		SendBuffer[6] = RecvBuffer[6];
		SendBuffer[7] = RecvBuffer[7];
		SendBuffer[8] = RecvBuffer[8];
		SendBuffer[9] = RecvBuffer[9];
		SendBuffer[10] = (uint8_t)(0x000000FF & step>>24);
		SendBuffer[11] = (uint8_t)(0x000000FF & step>>16);
		SendBuffer[12] = (uint8_t)(0x000000FF & step>>8);
		SendBuffer[13] = (uint8_t)(0x000000FF & step);
		SendBuffer[18] = RSSI;
		for(i=0; i<SEND_PACKAGE_NUM; i++)
		{
			Delay(0x10);
			CC1101SendPacket(SendBuffer, SEND_LENGTH, ADDRESS_CHECK);
		}
	}
	else if(index == 7)
	{
		data = ((uint32_t)(0xFF000000 & RecvBuffer[10]<<24)+(uint32_t)(0x00FF0000 & RecvBuffer[11]<<16)+(uint32_t)(0x0000FF00 & RecvBuffer[12]<<8)+(uint32_t)(0x000000FF & RecvBuffer[13]));
		DATAEEPROM_Program(EEPROM_START_ADDR, data);
		data = ((uint32_t)(0xFF000000 & RecvBuffer[14]<<24)+(uint32_t)(0x00FF0000 & RecvBuffer[15]<<16)+(uint32_t)(0x0000FF00 & RecvBuffer[16]<<8)+(uint32_t)(0x000000FF & RecvBuffer[17]));
		DATAEEPROM_Program(EEPROM_START_ADDR+4, data);
		SendBuffer[0] = 0xAB;
		SendBuffer[1] = 0xCD;
		SendBuffer[2] = RecvBuffer[2];
		SendBuffer[3] = RecvBuffer[3];		
		SendBuffer[4] = 0xD5;
		SendBuffer[5] = 0x01;
		SendBuffer[6] = RecvBuffer[6];
		SendBuffer[7] = RecvBuffer[7];
		SendBuffer[8] = RecvBuffer[8];
		SendBuffer[9] = RecvBuffer[9];
		dataeeprom = DATAEEPROM_Read(EEPROM_START_ADDR);
		SendBuffer[10] = (uint8_t)(0x000000FF & dataeeprom>>24);
		SendBuffer[11] = (uint8_t)(0x000000FF & dataeeprom>>16);
		SendBuffer[12] = (uint8_t)(0x000000FF & dataeeprom>>8);
		SendBuffer[13] = (uint8_t)(0x000000FF & dataeeprom);
		dataeeprom = DATAEEPROM_Read(EEPROM_START_ADDR+4);
		SendBuffer[14] = (uint8_t)(0x000000FF & dataeeprom>>24);
		SendBuffer[15] = (uint8_t)(0x000000FF & dataeeprom>>16);
		SendBuffer[16] = (uint8_t)(0x000000FF & dataeeprom>>8);
		SendBuffer[17] = (uint8_t)(0x000000FF & dataeeprom);	
		SendBuffer[18] = RSSI;
		for(i=0; i<SEND_PACKAGE_NUM; i++)
		{
			Delay(0x10);
			CC1101SendPacket(SendBuffer, SEND_LENGTH, ADDRESS_CHECK);
		}
	}
	else if(index == 8)
	{
		step = 0;
		SendBuffer[0] = 0xAB;
		SendBuffer[1] = 0xCD;
		SendBuffer[2] = RecvBuffer[2];
		SendBuffer[3] = RecvBuffer[3];		
		SendBuffer[4] = 0xD6;
		SendBuffer[5] = 0x01;
		SendBuffer[6] = RecvBuffer[6];
		SendBuffer[7] = RecvBuffer[7];
		SendBuffer[8] = RecvBuffer[8];
		SendBuffer[9] = RecvBuffer[9];
		SendBuffer[18] = RSSI;
		for(i=0; i<SEND_PACKAGE_NUM; i++)
		{
			Delay(0x10);
			CC1101SendPacket(SendBuffer, SEND_LENGTH, ADDRESS_CHECK);
		}
	}
//	else if(index == 1)
//	{
//		SendBuffer[0] = 0xAB;
//		SendBuffer[1] = 0xCD;
//		SendBuffer[2] = 0x01;
//		SendBuffer[3] = 0x01;
//		SendBuffer[4] = RecvBuffer[6];
//		SendBuffer[5] = RecvBuffer[7];
//		SendBuffer[6] = RecvBuffer[8];
//		SendBuffer[7] = RecvBuffer[9];
//		SendBuffer[14] = 0x01;
//		for(i=0; i<SEND_PACKAGE_NUM; i++)
//		{
//			CC1101SendPacket(SendBuffer, SEND_LENGTH, ADDRESS_CHECK);    // ��������
//			Delay(0xFFFF);									// ����õ�ƽ��27ms����һ������
////		Delay(0xFFFFF);									// ����õ�ƽ��130ms����һ������
//		}
//	}
//	else if(index == 2)
//	{
//		SendBuffer[0] = 0xAB;
//		SendBuffer[1] = 0xCD;
//		SendBuffer[2] = 0x02;
//		SendBuffer[3] = 0x02;
//		SendBuffer[4] = RecvBuffer[6];
//		SendBuffer[5] = RecvBuffer[7];
//		SendBuffer[6] = RecvBuffer[8];
//		SendBuffer[7] = RecvBuffer[9];
//		SendBuffer[14] = 0x02;
//		for(i=0; i<SEND_PACKAGE_NUM; i++)
//		{
//			CC1101SendPacket(SendBuffer, SEND_LENGTH, ADDRESS_CHECK);    // ��������
//			Delay(0xFFFF);									// ����õ�ƽ��27ms����һ������
////		Delay(0xFFFFF);									// ����õ�ƽ��130ms����һ������
//		}
//	}
//	else if(index == 3)
//	{
//		SendBuffer[0] = 0xAB;
//		SendBuffer[1] = 0xCD;
//		SendBuffer[2] = 0x03;
//		SendBuffer[3] = 0x03;
//		SendBuffer[4] = RecvBuffer[6];
//		SendBuffer[5] = RecvBuffer[7];
//		SendBuffer[6] = RecvBuffer[8];
//		SendBuffer[7] = RecvBuffer[9];
//		SendBuffer[14] = 0x03;
//		for(i=0; i<SEND_PACKAGE_NUM; i++)
//		{
//			CC1101SendPacket(SendBuffer, SEND_LENGTH, ADDRESS_CHECK);    // ��������
//			Delay(0xFFFF);									// ����õ�ƽ��27ms����һ������
////		Delay(0xFFFFF);									// ����õ�ƽ��130ms����һ������
//		}
//	}

	for(i=0; i<SEND_LENGTH; i++) // clear array
	{SendBuffer[i] = 0;}

//	Usart_SendString(&UartHandle, (uint8_t *)"Transmit OK\r\n");
	RF_Initial(addr_eeprom, sync_eeprom, IDLE);
	__enable_irq();
	HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);
	LED_GREEN_ON();
}

/*===========================================================================
* ���� :DATAEEPROM_Program() => ��eeprom�������                         			* 
============================================================================*/
void DATAEEPROM_Program(uint32_t Address, uint32_t Data)
{
	/* Unlocks the data memory and FLASH_PECR register access *************/ 
	if(HAL_FLASHEx_DATAEEPROM_Unlock() != HAL_OK)
	{
    Error_Handler();
	}
	/* Clear FLASH error pending bits */
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_WRPERR | FLASH_FLAG_SIZERR |
																											FLASH_FLAG_OPTVERR | FLASH_FLAG_RDERR | 
																												FLASH_FLAG_FWWERR | FLASH_FLAG_NOTZEROERR);
	/*Erase a word in data memory *************/
	if (HAL_FLASHEx_DATAEEPROM_Erase(Address) != HAL_OK)
	{
		Error_Handler();
	}
	/*Enable DATA EEPROM fixed Time programming (2*Tprog) *************/
	HAL_FLASHEx_DATAEEPROM_EnableFixedTimeProgram();
  /* Program word at a specified address *************/
	if (HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_WORD, Address, Data) != HAL_OK)
	{
		Error_Handler();
	}
	/*Disables DATA EEPROM fixed Time programming (2*Tprog) *************/
	HAL_FLASHEx_DATAEEPROM_DisableFixedTimeProgram();

  /* Locks the Data memory and FLASH_PECR register access. (recommended
     to protect the DATA_EEPROM against possible unwanted operation) *********/
  HAL_FLASHEx_DATAEEPROM_Lock(); 

}

/*===========================================================================
* ���� :DATAEEPROM_Read() => ��eeprom��ȡ����                         				* 
============================================================================*/
uint32_t DATAEEPROM_Read(uint32_t Address)
{
	return *(__IO uint32_t*)Address;
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
