/******************************************************************************
 *
 * @brief     This file is part of the TouchGFX 4.5.0 evaluation distribution.
 *
 * @author    Draupner Graphics A/S <http://www.touchgfx.com>
 *
 ******************************************************************************
 *
 * @section Copyright
 *
 * Copyright (C) 2014-2016 Draupner Graphics A/S <http://www.touchgfx.com>.
 * All rights reserved.
 *
 * TouchGFX is protected by international copyright laws and the knowledge of
 * this source code may not be used to write a similar product. This file may
 * only be used in accordance with a license and should not be re-
 * distributed in any way without the prior permission of Draupner Graphics.
 *
 * This is licensed software for evaluation use, any use must strictly comply
 * with the evaluation license agreement provided with delivery of the
 * TouchGFX software.
 *
 * The evaluation license agreement can be seen on www.touchgfx.com
 *
 * @section Disclaimer
 *
 * DISCLAIMER OF WARRANTY/LIMITATION OF REMEDIES: Draupner Graphics A/S has
 * no obligation to support this software. Draupner Graphics A/S is providing
 * the software "AS IS", with no express or implied warranties of any kind,
 * including, but not limited to, any implied warranties of merchantability
 * or fitness for any particular purpose or warranties against infringement
 * of any proprietary rights of a third party.
 *
 * Draupner Graphics A/S can not be held liable for any consequential,
 * incidental, or special damages, or any other relief, or for any claim by
 * any third party, arising from your use of this software.
 *
 *****************************************************************************/
#include <common/TouchGFXInit.hpp>
#include <touchgfx/hal/BoardConfiguration.hpp>
#include <platform/hal/ST/mcu/stm32f4cube/STM32F4DMA.hpp>
#include <platform/hal/ST/mcu/stm32f4cube/STM32F4HAL_DSI.hpp>
#include <touchgfx/hal/GPIO.hpp>
#include <platform/core/arm/cortex-m/CortexMMCUInstrumentation.hpp>
#include <platform/driver/lcd/LCD16bpp.hpp>
#include <platform/driver/lcd/LCD24bpp.hpp>
#include <bsp/OTM8009TouchController.hpp>

/**
 * In order to use double buffering, simply enable the USE_DOUBLE_BUFFERING #define below
 * Enabling double buffering will yield a performance increase (potentionally higher frame rate),
 * at the expense of consuming an additional frame buffer's worth of external RAM - e.g. 800 * 480 * 2 bytes (16 bpp)
 */
//#define USE_DOUBLE_BUFFERING 1

/***********************************************************
 ******         24 Bits Per Pixel Support            *******
 ***********************************************************
 *
 * The default bit depth of the framebuffer is 16bpp. If you want 24bpp support, define the symbol "USE_BPP" with a value
 * of "24", e.g. "USE_BPP=24". This symbol affects the following:
 *
 * 1. Type of TouchGFX LCD (16bpp vs 24bpp)
 * 2. Bit depth of the framebuffer(s)
 * 3. TFT controller configuration.
 *
 * WARNING: Remember to modify your image formats accordingly in app/config/. Please see the following knowledgebase article
 * for further details on how to choose and configure the appropriate image formats for your application:
 *
 * https://touchgfx.zendesk.com/hc/en-us/articles/206725849
 */

