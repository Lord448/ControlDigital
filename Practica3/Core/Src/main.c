#include "main.h"
#include "ssd1306.h"
#include "fonts.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"
#include <stdio.h>
#include <string.h>

#define SAMPLES 10
#define WriteMask 0xFFFFFF00
#define USER_DEBUG

typedef enum bool {
	false,
	true
}bool;

typedef enum btnStages{
	WaitingHigh,
	WaitingLow
}btnStages;

typedef struct Button {
	GPIO_TypeDef *GPIOx;
	uint16_t GPIO_Pin;
	uint16_t Highs;
	uint16_t Lows;
	btnStages Stages;
}Button;


ADC_HandleTypeDef hadc1;

I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim1; //Parsed Loop handler @ 2ms
TIM_HandleTypeDef htim3; //ADC Trigger - PSC @ 110

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM3_Init(void);
static void vTaskStart(void);
static void vTaskTimer(void);
static void vTaskOLED(void);
static void vTaskOLEDStart(void);
static uint16_t Debounce(Button *Button, uint16_t Samples);
static void ButtonInit(Button *Button, GPIO_TypeDef *GPIOx, uint16_t Pin);

#ifdef USER_DEBUG
bool ParsedFlag = true;
#endif

const uint32_t counterPeriod[4] = {65454, 6545, 1309, 654}; //10Hz 100Hz 500Hz 1KHz
int32_t FreqSelector = 0;
bool TurnOnFlag = false;
Button btnNext, btnPrev, btnStart;

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_ADC1_Init();
  MX_I2C1_Init();
  MX_TIM1_Init();
  MX_TIM3_Init();
  MX_USB_DEVICE_Init();
  ButtonInit(&btnNext, Next_GPIO_Port, Next_Pin);
  ButtonInit(&btnPrev, Prev_GPIO_Port, Prev_Pin);
  ButtonInit(&btnStart, Start_GPIO_Port, Start_Pin);
  SSD1306_Init();
  SSD1306_GotoXY(0, 0);
  SSD1306_Puts("Freq", &Font_11x18, 1);
  SSD1306_UpdateScreen();
  HAL_TIM_Base_Start_IT(&htim1);
  HAL_ADCEx_Calibration_Start(&hadc1);
  HAL_ADC_Start_IT(&hadc1);
  while (1)
  {
	  vTaskStart();
	  vTaskTimer();
	  vTaskOLED();
	  vTaskOLEDStart();
#ifdef USER_DEBUG
	  while(ParsedFlag);
	  ParsedFlag = true;
#else
	  HAL_SuspendTick();
	  HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
	  HAL_ResumeTick();
#endif
  }
}

static void vTaskStart(void)
{
	enum stages{
		Idle,
		Start,
		Stop
	}static stages = Idle;

	switch(stages)
	{
		case Idle:
			if(Debounce(&btnStart, SAMPLES) == 1)
				stages = Start;
		break;
		case Start:
			HAL_GPIO_WritePin(Transistor_GPIO_Port, Transistor_Pin, 1);
			HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
			TurnOnFlag = true;
			stages = Stop;
		break;
		case Stop:
			if(Debounce(&btnStart, SAMPLES) == 1)
			{
				HAL_GPIO_WritePin(Transistor_GPIO_Port, Transistor_Pin, 1);
				HAL_TIMEx_PWMN_Stop(&htim3, TIM_CHANNEL_1);
				TurnOnFlag = false;
				stages = Idle;
			}
		break;
	}
}
static void vTaskTimer(void)
{
	enum stages {
		CheckButton,
		SelectFreq
	}static stages = CheckButton;
	switch(stages)
	{
		case CheckButton:
			if(Debounce(&btnNext, SAMPLES) == 1) //Next button has been pressed
			{
				FreqSelector++;
				if(FreqSelector > 3)
					FreqSelector = 3;
				stages = SelectFreq;
			}
			else if(Debounce(&btnPrev, SAMPLES) == 1) //Prev button has been pressed
			{
				FreqSelector--;
				if(FreqSelector < 0)
					FreqSelector = 0;
				stages = SelectFreq;
			}
		break;
		case SelectFreq:
			TIM3 -> ARR = counterPeriod[FreqSelector];
			//Duty cycle @ 50%
			TIM3 -> CCR1 = counterPeriod[FreqSelector] / 2;
			stages = CheckButton;
		break;
	}
}

static void vTaskOLED(void)
{
	enum stages{
		ConstructBuffer,
		Goto,
		Print
	}static stages = ConstructBuffer;

	static char Buffer[7];
	static bool needToErase = false;
	const uint16_t y = 32;

	if(needToErase && FreqSelector != 3 && FreqSelector != 0)
	{
		SSD1306_GotoXY(46, y);
		SSD1306_Puts("     ", &Font_7x10, 1);
		SSD1306_UpdateScreen();
		needToErase = false;
	}
	switch(stages)
	{
		case ConstructBuffer:
			switch(FreqSelector)
			{
				case 0:
					sprintf(Buffer, "10Hz");
					stages = Goto;
				break;
				case 1:
					sprintf(Buffer, "100Hz");
					needToErase = true;
					stages = Goto;
				break;
				case 2:
					sprintf(Buffer, "500Hz");
					needToErase = true;
					stages = Goto;
				break;
				case 3:
					sprintf(Buffer, "1KHz");
					stages = Goto;
				break;
			}
		break;
		case Goto:
			switch(FreqSelector)
			{
				case 0:
					SSD1306_GotoXY(49, y);
					stages = Print;
				break;
				case 1:
					SSD1306_GotoXY(49, y);
					stages = Print;
				break;
				case 2:
					SSD1306_GotoXY(46, y);
					stages = Print;
				break;
				case 3:
					SSD1306_GotoXY(49, y);
					stages = Print;
				break;
			}
		break;
		case Print:
			SSD1306_Puts(Buffer, &Font_7x10, 1);
			SSD1306_UpdateScreen();
			stages = ConstructBuffer;
		break;

	}
}

