/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
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
#include "stm32f7xx_hal.h"
#include "stm32f7xx_hal_dcmi.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "my_usb.h"
#include "usbd_cdc_if.h"
#include <stdint.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define CHUNK_SIZE 512
#define OV7670_ADDR (0x21 << 1)
#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

DCMI_HandleTypeDef hdcmi;
DMA_HandleTypeDef hdma_dcmi;

I2C_HandleTypeDef hi2c2;

/* USER CODE BEGIN PV */
volatile uint8_t frame_ready = 0;
uint8_t frame_buffer[320 * 240 * 2] __attribute__((section(".sram")));

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MPU_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_DCMI_Init(void);
static void MX_I2C2_Init(void);
/* USER CODE BEGIN PFP */
void HAL_DCMI_FrameEventCallback(DCMI_HandleTypeDef* hdcmi);
void capture_and_send(void);
void OV7670_HardwareReset(void);
uint8_t OV7670_ReadReg(uint8_t reg);
void OV7670_WriteReg(uint8_t reg, uint8_t value);
void send_frame_usb(void);
static void OV7670_Init_QVGA_RGB565(void);
static void OV7670_BarsTest(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {

    /* USER CODE BEGIN 1 */

    /* USER CODE END 1 */

    /* MPU Configuration--------------------------------------------------------*/
    MPU_Config();

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
    MX_DMA_Init();
    MX_DCMI_Init();
    MX_I2C2_Init();
    MX_USB_DEVICE_Init();
    /* USER CODE BEGIN 2 */
    HAL_Delay(3000);

    USB_Print("Resetting OV7670 Camera\r\n");
    OV7670_HardwareReset();
    USB_Print("Validating PID and Version of OV7670 Camera\r\n");
    uint8_t pid = OV7670_ReadReg(0x0A);
    uint8_t ver = OV7670_ReadReg(0x0B);
    USB_Print("PID: %02X VER: %02X\r\n", pid, ver);

    if (pid == 0x76) {
        USB_Print("OV7670 detected\r\n");

        OV7670_Init_QVGA_RGB565();

        USB_Print("OV7670 configured\r\n");
    } else {
        USB_Print("Camera not detected\r\n");
        exit(-1);
    }

    USB_Print("COM7  = %02X\r\n", OV7670_ReadReg(0x12));
    USB_Print("CLKRC = %02X\r\n", OV7670_ReadReg(0x11));
    USB_Print("COM15 = %02X\r\n", OV7670_ReadReg(0x40));
    USB_Print("RGB444= %02X\r\n", OV7670_ReadReg(0x8C));

    USB_Print("Beginning Capture and Transmit Sequence with OV7670\r\n");
    OV7670_Init_QVGA_RGB565();
    USB_Print("Performing Bar Test");
    OV7670_BarsTest();
    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1) {
        USB_Print("Sending Frame!!!!!!");
        capture_and_send();
        send_frame_usb();
        USB_Print("Frame sent\r\n");
        HAL_Delay(1000);
        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
    }
    /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /** Configure the main internal regulator output voltage
     */
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /** Initializes the RCC Oscillators according to the specified parameters
     * in the RCC_OscInitTypeDef structure.
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 8;
    RCC_OscInitStruct.PLL.PLLN = 216;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 9;
    RCC_OscInitStruct.PLL.PLLR = 2;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    /** Activate the Over-Drive mode
     */
    if (HAL_PWREx_EnableOverDrive() != HAL_OK) {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
     */
    RCC_ClkInitStruct.ClockType =
        RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK) {
        Error_Handler();
    }
    HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_HSE, RCC_MCODIV_1);
}

/**
 * @brief DCMI Initialization Function
 * @param None
 * @retval None
 */
static void MX_DCMI_Init(void) {

    /* USER CODE BEGIN DCMI_Init 0 */

    /* USER CODE END DCMI_Init 0 */

    /* USER CODE BEGIN DCMI_Init 1 */

    /* USER CODE END DCMI_Init 1 */
    hdcmi.Instance = DCMI;
    hdcmi.Init.SynchroMode = DCMI_SYNCHRO_HARDWARE;
    hdcmi.Init.PCKPolarity = DCMI_PCKPOLARITY_RISING;
    hdcmi.Init.VSPolarity = DCMI_VSPOLARITY_HIGH;
    hdcmi.Init.HSPolarity = DCMI_HSPOLARITY_LOW;
    hdcmi.Init.CaptureRate = DCMI_CR_ALL_FRAME;
    hdcmi.Init.ExtendedDataMode = DCMI_EXTEND_DATA_8B;
    hdcmi.Init.JPEGMode = DCMI_JPEG_DISABLE;
    hdcmi.Init.ByteSelectMode = DCMI_BSM_ALL;
    hdcmi.Init.ByteSelectStart = DCMI_OEBS_ODD;
    hdcmi.Init.LineSelectMode = DCMI_LSM_ALL;
    hdcmi.Init.LineSelectStart = DCMI_OELS_ODD;
    if (HAL_DCMI_Init(&hdcmi) != HAL_OK) {
        Error_Handler();
    }
    /* USER CODE BEGIN DCMI_Init 2 */

    /* USER CODE END DCMI_Init 2 */
}

