/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "liquidcrystal_i2c.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */
#define TIME_BUFF_SIZE 4
#define TIMEOUT 500
#define TIM_FREQ 48000000
#define DEFAULT_PLAYBACK 700
/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
//global flags to indicate if uart transmission is complete
volatile bool uart_tx_complete = true;
volatile bool uart_rx_complete = false;
volatile bool command_complete = true;
volatile bool play_alarm_melody = false;
volatile bool display_alarm = false;


const char* main_menu = "Welcome to Alarm Clock Setup\r\ns - set time (24h)\r\na - set alarm (24h)\r\nt - set alarm tone\r\r\n\n";
const char* time_menu = "Setting time/alarm (X1X2:X3X4)\r\n";
const char* tones_menu = "Available tones\r\n1 - upbeat melody\r\n2 - morning melody\r\n3 - imperial melody\r\n";
const char* receive_err = "Failed to process input. Try again.\r\n";

float upbeat_melody[] = {329.6275569134758, 391.99543598166935, 440.0, 391.99543598166935, 329.6275569134758, 293.66476791740746,
		  261.6255653005986, 293.66476791740746, 329.6275569134758, 329.6275569134758, 391.99543598166935, 440.0, 391.99543598166935,
		  329.6275569134758, 293.66476791740746, 261.6255653005986, 293.66476791740746, 329.6275569134758, 391.99543598166935, 329.6275569134758};
float morning_melody[] = {261.6255653005986, 329.6275569134758, 391.99543598166935, 440.0, 391.99543598166935, 329.6275569134758,
		  261.6255653005986, 195.99771799083467, 261.6255653005986, 349.2282314330038, 440.0, 391.99543598166935, 329.6275569134758,
		  293.66476791740746, 261.6255653005986, 195.99771799083467, 261.6255653005986, 293.66476791740746, 329.6275569134758,
		  391.99543598166935, 440.0, 523.2511306011972, 440.0, 391.99543598166935, 349.2282314330038, 293.66476791740746, 261.6255653005986};
float imperial_melody[] = {261.6255653005986, 391.99543598166935, 329.6275569134758, 261.6255653005986, 391.99543598166935,
		  329.6275569134758, 293.66476791740746, 261.6255653005986, 195.99771799083467, 391.99543598166935, 329.6275569134758,
		  261.6255653005986, 391.99543598166935, 329.6275569134758, 293.66476791740746, 261.6255653005986, 195.99771799083467,
		  349.2282314330038, 440.0, 391.99543598166935, 329.6275569134758, 261.6255653005986, 293.66476791740746, 261.6255653005986,
		  195.99771799083467, 261.6255653005986};

unsigned melody_arr_counter = 0;
unsigned melody_arr_size;
unsigned melody_playback_speed = DEFAULT_PLAYBACK;
float *melody_pointer;

