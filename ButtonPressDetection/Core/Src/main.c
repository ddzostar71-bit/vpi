/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body - Button Press Detection with FreeRTOS
  * @author         : Generated for STM32G474 Lab Work
  ******************************************************************************
  * @attention
  *
  * Задача:
  * - Задача1 отслеживает нажатие кнопки PB15
  * - Различает одинарное и двойное (в течение 1 сек) нажатие
  * - Информация передается Задаче2 через Queue FreeRTOS
  * - Задача2 отображает тип нажатия миганием 1 или 2 светодиодов
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include <string.h>
#include <stdio.h>

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart3;

/* Definitions for FreeRTOS --------------------------------------------------*/
osThreadId_t ButtonTaskHandle;
osThreadId_t LEDTaskHandle;
osMessageQueueId_t ButtonQueueHandle;

const osThreadAttr_t ButtonTask_attributes = {
  .name = "ButtonTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

const osThreadAttr_t LEDTask_attributes = {
  .name = "LEDTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Button press type enumeration */
typedef enum {
    BUTTON_SINGLE_PRESS = 1,
    BUTTON_DOUBLE_PRESS = 2
} ButtonPressType_t;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART3_UART_Init(void);
void StartButtonTask(void *argument);
void StartLEDTask(void *argument);
void UART_Print(const char *msg);

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART3_UART_Init();

  /* Init scheduler */
  osKernelInitialize();

  /* Create the queue(s) */
  ButtonQueueHandle = osMessageQueueNew(5, sizeof(ButtonPressType_t), NULL);

  /* creation of ButtonTask */
  ButtonTaskHandle = osThreadNew(StartButtonTask, NULL, &ButtonTask_attributes);

  /* creation of LEDTask */
  LEDTaskHandle = osThreadNew(StartLEDTask, NULL, &LEDTask_attributes);

  /* Start scheduler */
  UART_Print("System started. Monitoring button PB15...\r\n");
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */
  while (1)
  {
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

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV4;
  RCC_OscInitStruct.PLL.PLLN = 85;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART3 Initialization Function (115200 8N1)
  * @param None
  * @retval None
  */
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
  huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart3.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();

  /*Configure GPIO pin Output Level - LEDs PE0, PE1 initially OFF */
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_0|GPIO_PIN_1, GPIO_PIN_RESET);

  /*Configure GPIO pins : PE0 PE1 (LED outputs) */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pin : PB15 (Button input with pull-up) */
  GPIO_InitStruct.Pin = GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

/**
  * @brief UART Print helper function
  * @param msg: Message to print
  * @retval None
  */
void UART_Print(const char *msg)
{
  HAL_UART_Transmit(&huart3, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
}

/**
  * @brief  Task 1: Button monitoring task
  * @details Monitors button PB15 and detects single/double press
  *          - Single press: one press
  *          - Double press: two presses within 1 second
  * @param  argument: Not used
  * @retval None
  */
void StartButtonTask(void *argument)
{
  uint32_t last_press_time = 0;
  uint8_t press_count = 0;
  GPIO_PinState button_prev_state = GPIO_PIN_SET;  // Button not pressed (pull-up)
  GPIO_PinState button_curr_state;
  ButtonPressType_t press_type;
  char msg[64];

  for(;;)
  {
    button_curr_state = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_15);

    // Detect button press (transition from HIGH to LOW)
    if (button_prev_state == GPIO_PIN_SET && button_curr_state == GPIO_PIN_RESET)
    {
      uint32_t current_time = osKernelGetTickCount();

      // Check if this is within 1 second of last press
      if (press_count > 0 && (current_time - last_press_time) <= 1000)
      {
        // Double press detected
        press_type = BUTTON_DOUBLE_PRESS;
        press_count = 0;  // Reset counter

        sprintf(msg, "[ButtonTask] Double press detected!\r\n");
        UART_Print(msg);

        // Send to queue
        osMessageQueuePut(ButtonQueueHandle, &press_type, 0, 0);
      }
      else
      {
        // First press or timeout occurred
        press_count = 1;
        last_press_time = current_time;
      }

      // Debounce delay
      osDelay(50);
    }

    // Check if we have a single press that timed out (no second press)
    if (press_count == 1 && (osKernelGetTickCount() - last_press_time) > 1000)
    {
      // Single press confirmed
      press_type = BUTTON_SINGLE_PRESS;
      press_count = 0;  // Reset counter

      sprintf(msg, "[ButtonTask] Single press detected!\r\n");
      UART_Print(msg);

      // Send to queue
      osMessageQueuePut(ButtonQueueHandle, &press_type, 0, 0);
    }

    button_prev_state = button_curr_state;
    osDelay(10);  // Poll every 10ms
  }
}

/**
  * @brief  Task 2: LED indication task
  * @details Receives button press type from queue and blinks LEDs:
  *          - Single press: Blink LED1 (PE0) 3 times
  *          - Double press: Blink LED1 and LED2 (PE0, PE1) 3 times
  * @param  argument: Not used
  * @retval None
  */
void StartLEDTask(void *argument)
{
  ButtonPressType_t press_type;
  osStatus_t status;
  char msg[64];

  for(;;)
  {
    // Wait for message from queue
    status = osMessageQueueGet(ButtonQueueHandle, &press_type, NULL, osWaitForever);

    if (status == osOK)
    {
      if (press_type == BUTTON_SINGLE_PRESS)
      {
        sprintf(msg, "[LEDTask] Indicating SINGLE press - blinking 1 LED\r\n");
        UART_Print(msg);

        // Blink only LED1 (PE0) 3 times
        for (int i = 0; i < 3; i++)
        {
          HAL_GPIO_WritePin(GPIOE, GPIO_PIN_0, GPIO_PIN_SET);
          osDelay(200);
          HAL_GPIO_WritePin(GPIOE, GPIO_PIN_0, GPIO_PIN_RESET);
          osDelay(200);
        }
      }
      else if (press_type == BUTTON_DOUBLE_PRESS)
      {
        sprintf(msg, "[LEDTask] Indicating DOUBLE press - blinking 2 LEDs\r\n");
        UART_Print(msg);

        // Blink both LED1 (PE0) and LED2 (PE1) 3 times
        for (int i = 0; i < 3; i++)
        {
          HAL_GPIO_WritePin(GPIOE, GPIO_PIN_0 | GPIO_PIN_1, GPIO_PIN_SET);
          osDelay(200);
          HAL_GPIO_WritePin(GPIOE, GPIO_PIN_0 | GPIO_PIN_1, GPIO_PIN_RESET);
          osDelay(200);
        }
      }
    }
  }
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM6) {
    HAL_IncTick();
  }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  __disable_irq();
  while (1)
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
void assert_failed(uint8_t *file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
}
#endif /* USE_FULL_ASSERT */