extern "C"
{
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_dsi.h"
#include "stm32469i_discovery_sdram.h"
#include "stm32469i_discovery_qspi.h"
#include "stm32469i_discovery.h"
#include "stm32469i_discovery_lcd.h"
    void OS_SysTick(void);
    bool os_inited = false;
    void SysTick_Handler(void)
    {
        HAL_IncTick();
        if (os_inited)
        {
            OS_SysTick();
        }
    }

    extern LTDC_HandleTypeDef hltdc_eval;
    extern DSI_HandleTypeDef hdsi_eval;
    DSI_CmdCfgTypeDef CmdCfg;
    DSI_LPCmdTypeDef LPCmd;
    DSI_PLLInitTypeDef dsiPllInit;
    static RCC_PeriphCLKInitTypeDef  PeriphClkInitStruct;


    /* Private define ------------------------------------------------------------*/
#define VSYNC           1
#define VBP             1
#define VFP             1
#define VACT            480
#define HSYNC           1
#define HBP             1
#define HFP             1
#define LAYER0_ADDRESS  (LCD_FB_START_ADDRESS)

#if !defined(USE_BPP) || USE_BPP==16
#define HACT            200      /* Note: 16bpp: Screen divided in 4 areas of 200 pixels to avoid DSI tearing */
#elif USE_BPP==24
#define HACT            400      /* Note: 24bpp: Screen divided in 2 areas of 400 pixels to avoid DSI tearing */
#else
#error Unknown USE_BPP
#endif
}


// 16 bpp screen map: 4 areas of 200 pixels
uint8_t pCols[4][4] =
{
    {0x00, 0x00, 0x00, 0xC7}, /*   0 -> 199 */
    {0x00, 0xC8, 0x01, 0x8F}, /* 200 -> 399 */
    {0x01, 0x90, 0x02, 0x57}, /* 400 -> 599 */
    {0x02, 0x58, 0x03, 0x1F}, /* 600 -> 799 */
};

// 24 bpp screen map: 2 areas of 400 pixels
uint8_t pColLeft[]    = {0x00, 0x00, 0x01, 0x8F}; /*   0 -> 399 */
uint8_t pColRight[]   = {0x01, 0x90, 0x03, 0x1F}; /* 400 -> 799 */

uint8_t pPage[]       = {0x00, 0x00, 0x01, 0xDF}; /*   0 -> 479 */
uint8_t pScanCol[]    = {0x02, 0x15};             /* Scan @ 533 */

static uint32_t frameBuf0 = (uint32_t)(LAYER0_ADDRESS); //Beginning of SDRAM

static void SystemClock_Config();
static void LTDC_Init();
static uint8_t LCD_Init(void);
static void LCD_LayerInit(uint16_t LayerIndex, uint32_t Address);

namespace touchgfx
{
void hw_init()
{
    HAL_Init();

    /* Configure the system clock to 180 MHz */
    SystemClock_Config();

    /* Initialize the SDRAM */
    BSP_SDRAM_Init();

    /* Initialize the QSPI */
    BSP_QSPI_Init();
    BSP_QSPI_MemoryMappedMode();
    HAL_NVIC_DisableIRQ(QUADSPI_IRQn);

    LCD_Init();

    /* Initialize LTDC layer 0 iused for Hint */
    LCD_LayerInit(0, LAYER0_ADDRESS);
    BSP_LCD_SelectLayer(0);

#if !defined(USE_BPP) || USE_BPP==16
    HAL_DSI_LongWrite(&hdsi_eval, 0, DSI_DCS_LONG_PKT_WRITE, 4, OTM8009A_CMD_CASET, pCols[0]);
#elif USE_BPP==24
    HAL_DSI_LongWrite(&hdsi_eval, 0, DSI_DCS_LONG_PKT_WRITE, 4, OTM8009A_CMD_CASET, pColLeft);
#else
#error Unknown USE_BPP
#endif

    HAL_DSI_LongWrite(&hdsi_eval, 0, DSI_DCS_LONG_PKT_WRITE, 4, OTM8009A_CMD_PASET, pPage);

    /* Update pitch : the draw is done on the whole physical X Size */
    HAL_LTDC_SetPitch(&hltdc_eval, BSP_LCD_GetXSize(), 0);

    GPIO::init();

}
STM32F4DMA dma;
OTM8009TouchController tc;
CortexMMCUInstrumentation mcuInstr;

#if !defined(USE_BPP) || USE_BPP==16
LCD16bpp display;
uint16_t bitDepth = 16;
#elif USE_BPP==24
LCD24bpp display;
uint16_t bitDepth = 24;
#else
#error Unknown USE_BPP
#endif

void touchgfx_init()
{
    uint16_t dispWidth = 800;
    uint16_t dispHeight = 480;
    HAL& hal = touchgfx_generic_init<STM32F4HAL_DSI>(dma, display, tc, dispWidth, dispHeight, 0, 0);
    os_inited = true;

#ifdef USE_DOUBLE_BUFFERING
    hal.setFrameBufferStartAddress((uint16_t*)frameBuf0, bitDepth);
#else
    hal.setFrameBufferStartAddress((uint16_t*)frameBuf0, bitDepth, false);
#endif

    // By default frame rate compensation is off.
    // Enable frame rate compensation to smooth out animations in case there is periodic slow frame rates.
    hal.setFrameRateCompensation(false);

    hal.setTouchSampleRate(2);
    hal.setFingerSize(40);

    // This platform can handle simultaneous DMA and TFT accesses to SDRAM, so disable lock to increase performance.
    hal.lockDMAToFrontPorch(false);

    mcuInstr.init();

    //Set MCU instrumentation and Load calculation
    hal.setMCUInstrumentation(&mcuInstr);
    hal.enableMCULoadCalculation(true);
}
}