/**
 * @brief I2C2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_I2C2_Init(void) {

    /* USER CODE BEGIN I2C2_Init 0 */

    /* USER CODE END I2C2_Init 0 */

    /* USER CODE BEGIN I2C2_Init 1 */

    /* USER CODE END I2C2_Init 1 */
    hi2c2.Instance = I2C2;
    hi2c2.Init.Timing = 0x20404768;
    hi2c2.Init.OwnAddress1 = 0;
    hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c2.Init.OwnAddress2 = 0;
    hi2c2.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    if (HAL_I2C_Init(&hi2c2) != HAL_OK) {
        Error_Handler();
    }

    /** Configure Analogue filter
     */
    if (HAL_I2CEx_ConfigAnalogFilter(&hi2c2, I2C_ANALOGFILTER_ENABLE) != HAL_OK) {
        Error_Handler();
    }

    /** Configure Digital filter
     */
    if (HAL_I2CEx_ConfigDigitalFilter(&hi2c2, 0) != HAL_OK) {
        Error_Handler();
    }
    /* USER CODE BEGIN I2C2_Init 2 */

    /* USER CODE END I2C2_Init 2 */
}

/**
 * Enable DMA controller clock
 */
static void MX_DMA_Init(void) {

    /* DMA controller clock enable */
    __HAL_RCC_DMA2_CLK_ENABLE();

    /* DMA interrupt init */
    /* DMA2_Stream1_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DMA2_Stream1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream1_IRQn);
}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    /* USER CODE BEGIN MX_GPIO_Init_1 */

    /* USER CODE END MX_GPIO_Init_1 */

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPIOE, OV_RET_Pin | OV_PWD_Pin, GPIO_PIN_RESET);

    /*Configure GPIO pins : OV_RET_Pin OV_PWD_Pin */
    GPIO_InitStruct.Pin = OV_RET_Pin | OV_PWD_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    /*Configure GPIO pin : OV_XCLK_Pin */
    GPIO_InitStruct.Pin = OV_XCLK_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF0_MCO;
    HAL_GPIO_Init(OV_XCLK_GPIO_Port, &GPIO_InitStruct);

    /* USER CODE BEGIN MX_GPIO_Init_2 */

    /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void HAL_DCMI_FrameEventCallback(DCMI_HandleTypeDef* hdcmi) {
    USB_Print("Frame callback\r\n");
    frame_ready = 1;
    HAL_DCMI_Stop(hdcmi);
}

void capture_and_send(void) {
    frame_ready = 0;

    // Start single-frame capture via DMA
    if (HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_SNAPSHOT, (uint32_t)frame_buffer,
                           (320 * 240 * 2) / 4) != HAL_OK) {
        USB_Print("DCMI DMA start failed\r\n");
        return;
    }
    // Wait for frame
    uint32_t start = HAL_GetTick();

    while (!frame_ready) {
        if (HAL_GetTick() - start > 3000) {
            USB_Print("Frame timeout\r\n");
            HAL_DCMI_Stop(&hdcmi);
            return;
        }
    }
    SCB_InvalidateDCache_by_Addr((uint32_t*)frame_buffer, sizeof(frame_buffer));
}

void send_frame_usb(void) {
    uint32_t total = 320 * 240 * 2;
    uint32_t offset = 0;

    while (offset < total) {
        uint32_t len = MIN(CHUNK_SIZE, total - offset);

        // Wait until USB is ready
        while (CDC_Transmit_FS(&frame_buffer[offset], len) == USBD_BUSY)
            ;

        offset += len;
        HAL_Delay(1); // Small delay to avoid overwhelming host
    }
}