I2C_HandleTypeDef hi2c1;
RTC_HandleTypeDef hrtc;
RTC_DateTypeDef Date;
RTC_TimeTypeDef Time;
RTC_AlarmTypeDef Alarm;
TIM_HandleTypeDef htim1;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void transmitData(UART_HandleTypeDef *huart, const uint8_t *pData);
void transmitDataByte(UART_HandleTypeDef *huart, const uint8_t *pData);
uint8_t* receiveData(UART_HandleTypeDef *huart, uint8_t *receiveBuff);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_RTC_Init(void);
static void MX_TIM1_Init(void);
static void MX_USART2_UART_Init(void);
void toneMenu();
void timeMenu();
void mainMenu();
int presForFrequency (int frequency);
void playTone(int *tone, int *duration, int *pause, int size);
void noTone (void);
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart);
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc);
int charToInt(uint8_t character);
void setAlarm(uint8_t *time_buff);
void setTime(uint8_t *time_buff);
void displayTime(volatile bool display_alarm);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

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
  MX_RTC_Init();
  MX_TIM1_Init();
  MX_USART2_UART_Init();
  HD44780_Init(2);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
  HAL_Delay(100);

  int time_index = 0;

  uint8_t time_command[1];
  uint8_t time_buff[TIME_BUFF_SIZE];
  uint8_t rx_buff[1];

  uint32_t timestamp_main = 0, timestamp_lcd = 0, timestamp_audio = 0; //when alarm callback is invoked will the variables be able to cycle inside of it to play the whole melody?

  HAL_UART_Receive_IT(&huart2, rx_buff, 1);
  transmitData(&huart2, (const uint8_t*) main_menu);
  //NOTE TO SELF: make the whole menu a function which is called inside the main while loop
  while (1){
	  if (uart_tx_complete && uart_rx_complete && command_complete){//ready to transmit
		if (rx_buff[0] == 's' || rx_buff[0] == 'a'){
			time_command[0] = rx_buff[0];

			transmitData(&huart2, (const uint8_t*) time_menu);

			command_complete = false;
			time_index = 0;
			uart_rx_complete = false;

			HAL_UART_Receive_IT(&huart2, rx_buff, 1);

			while (time_index < TIME_BUFF_SIZE){
				if (uart_rx_complete){
					uart_rx_complete = false;


				if (rx_buff[0] >= '0' && rx_buff[0] <= '9'){
					if (time_index == 0){ //first value can only be from 0 to 2
						if (rx_buff[0] >= '0' && rx_buff[0] <= '2'){
								time_buff[time_index] = rx_buff[0];
								time_index++;
						}else{
							transmitData(&huart2, (const uint8_t*) "Value must be between 0 and 2!\r\n");
						}
					}

					else if (time_index == 1){
						if (time_buff[0] == '0' || time_buff[0] == '1'){//if at index 0 values is 1 or 0 then value at index 1 can be from 0 to 9
							time_buff[time_index] = rx_buff[0];
							time_index++;
						}else{
							if (rx_buff[0] >= '0' && rx_buff[0] <= '3'){
									time_buff[time_index] = rx_buff[0];
									time_index++;
							}else{
								transmitData(&huart2, (const uint8_t*) "Value must be between 0 and 3!\r\n");
							}
						}
					}

					else if(time_index == 2){//if at index 0 value is 2  then value at index 1 can be from 0 to 3
						if (rx_buff[0] >= '0' && rx_buff[0] <= '5'){
							time_buff[time_index] = rx_buff[0];
							time_index++;
						}else{
							transmitData(&huart2, (const uint8_t*) "Value must be between 0 and 5!\r\n");
						}
					}

					else{
						time_buff[time_index] = rx_buff[0];
						time_index++;
					}

					if (time_index == TIME_BUFF_SIZE){
						break;
						}
					}
				}

				HAL_UART_Receive_IT(&huart2, rx_buff, 1);
			}

			command_complete = true;

			if (time_command[0] == 's'){
				setTime(time_buff);
				transmitData(&huart2, (const uint8_t*) "Time set!\r\r\n\n");
			}else {
				setAlarm(time_buff);
				transmitData(&huart2,(const uint8_t*) "Alarm set!\r\r\n\n");
			}

		}else if(rx_buff[0] == 't'){
			transmitData(&huart2, (const uint8_t*) tones_menu);
			command_complete = false;

			while (!command_complete){
				uart_rx_complete = false;
				HAL_UART_Receive_IT(&huart2, rx_buff, 1);

				if(rx_buff[0] == '1'){
					transmitData(&huart2, (const uint8_t*) "Alarm tone set to upbeat melody.\r\n");
					melody_pointer = &upbeat_melody[0];
					melody_arr_size = sizeof(upbeat_melody);
					command_complete = true;
				}else if(rx_buff[0] == '2'){
					transmitData(&huart2, (const uint8_t*) "Alarm tone set to morning melody.\r\n");
					melody_pointer = &morning_melody[0];
					melody_arr_size = sizeof(morning_melody);
					command_complete = true;
				}else if(rx_buff[0] == '3'){
					transmitData(&huart2, (const uint8_t*) "Alarm tone set to imperial melody.\r\n");
					melody_pointer = &imperial_melody[0];
					melody_arr_size = sizeof(imperial_melody);
					command_complete = true;
				}
			}
		}

		//resets flag and restart reception
		uart_rx_complete = false;
        HAL_UART_Receive_IT(&huart2, rx_buff, 1);
        transmitData(&huart2, (const uint8_t*) main_menu);
		}

	  timestamp_main = HAL_GetTick();

	   if (play_alarm_melody){ //if command that unlocks the playback command (using a callback flag when the alarm interrupt is called)
		   	if (timestamp_main - timestamp_audio >= melody_playback_speed && melody_arr_counter < melody_arr_size){
		   		__HAL_TIM_SET_PRESCALER(&htim1, presForFrequency(melody_pointer[melody_arr_counter]));
		   		melody_arr_counter++;
		   		timestamp_audio = timestamp_main;
		   	}

		   	if (melody_arr_counter == melody_arr_size){
		   		HD44780_Clear();
		   		melody_arr_counter = 0;
		   		display_alarm = false;
		   		play_alarm_melody = false;
		   	}
	   }

	   if (timestamp_main - timestamp_lcd >= TIMEOUT){
		   displayTime(display_alarm);
		   timestamp_lcd = timestamp_main;
	   	}
	  }

}

void transmitData(UART_HandleTypeDef *huart, const uint8_t *pData){
	if (uart_tx_complete){
		uart_tx_complete = false;
		uint16_t dataSize = strlen((const char*)pData);
		HAL_UART_Transmit_IT(huart, pData, dataSize);
		}
	}

void transmitDataByte(UART_HandleTypeDef *huart, const uint8_t *pData){
	if (uart_tx_complete){
		uart_tx_complete = false;
		HAL_UART_Transmit_IT(huart, pData, 1);
		}
	}