extern "C"
{
    /**
     * Request TE at scanline.
     */
    void LCD_ReqTear(void)
    {
        static uint8_t ScanLineParams[2];

        uint16_t scanline = 533;
        ScanLineParams[0] = scanline >> 8;
        ScanLineParams[1] = scanline & 0x00FF;

        HAL_DSI_LongWrite(&hdsi_eval, 0, DSI_DCS_LONG_PKT_WRITE, 2, 0x44, ScanLineParams);
        // set_tear_on
        HAL_DSI_ShortWrite(&hdsi_eval, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x35, 0x00);
    }

    // Configure display to update specified region.

    void LCD_SetUpdateRegion(int idx)
    {
        HAL_DSI_LongWrite(&hdsi_eval, 0, DSI_DCS_LONG_PKT_WRITE, 4, OTM8009A_CMD_CASET, pCols[idx]);
    }

    void LCD_SetUpdateRegionLeft()
    {
        HAL_DSI_LongWrite(&hdsi_eval, 0, DSI_DCS_LONG_PKT_WRITE, 4, OTM8009A_CMD_CASET, pColLeft);
    }

    void LCD_SetUpdateRegionRight()
    {
        HAL_DSI_LongWrite(&hdsi_eval, 0, DSI_DCS_LONG_PKT_WRITE, 4, OTM8009A_CMD_CASET, pColRight);
    }
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow :
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 180000000
  *            HCLK(Hz)                       = 180000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 4
  *            APB2 Prescaler                 = 2
  *            HSE Frequency(Hz)              = 25000000
  *            PLL_M                          = 25
  *            PLL_N                          = 360
  *            PLL_P                          = 2
  *            PLL_Q                          = 7
  *            VDD(V)                         = 3.3
  *            Main regulator output voltage  = Scale1 mode
  *            Flash Latency(WS)              = 5
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void)
{
    RCC_ClkInitTypeDef RCC_ClkInitStruct;
    RCC_OscInitTypeDef RCC_OscInitStruct;
    HAL_StatusTypeDef ret = HAL_OK;

    /* Enable Power Control clock */
    __HAL_RCC_PWR_CLK_ENABLE();

    /* The voltage scaling allows optimizing the power consumption when the device is
       clocked below the maximum system frequency, to update the voltage scaling value
       regarding system frequency refer to product datasheet.  */
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /* Enable HSE Oscillator and activate PLL with HSE as source */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 25;
    RCC_OscInitStruct.PLL.PLLN = 360;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 7;
    RCC_OscInitStruct.PLL.PLLR = 6;


#if defined(USE_STM32469I_DISCO_REVA)
    RCC_OscInitStruct.PLL.PLLM = 25;
#else
    RCC_OscInitStruct.PLL.PLLM = 8;
#endif /* USE_STM32469I_DISCO_REVA */
    RCC_OscInitStruct.PLL.PLLN = 360;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 7;
    RCC_OscInitStruct.PLL.PLLR = 6;



    ret = HAL_RCC_OscConfig(&RCC_OscInitStruct);
    if (ret != HAL_OK)
    {
        while (1)
        {
            ;
        }
    }

    /* Activate the OverDrive to reach the 180 MHz Frequency */
    ret = HAL_PWREx_EnableOverDrive();
    if (ret != HAL_OK)
    {
        while (1)
        {
            ;
        }
    }

    /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 clocks dividers */
    RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

    ret = HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);
    if (ret != HAL_OK)
    {
        while (1)
        {
            ;
        }
    }
}