void OV7670_HardwareReset(void) {
    HAL_GPIO_WritePin(GPIOE, OV_PWD_Pin, GPIO_PIN_RESET);

    HAL_GPIO_WritePin(GPIOE, OV_RET_Pin, GPIO_PIN_RESET);
    HAL_Delay(10);

    HAL_GPIO_WritePin(GPIOE, OV_RET_Pin, GPIO_PIN_SET);
    HAL_Delay(10);
}
uint8_t OV7670_ReadReg(uint8_t reg) {
    uint8_t data;

    HAL_I2C_Master_Transmit(&hi2c2, OV7670_ADDR, &reg, 1, HAL_MAX_DELAY);

    HAL_I2C_Master_Receive(&hi2c2, OV7670_ADDR, &data, 1, HAL_MAX_DELAY);

    return data;
}
void OV7670_WriteReg(uint8_t reg, uint8_t value) {
    uint8_t data[2] = {reg, value};

    HAL_I2C_Master_Transmit(&hi2c2, OV7670_ADDR, data, 2, HAL_MAX_DELAY);
}

static void OV7670_Init_QVGA_RGB565(void) {
    OV7670_WriteReg(0x12, 0x80); // Reset all registers
    HAL_Delay(100);
    OV7670_WriteReg(0x0C, 0x04);
    OV7670_WriteReg(0x11, 0x80); // 0x80 || 0x00 for default value OR'd with no scaling on PCLK
    OV7670_WriteReg(0x12, 0x14); // Set to QVGA and RGB output. RGB565 will be configured after
    OV7670_WriteReg(0x3E, 0x19);
    OV7670_WriteReg(0x40, 0xD0);
    OV7670_WriteReg(0x70, 0x3A);
    OV7670_WriteReg(0x71, 0x35);
    OV7670_WriteReg(0x72, 0x11);
    OV7670_WriteReg(0x73, 0xF1);
    OV7670_WriteReg(0x8C, 0x00);
    OV7670_WriteReg(0xA2, 0x02);

    OV7670_WriteReg(0x17, 0x16);
    OV7670_WriteReg(0x18, 0x04);
    OV7670_WriteReg(0x32, 0x80);

    OV7670_WriteReg(0x19, 0x02);
    OV7670_WriteReg(0x1A, 0x7A);
    OV7670_WriteReg(0x03, 0x0A);
}

static void OV7670_BarsTest() {
    // OV7670_WriteReg(0x12, 0x16); // Set to QVGA and RGB output. RGB565 will be configured after
    uint8_t com17 = OV7670_ReadReg(0x42);
    OV7670_WriteReg(0x42, com17 | 0x08);
}

/*
static void OV7670_Init_QVGA_RGB565(void) {
    OV7670_WriteReg(0x12, 0x80); // Reset all registers

    OV7670_WriteReg(0x12, 0x80);
    HAL_Delay(100);

    OV7670_WriteReg(0x12, 0x14);
    OV7670_WriteReg(0x40, 0xD0);
    OV7670_WriteReg(0x8C, 0x00);
    OV7670_WriteReg(0x3A, 0x04);

    OV7670_WriteReg(0x17, 0x16);
    OV7670_WriteReg(0x18, 0x04);
    OV7670_WriteReg(0x32, 0x80);

    OV7670_WriteReg(0x19, 0x02);
    OV7670_WriteReg(0x1A, 0x7A);
    OV7670_WriteReg(0x03, 0x0A);

    OV7670_WriteReg(0x0C, 0x00);
    OV7670_WriteReg(0x3E, 0x00);

    OV7670_WriteReg(0x70, 0x3A);
    OV7670_WriteReg(0x71, 0x35);
    OV7670_WriteReg(0x72, 0x11);
    OV7670_WriteReg(0x73, 0xF0);

    OV7670_WriteReg(0xA2, 0x02);

    OV7670_WriteReg(0x11, 0x01);

    HAL_Delay(50);
}
*/

/* USER CODE END 4 */

/* MPU Configuration */

void MPU_Config(void) {
    MPU_Region_InitTypeDef MPU_InitStruct = {0};

    /* Disables the MPU */
    HAL_MPU_Disable();

    /** Initializes and configures the Region and the memory to be protected
     */
    MPU_InitStruct.Enable = MPU_REGION_ENABLE;
    MPU_InitStruct.Number = MPU_REGION_NUMBER0;
    MPU_InitStruct.BaseAddress = 0x0;
    MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
    MPU_InitStruct.SubRegionDisable = 0x87;
    MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
    MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
    MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
    MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
    MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
    MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

    HAL_MPU_ConfigRegion(&MPU_InitStruct);
    /* Enables the MPU */
    HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while (1) {
    }
    /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t* file, uint32_t line) {
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
