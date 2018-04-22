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
#include <stdio.h>

/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c2;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim9;
TIM_HandleTypeDef htim10;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
int fadeVal = 0;
int fadeVal2 = 0;
volatile int state = 0; //0 - Listening, 1 - Order Received, 2 - Drink Loaded, 3 - Destination Reached, 4 - Drop drink, 5 - Return
int timerVal = 0;
// Manual/Autonomous Variables
volatile int mode = 0; //0 - manual, 1 - autonomous
volatile int PeriodCount = 0;
volatile int OnCount = 0;
//GPS Variables
int Rx_indx;
int GPS_Transfer_cplt = 0;
uint8_t Rx_data;
char Rx_Buffer[80];
char GPS_Data[60];
char CoordinateType[6];	//Determine if the current data received is GPGGA, GPGSA, GPRMC, or GPVTG, we only need RMC and GGA
char latDegBuff[3]; // "XX\0"
char latMinBuff[8]; // "XX.XXXX\0"
char longDegBuff[4]; // "XX\0"
char longMinBuff[8]; // "XX.XXXX\0"
char altitudeBuff[6];

int j = 0;
int k = 0;

//WiFi Variables
int WiFi_Transfer_cplt = 0;
uint8_t Rx1_data;
int Rx1_indx;
char Rx1_Buffer[40];
char targetLatDeg[3];
char targetLatMin[8];
char targetLongDeg[3];
char targetLongMin[8];
int order_received = 0;

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
	char drinkOrder[15];
} WiFi;

// UART Test Variables
char* bufftr = "Hello!\n\r";

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_I2C2_Init(void);
static void MX_TIM10_Init(void);
static void MX_TIM9_Init(void);
static void MX_TIM2_Init(void);

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);
                                
                                
                                

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

//LCD Functions
void lcd_send_cmd (char);
void lcd_send_data (char);
void lcd_print (char*);
void lcd_init(void);
void lcd_clear(void);