/**
  * @brief  Initializes the DSI LCD.
  * The ititialization is done as below:
  *     - DSI PLL ititialization
  *     - DSI ititialization
  *     - LTDC ititialization
  *     - OTM8009A LCD Display IC Driver ititialization
  * @param  None
  * @retval LCD state
  */
static uint8_t LCD_Init(void)
{

    GPIO_InitTypeDef GPIO_Init_Structure;

    /* Toggle Hardware Reset of the DSI LCD using
    * its XRES signal (active low) */
    BSP_LCD_Reset();

    /* Call first MSP Initialize only in case of first initialization
    * This will set IP blocks LTDC, DSI and DMA2D
    * - out of reset
    * - clocked
    * - NVIC IRQ related to IP blocks enabled
    */
    BSP_LCD_MspInit();

    /* LCD clock configuration */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LTDC;
#ifdef USE_DOUBLE_BUFFERING
    /* Double buffering use 384/7/2 = 27.4MHz */
    PeriphClkInitStruct.PLLSAI.PLLSAIN = 384;
    PeriphClkInitStruct.PLLSAI.PLLSAIR = 7;
#else
    /* Single buffering use 375/3/2 = 62.5MHz */
    PeriphClkInitStruct.PLLSAI.PLLSAIN = 375;
    PeriphClkInitStruct.PLLSAI.PLLSAIR = 3;
#endif
    PeriphClkInitStruct.PLLSAIDivR = RCC_PLLSAIDIVR_2;
    HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);

    /* Base address of DSI Host/Wrapper registers to be set before calling De-Init */
    hdsi_eval.Instance = DSI;

    HAL_DSI_DeInit(&(hdsi_eval));

    /* Disco board has 8Mhz crystal. */
    dsiPllInit.PLLNDIV  = 125;
    dsiPllInit.PLLIDF   = DSI_PLL_IN_DIV2;
    dsiPllInit.PLLODF  = DSI_PLL_OUT_DIV1;

    hdsi_eval.Init.NumberOfLanes = DSI_TWO_DATA_LANES;
    hdsi_eval.Init.TXEscapeCkdiv = 0x4;
    HAL_DSI_Init(&(hdsi_eval), &(dsiPllInit));

    /* Configure the DSI for Command mode */
    CmdCfg.VirtualChannelID      = 0;
    CmdCfg.HSPolarity            = DSI_HSYNC_ACTIVE_HIGH;
    CmdCfg.VSPolarity            = DSI_VSYNC_ACTIVE_HIGH;
    CmdCfg.DEPolarity            = DSI_DATA_ENABLE_ACTIVE_HIGH;
    CmdCfg.CommandSize           = HACT;
    CmdCfg.TearingEffectSource   = DSI_TE_EXTERNAL;
    CmdCfg.TearingEffectPolarity = DSI_TE_RISING_EDGE;
    CmdCfg.VSyncPol              = DSI_VSYNC_FALLING;
    CmdCfg.AutomaticRefresh      = DSI_AR_DISABLE;
    CmdCfg.TEAcknowledgeRequest  = DSI_TE_ACKNOWLEDGE_ENABLE;

#if !defined(USE_BPP) || USE_BPP==16
    CmdCfg.ColorCoding           = DSI_RGB565;
#elif USE_BPP==24
    CmdCfg.ColorCoding           = DSI_RGB888;
