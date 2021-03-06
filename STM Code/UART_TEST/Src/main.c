
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2018 STMicroelectronics
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
#include "stm32f4xx_hal.h"

/* USER CODE BEGIN Includes */
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

char* buff = "Hello!\n\r";
char* test = "Fop";
int result = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART3_UART_Init(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

//GPS Variables
int Rx_indx;
int Transfer_cplt = 0;
uint8_t Rx_data;
char Rx_Buffer[80];
char GPS_Data[60];
char CoordinateType[6];	//Determine if the current data received is GPGGA, GPGSA, GPRMC, or GPVTG, we only need RMC and GGA
char latDegBuff[3]; // "XX\0"
char latMinBuff[8]; // "XX.XXXX\0"
char longDegBuff[4]; // "XX\0"
char longMinBuff[8]; // "XX.XXXX\0"
char altitudeBuff[6];
int printLCD = 0;

//WiFi Variables
int WiFi_Transfer_cplt = 0;
uint8_t Rx3_data;
int Rx3_indx;
char Rx3_Buffer[35];
char drinkOrder[15];
char targetLatDeg[3];
char targetLatMin[9];
char targetLongDeg[3];
char targetLongMin[9];

unsigned long prevTimeLED;
unsigned long prevTimeLCD;
unsigned long prevTimeTransmit;
unsigned long currTime;

struct Latitude_data
{
	int degrees;
	double decimalMin;
	char orientation;
};

struct Longitude_data
{
	int degrees;
	double decimalMin;
	char orientation;
};

struct RMC_data {
	char fixStatus;
	struct Latitude_data Latitude;
    struct Longitude_data Longitude;
    double altitude;
} RMC;

struct WiFi_data {
	struct Latitude_data Latitude;
	struct Longitude_data Longitude;
	int orderCancelled;
} WiFi;

//LCD Functions
void lcd_send_cmd (char);
void lcd_send_data (char);
void lcd_print (char*);
void lcd_init(void);
void lcd_clear(void);
void lcd_firstLine(void);
void lcd_secondLine(void);

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    uint8_t i;
    uint8_t j;
    uint8_t k;

    if (huart->Instance == USART3)
    {
    	if (Rx3_indx==0) {for (i=0;i<35;i++) Rx3_Buffer[i]=0;}   //clear Rx_Buffer before receiving new data

		if (Rx3_data != 13) //if received data different from ascii 13 (enter)
		{
			Rx3_Buffer[Rx3_indx++]=Rx3_data;    //add data to Rx_Buffer
		}
		else            //if received data = 13
		{
			Rx3_indx=0;

			WiFi_Transfer_cplt=1;//transfer complete, data is ready to read
		}

		HAL_UART_Receive_IT(&huart3, &Rx3_data, 1);   //activate UART receive interrupt every time
    }

    if (huart->Instance == USART2)  //current UART
    {
        if (Rx_indx==0) {for (i=0;i<80;i++) Rx_Buffer[i]=0;}   //clear Rx_Buffer before receiving new data

        if (Rx_data != 10) //if received data different from ascii 13 (enter)
        {
            Rx_Buffer[Rx_indx++]=Rx_data;    //add data to Rx_Buffer
        }
        else            //if received data = 13
        {
            Rx_indx=0;

			for (j=0;j<6;j++) {
				CoordinateType[j] = Rx_Buffer[j];
			}

			j++;

			for (k=0;k<60;k++) {
				GPS_Data[k] = Rx_Buffer[j];
				j++;
			}

            Transfer_cplt=1;//transfer complete, data is ready to read
        }

        HAL_UART_Receive_IT(&huart2, &Rx_data, 1);   //activate UART receive interrupt every time
    }
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  *
  * @retval None
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_I2C1_Init();
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */

  lcd_init();
  lcd_clear();
  lcd_print("TEST");

  result = strcmp(test, "Foo");

  HAL_UART_Receive_IT(&huart2, &Rx_data, 1);
  HAL_UART_Receive_IT(&huart3, &Rx3_data, 1);   //activate UART receive interrupt every time

  prevTimeLED = 0;
  prevTimeLCD = 0;
  prevTimeTransmit = 0;


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  currTime = HAL_GetTick();

	  if (currTime - prevTimeLED >= 1000) {
		  prevTimeLED = currTime;
		  HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_12);
	  }

  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */

	  if (Transfer_cplt) {
		  Transfer_cplt = 0;

		  if (strcmp(CoordinateType, "$GPRMC") == 0)
		  {
			  RMC.fixStatus = GPS_Data[11];

			  if (RMC.fixStatus == 'V') {
				  if (printLCD) {
					  lcd_clear();
					  lcd_print("NO GPS FIX");
					  printLCD = 0;
				  }
			  }


			  //Latitude Degrees
			  memcpy(latDegBuff, &GPS_Data[13], 2);
			  latDegBuff[2] = '\0';
			  RMC.Latitude.degrees = atoi(latDegBuff);

			  //Latitude Decimal Minutes
			  memcpy(latMinBuff, &GPS_Data[15], 7);
			  latMinBuff[7] = '\0';
			  RMC.Latitude.decimalMin = atof(latMinBuff);

			  //Latitude Orientation
			  RMC.Latitude.orientation = GPS_Data[23];

			  //Longitude Degrees
			  memcpy(longDegBuff, &GPS_Data[25], 3);
			  latDegBuff[3] = '\0';
			  RMC.Longitude.degrees = atoi(longDegBuff);

			  //Longitude Decimal Minutes
			  memcpy(latMinBuff, &GPS_Data[28], 7);
			  latMinBuff[7] = '\0';
			  RMC.Longitude.decimalMin = atof(latMinBuff);

			  //Longitude Orientation
			  RMC.Longitude.orientation = GPS_Data[36];
		  }

		  else if (strcmp(CoordinateType, "$GPGGA") == 0)
		  {
			  int a = 46;
			  int b = 0;

			  while (GPS_Data[a] != ',') {
				  altitudeBuff[b] = GPS_Data[a];
				  a++;
				  b++;
			  }
			  altitudeBuff[b] = '\0';

			  RMC.altitude = roundf((atof(altitudeBuff)) * 10) / 10;
		  }
	  }

	  else if (WiFi_Transfer_cplt)
	  {
		  WiFi_Transfer_cplt = 0;
		  //LATITUDE
		  if (Rx3_Buffer[0] == '+')
		  {
			  WiFi.Latitude.orientation = 'N';
			  WiFi.orderCancelled = 0;
		  }
		  else if (Rx3_Buffer[0] == '-')
		  {
			  WiFi.Latitude.orientation = 'S';
			  WiFi.orderCancelled = 0;
		  }
		  else
		  {
			  WiFi.orderCancelled = 1;
		  }

		  if (WiFi.orderCancelled == 0)
		  {
			  //Latitude Degrees
			  targetLatDeg[0] = Rx3_Buffer[1];
			  targetLatDeg[1] = Rx3_Buffer[2];
			  targetLatDeg[2] = '\0';

			  WiFi.Latitude.degrees = atoi(targetLatDeg);

			  //Latitude decimalMin
			  targetLatMin[0] = Rx3_Buffer[4];
			  targetLatMin[1] =Rx3_Buffer[5];
			  targetLatMin[2] = '.';
			  targetLatMin[3] = Rx3_Buffer[6];
			  targetLatMin[4] = Rx3_Buffer[7];
			  targetLatMin[5] = Rx3_Buffer[8];
			  targetLatMin[6] = Rx3_Buffer[9];
			  targetLatMin[7] = Rx3_Buffer[10];
			  targetLatMin[8] = '\0';

			  WiFi.Latitude.decimalMin = atof(targetLatMin);


			  //LONGITUDE
			  if (Rx3_Buffer[13] == '+')
			  {
				WiFi.Longitude.orientation = 'E';
			  }
			  else if (Rx3_Buffer[13] == '-')
			  {
				WiFi.Longitude.orientation = 'W';
			  }

			  //Longitude Degrees
			  targetLongDeg[0] = Rx3_Buffer[14];
			  targetLatDeg[1] = Rx3_Buffer[15];
			  targetLatDeg[2] = '\0';

			  WiFi.Longitude.degrees = atoi(targetLongDeg);

			  //Longitude decimalMin
			  targetLongMin[0] = Rx3_Buffer[17];
			  targetLongMin[1] = Rx3_Buffer[18];
			  targetLongMin[2] = '.';
			  targetLongMin[3] = Rx3_Buffer[19];
			  targetLongMin[4] = Rx3_Buffer[20];
			  targetLongMin[5] = Rx3_Buffer[21];
			  targetLongMin[6] = Rx3_Buffer[22];
			  targetLongMin[7] = Rx3_Buffer[23];
			  targetLongMin[8] = '\0';

			  WiFi.Longitude.decimalMin = atof(targetLongMin);


			  //DRINK ORDER
			  int a = 25;
			  int b = 0;

			  while (Rx3_Buffer[a] != ',')
			  {
				  drinkOrder[b] = Rx3_Buffer[a];
				  a++;
				  b++;
			  }
			  drinkOrder[b] = '\0';
		  }
	  }

  }
  /* USER CODE END 3 */

}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

    /**Configure the main internal regulator output voltage 
    */
  __HAL_RCC_PWR_CLK_ENABLE();

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = 16;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 50;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* I2C1 init function */
static void MX_I2C1_Init(void)
{

  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* USART1 init function */
static void MX_USART1_UART_Init(void)
{

  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* USART2 init function */
static void MX_USART2_UART_Init(void)
{

  huart2.Instance = USART2;
  huart2.Init.BaudRate = 9600;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* USART3 init function */
static void MX_USART3_UART_Init(void)
{

  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
static void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);

  /*Configure GPIO pin : PD12 */
  GPIO_InitStruct.Pin = GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

//-------------------USER FUNCTIONS--------------------//

void lcd_send_cmd (char cmd)
{
  char data_u, data_l;
	uint8_t data_t[4];
	data_u = cmd&0xf0;
	data_l = (cmd<<4)&0xf0;
	data_t[0] = data_u|0x04|0x08;  	//en=1, rs=0
	data_t[1] = data_u|0x08;  		//en=0, rs=0
	data_t[2] = data_l|0x04|0x08;  	//en=1, rs=0
	data_t[3] = data_l|0x08;  		//en=0, rs=0
	HAL_I2C_Master_Transmit (&hi2c1, 0x27<<1,(uint8_t *) data_t, 4, 100);
}

void lcd_send_data (char data)
{
	char data_u, data_l;
	uint8_t data_t[4];
	data_u = data&0xf0;
	data_l = (data<<4)&0xf0;
	data_t[0] = data_u|0x05|0x08;  //en=1, rs=0
	data_t[1] = data_u|0x01|0x08;  //en=0, rs=0
	data_t[2] = data_l|0x05|0x08;  //en=1, rs=0
	data_t[3] = data_l|0x01|0x08;  //en=0, rs=0
	HAL_I2C_Master_Transmit (&hi2c1, 0x27<<1,(uint8_t *) data_t, 4, 100);
}

void lcd_init (void)
{
	HAL_Delay(100);
	lcd_send_cmd (0x02);
	HAL_Delay(100);
	lcd_send_cmd (0x28);
	HAL_Delay(1);
	lcd_send_cmd (0x0C);
	HAL_Delay(1);
	lcd_send_cmd (0x80);
	HAL_Delay(1);
	lcd_send_cmd (0x01);
	HAL_Delay(1);
}

void lcd_print (char *str)
{
	while (*str) lcd_send_data (*str++);
}

void lcd_clear (void)
{
	lcd_send_cmd(0x01);
	HAL_Delay(1);
}

void lcd_firstLine (void)
{
	lcd_send_cmd(0x80);
	HAL_Delay(1);
}

void lcd_secondLine (void)
{
	lcd_send_cmd(0xC0);
	HAL_Delay(1);
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  file: The file name as string.
  * @param  line: The line in file as a number.
  * @retval None
  */
void _Error_Handler(char *file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
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
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