int presForFrequency (int frequency)// calculates prescaler value
{
	if (frequency == 0) return 0;
	return ((TIM_FREQ/(1000*frequency))-1);
}
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart){
	if(huart->Instance == huart2.Instance){
		uart_tx_complete = true;
	}
}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
	if(huart->Instance == huart2.Instance){
		uart_rx_complete = true;
	}
}
void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc){
		play_alarm_melody = true;
		display_alarm = true;
}
int charToInt(uint8_t character) {
    return (int) character - '0';
}
void setAlarm(uint8_t *time_buff){
	HAL_RTC_GetAlarm(&hrtc, &Alarm, RTC_ALARM_A, RTC_FORMAT_BIN);

	int hour_x1 = charToInt(time_buff[0]); //array indexing which automatically dererferences the pointers
	int hour_x2 = charToInt(time_buff[1]);
	int minute_x3 = charToInt(time_buff[2]);
	int minute_x4 = charToInt(time_buff[3]);

	Alarm.AlarmTime.Hours = hour_x1 * 10 + hour_x2;
	Alarm.AlarmTime.Minutes = minute_x3 * 10 + minute_x4;

	HAL_RTC_SetAlarm_IT(&hrtc, &Alarm, RTC_FORMAT_BIN);
}
void setTime(uint8_t *time_buff){
	HAL_RTC_GetTime(&hrtc, &Time, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &Date, RTC_FORMAT_BIN);

	int hour_x1 = charToInt(time_buff[0]); //array indexing which automatically dererferences the pointers
	int hour_x2 = charToInt(time_buff[1]);
	int minute_x3 = charToInt(time_buff[2]);
	int minute_x4 = charToInt(time_buff[3]);

	Time.Hours = hour_x1 * 10 + hour_x2;
	Time.Minutes = minute_x3 * 10 + minute_x4;

	HAL_RTC_SetTime(&hrtc, &Time, RTC_FORMAT_BIN);
}
void displayTime(volatile bool display_alarm){
	char ds_time_buffer[16]; //stores the formatted time (10 bytes)

	HAL_RTC_GetTime(&hrtc, &Time, RTC_FORMAT_BIN); //&Time only gives the address of the variable. & is not  a reference operator like in c++.
	HAL_RTC_GetDate(&hrtc, &Date, RTC_FORMAT_BIN); //also needs this to unlock shadow registers

    //Format: HH:MM:SS (constructs a "string" into the buffer)
    snprintf(ds_time_buffer, sizeof(ds_time_buffer), "%02d:%02d:%02d", Time.Hours, Time.Minutes, Time.Seconds);
    //Format: year:month:day (constructs a "string" into the buffer)

    HD44780_Home();
    HD44780_SetCursor(0, 0);
    HD44780_PrintStr("Time:   ");
    HD44780_PrintStr(ds_time_buffer);

    if (display_alarm){
        HD44780_SetCursor(0, 1);
        HD44780_PrintStr("ALARM!!!!!");
    }
}
/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  __HAL_FLASH_SET_LATENCY(FLASH_LATENCY_1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x10805D88;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};
  RTC_AlarmTypeDef sAlarm = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  hrtc.Init.OutPutPullUp = RTC_OUTPUT_PULLUP_NONE;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 0;
  sTime.Minutes = 0;
  sTime.Seconds = 45;
  sTime.SubSeconds = 0;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }
  sDate.WeekDay = RTC_WEEKDAY_MONDAY;
  sDate.Month = RTC_MONTH_JANUARY;
  sDate.Date = 1;
  sDate.Year = 0;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enable the Alarm A
  */
  sAlarm.AlarmTime.Hours = 0;
  sAlarm.AlarmTime.Minutes = 0;
  sAlarm.AlarmTime.Seconds = 0;
  sAlarm.AlarmTime.SubSeconds = 0;
  sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
  sAlarm.AlarmMask = RTC_ALARMMASK_NONE;
  sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
  sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
  sAlarm.AlarmDateWeekDay = 1;
  sAlarm.Alarm = RTC_ALARM_A;
  if (HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 0;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 1000;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 500;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.BreakFilter = 0;
  sBreakDeadTimeConfig.BreakAFMode = TIM_BREAK_AFMODE_INPUT;
  sBreakDeadTimeConfig.Break2State = TIM_BREAK2_DISABLE;
  sBreakDeadTimeConfig.Break2Polarity = TIM_BREAK2POLARITY_HIGH;
  sBreakDeadTimeConfig.Break2Filter = 0;
  sBreakDeadTimeConfig.Break2AFMode = TIM_BREAK_AFMODE_INPUT;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
  HAL_TIM_MspPostInit(&htim1);

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : LED_Pin */
  GPIO_InitStruct.Pin = LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
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
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
