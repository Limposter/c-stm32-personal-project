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
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "string.h"
#include "fonts.h"
#include "ssd1306.h"
#include "test.h"
#include "bitmap.h"
#include "sht31.h"
#include <stdbool.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define rx_buffer_size 100
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c2;

UART_HandleTypeDef huart1;

osThreadId Sensor_TaskHandle;
osThreadId OLED_TaskHandle;
osThreadId Button_TaskHandle;
osThreadId Uart_TaskHandle;
osThreadId Warning_TaskHandle;
osMessageQId temp_data_QueueHandle;
osMessageQId button_it_QueueHandle;
osMessageQId button_send_QueueHandle;
osMessageQId uart_QueueHandle;
osMessageQId humi_data_QueueHandle;
osMessageQId temp_warning_queueHandle;
osMessageQId humi_warning_queueHandle;
osMutexId myMutex01Handle;
/* USER CODE BEGIN PV */

uint8_t rx_input=0;
uint8_t rx_buffer[rx_buffer_size];
uint8_t rx_index=0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_I2C2_Init(void);
static void MX_USART1_UART_Init(void);
void Sensor_startTask(void const * argument);
void OLED_startTask(void const * argument);
void Button_startTask(void const * argument);
void Uart_startTask(void const * argument);
void Warning_startTask(void const * argument);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){  //xshell_input_interrupt
	if(huart->Instance==USART1){
		if(rx_input=='\r' || rx_input == '\n'){
			 rx_buffer[rx_index]='\0';           // this is the main item : if delete it, error

			uint8_t uart_cmd=0;
			if(strcmp((char*)rx_buffer, "start")==0){
				uart_cmd = 1;
				BaseType_t xHigherPriorityTaskWoken = pdFALSE;
			    xQueueSendFromISR(uart_QueueHandle, &uart_cmd, &xHigherPriorityTaskWoken);
			}
			else if(strcmp((char*)rx_buffer, "stop")==0){
				uart_cmd = 2;
				BaseType_t xHigherPriorityTaskWoken = pdFALSE;
				xQueueSendFromISR(uart_QueueHandle, &uart_cmd, &xHigherPriorityTaskWoken);
			}

			rx_index=0;
		}
		else{
			if(rx_index < rx_buffer_size-1){
				rx_buffer[rx_index++] = rx_input;
			}
		}
	}

	HAL_UART_Receive_IT(&huart1, &rx_input, 1);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){     // switch_iterrupt
	if(GPIO_Pin == GPIO_PIN_5){
		uint8_t button_cmd=1;
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;

		xQueueSendFromISR(button_it_QueueHandle, &button_cmd, &xHigherPriorityTaskWoken);
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
}

void Scan_I2C_Devices(I2C_HandleTypeDef *hi2c)
{
    char msg[32];
    for (uint8_t addr = 1; addr < 127; addr++)
    {
        if (HAL_I2C_IsDeviceReady(hi2c, (addr << 1), 2, 100) == HAL_OK)
        {
            snprintf(msg, sizeof(msg), "Found device at 0x%02X\r\n", addr);
            HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
        }
    }
}
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
  MX_I2C2_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  SSD1306_Init();

HAL_UART_Receive_IT(&huart1, &rx_input, 1);

Scan_I2C_Devices(&hi2c2);
/*
  SSD1306_GotoXY (0,0);  //Location(x,y)
  SSD1306_Puts ("LIM", &Font_11x18, 1);
  SSD1306_GotoXY (0, 30);
  SSD1306_Puts ("CHANGHYEON", &Font_11x18, 1);
  SSD1306_UpdateScreen();
  HAL_Delay (2000);
*/
  /* USER CODE END 2 */

  /* Create the mutex(es) */
  /* definition and creation of myMutex01 */
  osMutexDef(myMutex01);
  myMutex01Handle = osMutexCreate(osMutex(myMutex01));

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* definition and creation of temp_data_Queue */
  osMessageQDef(temp_data_Queue, 16, float);
  temp_data_QueueHandle = osMessageCreate(osMessageQ(temp_data_Queue), NULL);

  /* definition and creation of button_it_Queue */
  osMessageQDef(button_it_Queue, 16, uint16_t);
  button_it_QueueHandle = osMessageCreate(osMessageQ(button_it_Queue), NULL);

  /* definition and creation of button_send_Queue */
  osMessageQDef(button_send_Queue, 16, uint16_t);
  button_send_QueueHandle = osMessageCreate(osMessageQ(button_send_Queue), NULL);

  /* definition and creation of uart_Queue */
  osMessageQDef(uart_Queue, 16, uint16_t);
  uart_QueueHandle = osMessageCreate(osMessageQ(uart_Queue), NULL);

  /* definition and creation of humi_data_Queue */
  osMessageQDef(humi_data_Queue, 16, float);
  humi_data_QueueHandle = osMessageCreate(osMessageQ(humi_data_Queue), NULL);

  /* definition and creation of temp_warning_queue */
  osMessageQDef(temp_warning_queue, 16, uint16_t);
  temp_warning_queueHandle = osMessageCreate(osMessageQ(temp_warning_queue), NULL);

  /* definition and creation of humi_warning_queue */
  osMessageQDef(humi_warning_queue, 16, uint16_t);
  humi_warning_queueHandle = osMessageCreate(osMessageQ(humi_warning_queue), NULL);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of Sensor_Task */
  osThreadDef(Sensor_Task, Sensor_startTask, osPriorityNormal, 0, 512);
  Sensor_TaskHandle = osThreadCreate(osThread(Sensor_Task), NULL);

  /* definition and creation of OLED_Task */
  osThreadDef(OLED_Task, OLED_startTask, osPriorityAboveNormal, 0, 256);
  OLED_TaskHandle = osThreadCreate(osThread(OLED_Task), NULL);

  /* definition and creation of Button_Task */
  osThreadDef(Button_Task, Button_startTask, osPriorityRealtime, 0, 128);
  Button_TaskHandle = osThreadCreate(osThread(Button_Task), NULL);

  /* definition and creation of Uart_Task */
  osThreadDef(Uart_Task, Uart_startTask, osPriorityNormal, 0, 128);
  Uart_TaskHandle = osThreadCreate(osThread(Uart_Task), NULL);

  /* definition and creation of Warning_Task */
  osThreadDef(Warning_Task, Warning_startTask, osPriorityHigh, 0, 128);
  Warning_TaskHandle = osThreadCreate(osThread(Warning_Task), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  vTaskSuspend(Sensor_TaskHandle);
  /* USER CODE END RTOS_THREADS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
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
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
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
  hi2c1.Init.ClockSpeed = 400000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief I2C2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.ClockSpeed = 400000;
  hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
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
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

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
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, temp_warning_led_Pin|humi_warning_led_Pin, GPIO_PIN_SET);

  /*Configure GPIO pins : temp_warning_led_Pin humi_warning_led_Pin */
  GPIO_InitStruct.Pin = temp_warning_led_Pin|humi_warning_led_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : displaymode_button_Pin */
  GPIO_InitStruct.Pin = displaymode_button_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(displaymode_button_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* USER CODE BEGIN Header_Sensor_startTask */
/**
  * @brief  Function implementing the Sensor_Task thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_Sensor_startTask */
void Sensor_startTask(void const * argument)
{
  /* USER CODE BEGIN 5 */

	float temperature = 0.0f;
	float humidity = 0.0f;

	/* Infinite loop */
  for(;;)
  {
	  HAL_StatusTypeDef ret = SHT31_ReadTempHum(&hi2c2, &temperature, &humidity);


	  if (ret == HAL_OK){
		  xQueueSend(temp_data_QueueHandle, &temperature, 0);
		  xQueueSend(humi_data_QueueHandle, &humidity, 0);
	  }
	  else{
		  const char *msg = "SHT31 Read Error\r\n";
		  HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
	  }

	  osDelay(2000);
  }
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_OLED_startTask */
/**
* @brief Function implementing the OLED_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_OLED_startTask */
void OLED_startTask(void const * argument)
{
  /* USER CODE BEGIN OLED_startTask */

	float temperature = 0.0f, humidity = 0.0f;
	uint8_t display_mode = 0;
	uint8_t button_send_cmd = 0;
	char line[32];

	SSD1306_Init();
	SSD1306_Clear();
	SSD1306_UpdateScreen();

  /* Infinite loop */
  for(;;)
  {
	  if (xQueueReceive(button_send_QueueHandle, &button_send_cmd, 0) == pdPASS){
		 display_mode = (display_mode + 1) % 3;
	  }

	  BaseType_t temp_ok = xQueueReceive(temp_data_QueueHandle, &temperature, 10);
	  BaseType_t humi_ok = xQueueReceive(humi_data_QueueHandle, &humidity, 10);

	  SSD1306_Clear();

	  if (temp_ok == pdPASS && humi_ok == pdPASS) {

		  switch (display_mode)
		  {
		  case 0:
			  snprintf(line, sizeof(line), "Temp: %.2f C", temperature);
			  SSD1306_GotoXY(0, 0);
			  SSD1306_Puts(line, &Font_7x10, 1);
			  snprintf(line, sizeof(line), "Humi: %.2f %%", humidity);
			  SSD1306_GotoXY(0, 20);
			  SSD1306_Puts(line, &Font_7x10, 1);
			  break;

		  case 1:
			  snprintf(line, sizeof(line), "Temp: %.2f C", temperature);
			  SSD1306_GotoXY(0, 0);
			  SSD1306_Puts(line, &Font_7x10, 1);
			  break;

		  case 2:
			  snprintf(line, sizeof(line), "Humi: %.2f %%", humidity);
			  SSD1306_GotoXY(0, 0);
			  SSD1306_Puts(line, &Font_7x10, 1);
			  break;
		  }

		  if(temperature >= 29 || temperature <= 0){
			  uint8_t warning_cmd = 1;
			  xQueueSend(temp_warning_queueHandle, &warning_cmd, portMAX_DELAY);
		  }
		  if(humidity >= 70 || humidity <= 30){
			  uint8_t warning_cmd = 1;
			  xQueueSend(humi_warning_queueHandle, &warning_cmd, portMAX_DELAY);
		  }
		  //warning code

	  }
	  else {
		snprintf(line, sizeof(line), "Temp: 0.00");
		SSD1306_GotoXY(0, 0);
		SSD1306_Puts(line, &Font_7x10, 1);
		snprintf(line, sizeof(line), "Humi: 0.00");
		SSD1306_GotoXY(0, 20);
		SSD1306_Puts(line, &Font_7x10, 1);
	  }
	  SSD1306_UpdateScreen();
	  osDelay(2000);

  }
  /* USER CODE END OLED_startTask */
}

/* USER CODE BEGIN Header_Button_startTask */
/**
* @brief Function implementing the Button_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Button_startTask */
void Button_startTask(void const * argument)
{
  /* USER CODE BEGIN Button_startTask */
  uint8_t button_cmd;

  /* Infinite loop */
  for(;;)
  {
	  if(xQueueReceive(button_it_QueueHandle, &button_cmd, portMAX_DELAY)==pdPASS){

		  uint8_t button_send_cmd=1;
		  xQueueSend(button_send_QueueHandle, &button_send_cmd, portMAX_DELAY);

		  const char *msg="Display mode changed\r\n";
		  HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), 10);

		  taskYIELD();

	  }
  }
  /* USER CODE END Button_startTask */
}

/* USER CODE BEGIN Header_Uart_startTask */
/**
* @brief Function implementing the Uart_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Uart_startTask */
void Uart_startTask(void const * argument)
{
  /* USER CODE BEGIN Uart_startTask */
  /* Infinite loop */

  uint8_t uart_cmd_rx;
  for(;;)
  {
    if(xQueueReceive(uart_QueueHandle, &uart_cmd_rx, portMAX_DELAY)==pdPASS){
    	if(uart_cmd_rx==1){
    		char *msg = "sensor start\r\n";
			HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), 100);
    		vTaskResume(Sensor_TaskHandle);

    	}
    	else if(uart_cmd_rx==2){
    		char *msg = "sensor stop\r\n";
			HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), 100);
    		vTaskSuspend(Sensor_TaskHandle);

    	}
    }

  }
  /* USER CODE END Uart_startTask */
}

/* USER CODE BEGIN Header_Warning_startTask */
/**
* @brief Function implementing the Warning_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Warning_startTask */
void Warning_startTask(void const * argument)
{
  /* USER CODE BEGIN Warning_startTask */
  uint8_t temp_warning_cmd=0;
  uint8_t humi_warning_cmd=0;
  /* Infinite loop */
  for(;;)
  {
	 if (xQueueReceive(temp_warning_queueHandle, &temp_warning_cmd, 0) == pdPASS){
		 HAL_UART_Transmit(&huart1, (uint8_t*)"temp warning\r\n", 21, 10);
		 if (temp_warning_cmd == 1){
			 HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);  // temp warning on
		 }
	 }
	 else {
		 HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);  // temp off
	 }

	 if (xQueueReceive(humi_warning_queueHandle, &humi_warning_cmd, 0) == pdPASS){
		 HAL_UART_Transmit(&huart1, (uint8_t*)"humi warning\r\n", 21, 10);
		 if (humi_warning_cmd == 1){
			 HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_RESET);  // humi warning on
		 }
	 }
	 else {
		 HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_SET);  // humi off
	 }

	 osDelay(200);
  }
  /* USER CODE END Warning_startTask */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM4 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM4)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

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