#else
#error Unknown USE_BPP
#endif

    HAL_DSI_ConfigAdaptedCommandMode(&hdsi_eval, &CmdCfg);

    LPCmd.LPGenShortWriteNoP    = DSI_LP_GSW0P_ENABLE;
    LPCmd.LPGenShortWriteOneP   = DSI_LP_GSW1P_ENABLE;
    LPCmd.LPGenShortWriteTwoP   = DSI_LP_GSW2P_ENABLE;
    LPCmd.LPGenShortReadNoP     = DSI_LP_GSR0P_ENABLE;
    LPCmd.LPGenShortReadOneP    = DSI_LP_GSR1P_ENABLE;
    LPCmd.LPGenShortReadTwoP    = DSI_LP_GSR2P_ENABLE;
    LPCmd.LPGenLongWrite        = DSI_LP_GLW_ENABLE;
    LPCmd.LPDcsShortWriteNoP    = DSI_LP_DSW0P_ENABLE;
    LPCmd.LPDcsShortWriteOneP   = DSI_LP_DSW1P_ENABLE;
    LPCmd.LPDcsShortReadNoP     = DSI_LP_DSR0P_ENABLE;
    LPCmd.LPDcsLongWrite        = DSI_LP_DLW_ENABLE;
    HAL_DSI_ConfigCommand(&hdsi_eval, &LPCmd);

    /* Initialize LTDC */
    LTDC_Init();

    /* Start DSI */
    HAL_DSI_Start(&(hdsi_eval));

    /* Initialize the OTM8009A LCD Display IC Driver (KoD LCD IC Driver)
    *  depending on configuration set in 'hdsivideo_handle'.
    */
#if !defined(USE_BPP) || USE_BPP==16
    OTM8009A_Init(OTM8009A_FORMAT_RBG565, LCD_ORIENTATION_LANDSCAPE);
#elif USE_BPP==24
    OTM8009A_Init(OTM8009A_FORMAT_RGB888, LCD_ORIENTATION_LANDSCAPE);
#else
#error Unknown USE_BPP
#endif

    LPCmd.LPGenShortWriteNoP    = DSI_LP_GSW0P_DISABLE;
    LPCmd.LPGenShortWriteOneP   = DSI_LP_GSW1P_DISABLE;
    LPCmd.LPGenShortWriteTwoP   = DSI_LP_GSW2P_DISABLE;
    LPCmd.LPGenShortReadNoP     = DSI_LP_GSR0P_DISABLE;
    LPCmd.LPGenShortReadOneP    = DSI_LP_GSR1P_DISABLE;
    LPCmd.LPGenShortReadTwoP    = DSI_LP_GSR2P_DISABLE;
    LPCmd.LPGenLongWrite        = DSI_LP_GLW_DISABLE;
    LPCmd.LPDcsShortWriteNoP    = DSI_LP_DSW0P_DISABLE;
    LPCmd.LPDcsShortWriteOneP   = DSI_LP_DSW1P_DISABLE;
    LPCmd.LPDcsShortReadNoP     = DSI_LP_DSR0P_DISABLE;
    LPCmd.LPDcsLongWrite        = DSI_LP_DLW_DISABLE;
    HAL_DSI_ConfigCommand(&hdsi_eval, &LPCmd);

    HAL_DSI_ConfigFlowControl(&hdsi_eval, DSI_FLOW_CONTROL_BTA);


    /* Enable GPIOJ clock */
    __HAL_RCC_GPIOJ_CLK_ENABLE();

    /* Configure DSI_TE pin from MB1166 : Tearing effect on separated GPIO from KoD LCD */
    /* that is mapped on GPIOJ2 as alternate DSI function (DSI_TE)                      */
    /* This pin is used only when the LCD and DSI link is configured in command mode    */
    /* Not used in DSI Video mode.                                                      */

    GPIO_Init_Structure.Pin       = GPIO_PIN_2;
    GPIO_Init_Structure.Mode      = GPIO_MODE_AF_PP;
    GPIO_Init_Structure.Pull      = GPIO_NOPULL;
    GPIO_Init_Structure.Speed     = GPIO_SPEED_HIGH;
    GPIO_Init_Structure.Alternate = GPIO_AF13_DSI;
    HAL_GPIO_Init(GPIOJ, &GPIO_Init_Structure);

    /* Refresh the display */
    HAL_DSI_Refresh(&hdsi_eval);

    return LCD_OK;
}