void lcd_firstLine(void);
void lcd_secondLine(void);
void uselessFunction(void);
/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    uint8_t i;
    uint8_t j;
    uint8_t k;

    //HAL_NVIC_DisableIRQ(TIM1_UP_TIM10_IRQn);

    if (huart->Instance == USART1)
	{
		if (Rx1_indx==0) {for (i=0;i<35;i++) Rx1_Buffer[i]=0;}   //clear Rx_Buffer before receiving new data

		if (Rx1_data != 13) //if received data different from ascii 13 (enter)
		{
			Rx1_Buffer[Rx1_indx++]=Rx1_data;    //add data to Rx_Buffer
		}
		else            //if received data = 13
		{
			Rx1_indx=0;

			WiFi_Transfer_cplt=1;//transfer complete, data is ready to read
		}

		HAL_UART_Receive_IT(&huart1, &Rx1_data, 1);   //activate UART receive interrupt every time
	}

    else if (huart->Instance == USART2)  //current UART
    {
        if (Rx_indx==0) {for (i=0;i<80;i++) Rx_Buffer[i]=0;}   //clear Rx_Buffer before receiving new data

        if (Rx_data != 10 && Rx_indx < 80) //if received data different from ascii 13 (enter)
        {
            Rx_Buffer[Rx_indx++]=Rx_data;    //add data to Rx_Buffer
        }
        else            //if received data = 13
        {
            Rx_indx=0;

            GPS_Transfer_cplt=1;//transfer complete, data is ready to read
        }

        HAL_UART_Receive_IT(&huart2, &Rx_data, 1);   //activate UART receive interrupt every time

    }

   // HAL_NVIC_EnableIRQ(TIM1_UP_TIM10_IRQn);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	// autonomous/manual select channel interpretation
	if (htim->Instance==TIM10)
	{
		//HAL_NVIC_DisableIRQ(USART2_IRQn);
		//sample Manual/Autonomous toggle signal
		if(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_4) == 1){
			OnCount++;
		}
		//check if end of a period
		if(PeriodCount++ == 183){
			//Set Mux Select Line
			if(OnCount >= 14){
 				HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_SET);
 				mode = 1;
			}
			else{
				HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_RESET);
				mode = 0;
			}
			//Reset Counts
			PeriodCount = 0;
			OnCount = 0;
		}
	}

	//HAL_NVIC_EnableIRQ(USART2_IRQn);
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
  int ManualDeliveryInterupt = 0;
  double StartAltitude = 0;
  int StatePrint = 1;
  char alt[16];
  double prevalt = 100000000;
  double maxalt = 0;
  double minalt = 100000000;
  int toggle = 0;

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
  MX_I2C1_Init();
  MX_TIM1_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_I2C2_Init();
  MX_TIM10_Init();
  MX_TIM9_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
  lcd_init();


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_RESET);

  	  //HAL_Delay(500);
  	  // END OF TEST CODE

  RMC.altitude = 0;

  //Start PWM signals
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim9, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim9, TIM_CHANNEL_2);

  HAL_UART_Receive_IT(&huart1, &Rx1_data, 1);
  HAL_UART_Receive_IT(&huart2, &Rx_data, 1);

  HAL_TIM_Base_Start_IT(&htim10);

  HAL_NVIC_DisableIRQ(USART2_IRQn);
  HAL_NVIC_DisableIRQ(TIM1_UP_TIM10_IRQn);

  //claw initializations
  __HAL_TIM_SET_COMPARE(&htim9, TIM_CHANNEL_1, 700); //PE5
  __HAL_TIM_SET_COMPARE(&htim9, TIM_CHANNEL_2, 700); //PE5
  //FC initializations
  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, 150); //FC channel 1 (Aileron) pin PE 14
  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, 150); //FC channel 2 (Elevator) pin PE 13
  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 150); //FC channel 3 (Throttle) pin PA 8
  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 150); //FC channel 4 (Rudder) pin PE 11
  __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 150); //FC channel 6 (IOC) pin PA 1

  HAL_NVIC_EnableIRQ(USART2_IRQn);
  HAL_NVIC_EnableIRQ(TIM1_UP_TIM10_IRQn);

  while (1)
  {
	  //GPS code
	 currTime = HAL_GetTick();

	  if (currTime - prevTimeLED >= 1000) {
		  toggle = 1;
		  prevTimeLED = currTime;
		  HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_13);
		  //HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_12);
	  }

	  if (GPS_Transfer_cplt) {
		  GPS_Transfer_cplt = 0;

		  for (j=0;j<6;j++) {
			CoordinateType[j] = Rx_Buffer[j];
		  }

		  j++;

		  for (k=0;k<60;k++) {
			GPS_Data[k] = Rx_Buffer[j];
			j++;
		  }

		  if (strcmp(CoordinateType, "$GPRMC") == 0)
		  {
			  RMC.fixStatus = GPS_Data[11];

			  if (RMC.fixStatus == 'V') {
					//  lcd_clear();
					//  lcd_print("NO GPS FIX");
			  }
			  else {

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
		  }

		  else if (strcmp(CoordinateType, "$GPGGA") == 0)
		  {
			  int a = 46;
			  int b = 0;

			  if (RMC.fixStatus == 'V') {
				// lcd_clear();
				 // lcd_print("NO GPS FIX");
			  }
			  else {

				  while(GPS_Data[a] != ','){
					  altitudeBuff[b] = GPS_Data[a];
					  a++;
					  b++;
				  }
				  altitudeBuff[b] = '\0';

				  RMC.altitude = atof(altitudeBuff);
			  }
		  }
	  }

	  if (WiFi_Transfer_cplt)
	  {
		  WiFi_Transfer_cplt = 0;
		  //LATITUDE
		  if (Rx1_Buffer[0] == '+')
		  {
			  WiFi.Latitude.orientation = 'N';
			  WiFi.orderCancelled = 0;
		  }
		  else if (Rx1_Buffer[0] == '-')
		  {
			  WiFi.Latitude.orientation = 'S';
			  WiFi.orderCancelled = 0;
		  }
		  else
		  {
			  WiFi.orderCancelled = 1;
			  lcd_clear();
			  lcd_print("Drink Cancelled");
		  }

		  if (WiFi.orderCancelled == 0)
		  {
			  //Latitude Degrees
			  targetLatDeg[0] = Rx1_Buffer[1];
			  targetLatDeg[1] = Rx1_Buffer[2];
			  targetLatDeg[2] = '\0';

			  WiFi.Latitude.degrees = atoi(targetLatDeg);

			  //Latitude decimalMin
			  targetLatMin[0] = Rx1_Buffer[4];
			  targetLatMin[1] =Rx1_Buffer[5];
			  targetLatMin[2] = '.';
			  targetLatMin[3] = Rx1_Buffer[6];
			  targetLatMin[4] = Rx1_Buffer[7];
			  targetLatMin[5] = Rx1_Buffer[8];
			  targetLatMin[6] = Rx1_Buffer[9];
			  targetLatMin[7] = '\0';

			  WiFi.Latitude.decimalMin = atof(targetLatMin);


			  //LONGITUDE
			  if (Rx1_Buffer[13] == '+')
			  {
				WiFi.Longitude.orientation = 'E';
			  }
			  else if (Rx1_Buffer[13] == '-')
			  {
				WiFi.Longitude.orientation = 'W';
			  }

			  //Longitude Degrees
			  targetLongDeg[0] = Rx1_Buffer[14];
			  targetLatDeg[1] = Rx1_Buffer[15];
			  targetLatDeg[2] = '\0';

			  WiFi.Longitude.degrees = atoi(targetLongDeg);

			  //Longitude decimalMin
			  targetLongMin[0] = Rx1_Buffer[17];
			  targetLongMin[1] = Rx1_Buffer[18];
			  targetLongMin[2] = '.';
			  targetLongMin[3] = Rx1_Buffer[19];
			  targetLongMin[4] = Rx1_Buffer[20];
			  targetLongMin[5] = Rx1_Buffer[21];
			  targetLongMin[6] = Rx1_Buffer[22];
			  targetLongMin[7] = '\0';

			  WiFi.Longitude.decimalMin = atof(targetLongMin);


			  //DRINK ORDER
			  int a = 23;
			  int b = 0;

			  while (Rx1_Buffer[a] != ',')
			  {
				  WiFi.drinkOrder[b] = Rx1_Buffer[a];
				  a++;
				  b++;
			  }
			  WiFi.drinkOrder[b] = '\0';
			  order_received = 1;
		  }
	  }

  	  //Drone States code
	  switch(state){
	  case 0: //idle
		  //default initializations, but should not be used until state 3
		  ManualDeliveryInterupt = 0;

		  if(StatePrint == 1 || (toggle == 1/*RMC.altitude < prevalt-1*/ && RMC.altitude != 0)){
			  toggle = 0;
			  if(StatePrint == 0){
				  prevalt = RMC.altitude;
				  if(RMC.altitude < prevalt){
					  maxalt = RMC.altitude;
				  }
				  if(RMC.altitude > prevalt){
					  minalt = RMC.altitude;
				  }
			  }

			  StatePrint = 0;
			  lcd_clear();
			  //lcd_firstLine();
			  lcd_print("Altitude"); //for test
			  lcd_secondLine();
		  	  sprintf(alt, "%d", (int) prevalt);
		  	  lcd_print(alt); //for test
		  }
		  //for testing ascend descend
		  if(mode == 1){
			  //lcd_print("test");
			  StatePrint = 1;
			  state = 3;
		  }
		 /* if(order_received){
			StatePrint = 1;
		   	state = 1;
	  	  }*/
		  break;
	  case 1: // order received
		  if(StatePrint == 1){
		  	StatePrint = 0;
		  	lcd_clear();
		  	lcd_print("Order Received!"); //also display which drink
		  	lcd_secondLine();
		  	lcd_print(WiFi.drinkOrder);	//Drink order!!
		  }
		 if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_15) == 0) {
			//close claw
			__HAL_TIM_SET_COMPARE(&htim9, TIM_CHANNEL_2, 460);
			__HAL_TIM_SET_COMPARE(&htim9, TIM_CHANNEL_1, 940);
			StatePrint = 1;
		   	state = 2;
		  }
		  break;
	  case 2: //order complete
		  if(StatePrint == 1){
		  	StatePrint = 0;
		  	lcd_clear();
		  	lcd_print("Payload loaded! ");
		  	lcd_secondLine();
		  	lcd_print("Position & Start");
		  }
		  //start ascent if mode is autonomous
		  if(mode == 1){
			  StatePrint = 1;
			  state = 3;
		  }
		  break;
	  case 3: //ascend
		  //save start location
		  StartAltitude = RMC.altitude;
		  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 150); //FC channel 3 (Throttle) pin PA 8

		  /* if mode == 0
		   * 	ManualDeliveryInterupt = 1;
		   * 	state = 7; descend state
		   */

		  if(StatePrint == 1 /*|| (RMC.altitude >= curralt || RMC.altitude < curralt - 1*/){
			  //curralt = RMC.altitude;
			  StatePrint = 0;
		  	  lcd_clear();
		  	  lcd_firstLine();
		  	  sprintf(alt, "%d", (int) minalt);
		  	  lcd_secondLine();
		  	  sprintf(alt, "%d", (int) maxalt);
		  	  lcd_print(alt); //for test
		   }
		  if(mode == 0){
			  StatePrint = 1;
			  state = 0;
		  }
		  //go to descend state
		   /*if(RMC.altitude <= StartAltitude - 5){
			   StatePrint = 1;
		       //state = 4;
		       state = 7;
		   }*/
		  break;
	  case 4: //go to target
		  //fly towards target directional input
		  /* if(target.longitude == currLocation.longitude)
		   * if(target.lattitdue == currLocation.lattitude)
		   * if(target.longitude < currLocation.longitude && target.lattitude < currLocation.lattitude) //SW quad
		   * if(target.longitude < currLocation.longitude && target.tattidud
		   */
		  /* if mode == 0
		   * 	ManualDeliveryInterupt = 1;
		   * 	state = 6; return to start state
		   * if near enough to target
		   * 	state = 5;
		   */
		  break;
	  case 5: //release
		  //hover signals
		  /*AileronValue = 60;
		  ElevatorValue = 60;
		  ThrottleValue = 60;
		  RudderValue = 60;*/
		 // if(mode == 0){
			__HAL_TIM_SET_COMPARE(&htim9, TIM_CHANNEL_2, 360);
			__HAL_TIM_SET_COMPARE(&htim9, TIM_CHANNEL_1, 700);

		 // }
		  /* if mode == 0
		   * 	ManualDeliveryInterupt = 1;
		   * 	state = return to start state;
		   * if can released
		   * 	state = 6;
		   */
		  break;
	  case 6: //return to start location
		  /* if mode == 0
		   * 	ManualDeliveryInterupt = 1;
		   * if current location within range of start location
		   * 	state = 7;
		   */
		  break;
	  case 7: //descend
		  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 130); //slow descent speed
		  /* if mode == 0
		   * 	ManualDeliveryInterupt = 1;
		   */
		   if(RMC.altitude >= StartAltitude - 1 || RMC.altitude <= StartAltitude + 1){
			   __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 90); //power down motors
		   /* 	if ManualDeliveryInterrupt
		   * 		print delivery was interrupted
		   * 	else
		   * 		print order complete
		   */
		    	if(StatePrint == 1){
		    		StatePrint = 0;
		    		lcd_clear();
		    		lcd_print("Ta Daaa!");
		    	}
		   }
		  break;
	  }

  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */

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
  RCC_OscInitStruct.PLL.PLLN = 100;
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
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
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