static void vTaskOLEDStart(void)
{
	enum stages{
		Check,
		Print
	}static stages = Check;
	static bool isAlreadyPrinted = false;
	const uint16_t y = 10; //Adjust

	switch(stages)
	{
		case Check:
			if(TurnOnFlag && !isAlreadyPrinted)
				stages = Print;
		break;
		case Print:
			SSD1306_GotoXY(10, y);
			SSD1306_Puts("       ", &Font_11x18, 1);
			SSD1306_UpdateScreen();

			if(TurnOnFlag)
			{
				SSD1306_GotoXY(10, y);
				SSD1306_Puts("Started", &Font_11x18, 1);
				SSD1306_UpdateScreen();
			}
			else
			{
				SSD1306_GotoXY(10, y);
				SSD1306_Puts("Idle", &Font_11x18, 1);
				SSD1306_UpdateScreen();
			}
			stages = Check;
		break;
	}
}

/**
 * @brief This function debounce a terminal, it detects the falling edge
 * 		  and avoids the terminal noise and button bouncing. it is
 * 		  designed for parsed loop.
 * @note  It requires the declaration of the button structure and
 * 		  the typedef enum stages with the members WaitingHigh and WaitingLow.
 * 		  It also need to declare the function ButtonInit so the structure
 * 		  can have some values when this function its executed
 *
 * @param Button: Button that will be debounced
 * @param Samples: Samples to avoid the noise
 * @return uint16_t: 1 for activate, 0 for desactivate
 */
static uint16_t Debounce(Button *Button, uint16_t Samples)
{
	if(HAL_GPIO_ReadPin(Button -> GPIOx, Button -> GPIO_Pin) == 0)
	{
		Button -> Lows = Button -> Lows + 1;
		Button -> Highs = 0;
	}
	else
	{
		Button -> Highs = Button -> Highs + 1;
		Button -> Lows = 0;
	}
	switch(Button -> Stages)
	{
		case WaitingHigh:
			if(Button -> Highs == Samples)
			{
				Button -> Stages = WaitingLow;
			}
			return 0;
		break;
		case WaitingLow:
			if(Button -> Lows == Samples)
			{
				Button -> Stages = WaitingHigh;
				return 1; //Falling edge detected
			}
			else
				return 0;
		break;
		default:
			return 0;
		break;
	}
}

/**
 * @brief It initializes the button structure that is used in the
 * 		  debounce function
 * @note  It requires the declaration of the button structure and
 * 		  the typedef enum stages with the members WaitingHigh and WaitingLow.
 *
 * @param Button: Button that will be debounced
 * @param GPIOx: Port of the button
 * @param Pin: Pin of the button
 * @return none
 */
static void ButtonInit(Button *Button, GPIO_TypeDef *GPIOx, uint16_t Pin)
{
	Button -> GPIOx = GPIOx;
	Button -> GPIO_Pin = Pin;
	Button -> Highs = 0;
	Button -> Lows = 0;
	Button -> Stages = WaitingLow;
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) //Late 22.7us
{
	char Buffer[10];
	uint16_t ADC_Res = HAL_ADC_GetValue(hadc);
	uint32_t tmp, DACVal;

	HAL_GPIO_WritePin(ADCFreq_GPIO_Port, ADCFreq_Pin, 1);
	sprintf(Buffer, "%d \r\n", (int)ADC_Res);
	if(CDC_getReady() == USBD_OK)
	{
	  CDC_Transmit_FS((uint8_t *) Buffer, strlen(Buffer));
	}
	DACVal = (ADC_Res * 0xFF) / 4095; //Scaling data from 12bits to 8bits
	tmp = GPIOA -> ODR & WriteMask;
	GPIOA -> ODR = tmp | DACVal;
	HAL_GPIO_WritePin(ADCFreq_GPIO_Port, ADCFreq_Pin, 0);
}

#ifdef USER_DEBUG
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	ParsedFlag = false;
}
#endif

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC|RCC_PERIPHCLK_USB;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T3_TRGO;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_8;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

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

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 2;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 47999;
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
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 110;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 65454;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

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
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7
                          |Transistor_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(ADCFreq_GPIO_Port, ADCFreq_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PA0 PA1 PA2 PA3
                           PA4 PA5 PA6 PA7
                           Transistor_Pin */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7
                          |Transistor_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : ADCFreq_Pin */
  GPIO_InitStruct.Pin = ADCFreq_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(ADCFreq_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : Next_Pin Prev_Pin Start_Pin */
  GPIO_InitStruct.Pin = Next_Pin|Prev_Pin|Start_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

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