/**
  * @brief  Initialize the LTDC
  * @param  None
  * @retval None
  */
static void LTDC_Init(void)
{
    /* DeInit */
    HAL_LTDC_DeInit(&hltdc_eval);

    /* LTDC Config */
    /* Timing and polarity */
    hltdc_eval.Init.HorizontalSync = HSYNC;
    hltdc_eval.Init.VerticalSync = VSYNC;
    hltdc_eval.Init.AccumulatedHBP = HSYNC + HBP;
    hltdc_eval.Init.AccumulatedVBP = VSYNC + VBP;
    hltdc_eval.Init.AccumulatedActiveH = VSYNC + VBP + VACT;
    hltdc_eval.Init.AccumulatedActiveW = HSYNC + HBP + HACT;
    hltdc_eval.Init.TotalHeigh = VSYNC + VBP + VACT + VFP;
    hltdc_eval.Init.TotalWidth = HSYNC + HBP + HACT + HFP;

    /* background value */
    hltdc_eval.Init.Backcolor.Blue = 0;
    hltdc_eval.Init.Backcolor.Green = 0;
    hltdc_eval.Init.Backcolor.Red = 0;

    /* Polarity */
    hltdc_eval.Init.HSPolarity = LTDC_HSPOLARITY_AL;
    hltdc_eval.Init.VSPolarity = LTDC_VSPOLARITY_AL;
    hltdc_eval.Init.DEPolarity = LTDC_DEPOLARITY_AL;
    hltdc_eval.Init.PCPolarity = LTDC_PCPOLARITY_IPC;
    hltdc_eval.Instance = LTDC;

    HAL_LTDC_Init(&hltdc_eval);
}

/**
  * @brief  Initializes the LCD layers.
  * @param  LayerIndex: Layer foreground or background
  * @param  FB_Address: Layer frame buffer
  * @retval None
  */
static void LCD_LayerInit(uint16_t LayerIndex, uint32_t Address)
{
    LCD_LayerCfgTypeDef  Layercfg;

    /* Layer Init */
    Layercfg.WindowX0 = 0;
    Layercfg.WindowY0 = 0;
    Layercfg.WindowY1 = BSP_LCD_GetYSize();
    Layercfg.FBStartAdress = Address;
    Layercfg.Alpha = 255;
    Layercfg.Alpha0 = 0;
    Layercfg.Backcolor.Blue = 0;
    Layercfg.Backcolor.Green = 0;
    Layercfg.Backcolor.Red = 0;
    Layercfg.ImageHeight = BSP_LCD_GetYSize();

#if !defined(USE_BPP) || USE_BPP==16
    Layercfg.WindowX1 = BSP_LCD_GetXSize() / 4; //Note: Div4 due to screen being divided into 4 areas.
    Layercfg.PixelFormat = LTDC_PIXEL_FORMAT_RGB565;
    Layercfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_CA;
    Layercfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_CA;
    Layercfg.ImageWidth = BSP_LCD_GetXSize() / 4; //Note: Div4 due to screen being divided into 4 areas.
#elif USE_BPP==24
    Layercfg.WindowX1 = BSP_LCD_GetXSize() / 2; //Note: Div2 due to screen being divided into 2 areas.
    Layercfg.PixelFormat = LTDC_PIXEL_FORMAT_RGB888;
    Layercfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_PAxCA;
    Layercfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_PAxCA;
    Layercfg.ImageWidth = BSP_LCD_GetXSize() / 2; //Note: Div2 due to screen being divided into 2 areas.
#else
#error Unknown USE_BPP
#endif

    HAL_LTDC_ConfigLayer(&hltdc_eval, &Layercfg, LayerIndex);
}