/* I2C2 init function */
static void MX_I2C2_Init(void)
{

  hi2c2.Instance = I2C2;
  hi2c2.Init.ClockSpeed = 100000;
  hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* TIM1 init function */
static void MX_TIM1_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;
  TIM_OC_InitTypeDef sConfigOC;
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig;

  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 1000;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 1832;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  HAL_TIM_MspPostInit(&htim1);

}

/* TIM2 init function */
static void MX_TIM2_Init(void)
{

  TIM_MasterConfigTypeDef sMasterConfig;
  TIM_OC_InitTypeDef sConfigOC;

  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 500;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 1832;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  HAL_TIM_MspPostInit(&htim2);

}

/* TIM9 init function */
static void MX_TIM9_Init(void)
{

  TIM_OC_InitTypeDef sConfigOC;

  htim9.Instance = TIM9;
  htim9.Init.Prescaler = 198;
  htim9.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim9.Init.Period = 5000;
  htim9.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  if (HAL_TIM_PWM_Init(&htim9) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim9, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  if (HAL_TIM_PWM_ConfigChannel(&htim9, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  HAL_TIM_MspPostInit(&htim9);

}

/* TIM10 init function */
static void MX_TIM10_Init(void)
{

  TIM_OC_InitTypeDef sConfigOC;

  htim10.Instance = TIM10;
  htim10.Init.Prescaler = 5000;
  htim10.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim10.Init.Period = 1;
  htim10.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  if (HAL_TIM_Base_Init(&htim10) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  if (HAL_TIM_OC_Init(&htim10) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sConfigOC.OCMode = TIM_OCMODE_TIMING;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_OC_ConfigChannel(&htim10, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* USART1 init function */
static void MX_USART1_UART_Init(void)
{

  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
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
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);

  /*Configure GPIO pin : PA4 */
  GPIO_InitStruct.Pin = GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PA5 */
  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PC4 */
  GPIO_InitStruct.Pin = GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PC5 */
  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB15 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PB13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

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

void uselessFunction (void)
{
	int x = 2;

	x++;
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
