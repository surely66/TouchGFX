/**********************************************************************
* $Id$          lpc177x_8x_i2c.c                        2011-06-02
*//**
* @file         lpc177x_8x_i2c.c
* @brief        Contains all functions support for I2C firmware library
*                       on LPC177x_8x
* @version      1.0
* @date         02. June. 2011
* @author       NXP MCU SW Application Team
* 
* Copyright(C) 2011, NXP Semiconductor
* All rights reserved.
*
***********************************************************************
* Software that is described herein is for illustrative purposes only
* which provides customers with programming information regarding the
* products. This software is supplied "AS IS" without any warranties.
* NXP Semiconductors assumes no responsibility or liability for the
* use of the software, conveys no license or title under any patent,
* copyright, or mask work right to the product. NXP Semiconductors
* reserves the right to make changes in the software without
* notification. NXP Semiconductors also make no representation or
* warranty that such application will be suitable for the specified
* use without further testing or modification.
**********************************************************************/

/* Peripheral group ----------------------------------------------------------- */
/** @addtogroup I2C
 * @{
 */

/* Includes ------------------------------------------------------------------- */
#include "lpc177x_8x_i2c.h"
#include "lpc177x_8x_clkpwr.h"
#include "lpc177x_8x_pinsel.h"

/* Private Types -------------------------------------------------------------- */
/** @defgroup I2C_Private_Types I2C Private Types
 * @{
 */

/**
 * @brief I2C device configuration structure type
 */
typedef struct
{
  uint32_t      txrx_setup;                                             /* Transmission setup */
  int32_t               dir;                                                            /* Current direction phase, 0 - write, 1 - read */
} I2C_CFG_T;

/**
 * @}
 */

/* Private Variables ---------------------------------------------------------- */
/**
 * @brief II2C driver data for I2C0, I2C1 and I2C2
 */
static I2C_CFG_T i2cdat[3];

static uint32_t I2C_MasterComplete[3];
static uint32_t I2C_SlaveComplete[3];

static uint32_t I2C_MonitorBufferIndex;

/* Private Functions ---------------------------------------------------------- */

/* Get pointer to expected I2C */
static LPC_I2C_TypeDef* I2C_GetPointer(en_I2C_unitId compId);

/* Get I2C number */
static int32_t I2C_getNum(LPC_I2C_TypeDef *I2Cx);

/* Generate a start condition on I2C bus (in master mode only) */
static uint32_t I2C_Start (LPC_I2C_TypeDef *I2Cx);

/* Generate a stop condition on I2C bus (in master mode only) */
static void I2C_Stop (LPC_I2C_TypeDef *I2Cx);

/* I2C send byte subroutine */
static uint32_t I2C_SendByte (LPC_I2C_TypeDef *I2Cx, uint8_t databyte);

/* I2C get byte subroutine */
static uint32_t I2C_GetByte (LPC_I2C_TypeDef *I2Cx, uint8_t *retdat, Bool ack);

/* I2C set clock (hz) */
static void I2C_SetClock (LPC_I2C_TypeDef *I2Cx, uint32_t target_clock);

/*--------------------------------------------------------------------------------*/

/********************************************************************//**
 * @brief               Convert from I2C peripheral to number
 * @param[in]   I2Cx: I2C peripheral selected, should be:
 *                              - LPC_I2C0
 *                              - LPC_I2C1
 *                              - LPC_I2C2
 * @return              I2C number, could be: 0..2
 *********************************************************************/
static LPC_I2C_TypeDef* I2C_GetPointer(en_I2C_unitId compId)
{
        LPC_I2C_TypeDef* pI2C;

        switch (compId)
        {
                case I2C_0:
                        pI2C = LPC_I2C0;
                        break;

                case I2C_1:
                        pI2C = LPC_I2C1;
                        break;

                case I2C_2:
                        pI2C = LPC_I2C2;
                        break;

                default:
                        pI2C = NULL;
                        break;
        }

        return pI2C;
}


/********************************************************************//**
 * @brief               Convert from I2C peripheral to number
 * @param[in]   I2Cx: I2C peripheral selected, should be:
 *                              - LPC_I2C0
 *                              - LPC_I2C1
 *                              - LPC_I2C2
 * @return              I2C number, could be: 0..2
 *********************************************************************/
static int32_t I2C_getNum(LPC_I2C_TypeDef *I2Cx)
{
        if (I2Cx == LPC_I2C0)
        {
                return (0);
        }
        else if (I2Cx == LPC_I2C1)
        {
                return (1);
        }
        else if (I2Cx == LPC_I2C2)
        {
                return (2);
        }

        return (-1);
}

/********************************************************************//**
 * @brief               Generate a start condition on I2C bus (in master mode only)
 * @param[in]   I2Cx: I2C peripheral selected, should be:
 *                              - LPC_I2C0
 *                              - LPC_I2C1
 *                              - LPC_I2C2
 * @return              value of I2C status register after generate a start condition
 *********************************************************************/
static uint32_t I2C_Start (LPC_I2C_TypeDef *I2Cx)
{
        I2Cx->CONCLR = I2C_I2CONCLR_SIC;

        I2Cx->CONSET = I2C_I2CONSET_STA;

        // Wait for complete
        while (!(I2Cx->CONSET & I2C_I2CONSET_SI));

        I2Cx->CONCLR = I2C_I2CONCLR_STAC;

        return (I2Cx->STAT & I2C_STAT_CODE_BITMASK);
}

/********************************************************************//**
 * @brief               Generate a stop condition on I2C bus (in master mode only)
 * @param[in]   I2Cx: I2C peripheral selected, should be:
 *                              - LPC_I2C0
 *                              - LPC_I2C1
 *                              - LPC_I2C2
 * @return              None
 *********************************************************************/
static void I2C_Stop (LPC_I2C_TypeDef *I2Cx)
{
        /* Make sure start bit is not active */
        if (I2Cx->CONSET & I2C_I2CONSET_STA)
        {
                I2Cx->CONCLR = I2C_I2CONCLR_STAC;
        }

        I2Cx->CONSET = I2C_I2CONSET_STO;

        I2Cx->CONCLR = I2C_I2CONCLR_SIC;
}

/********************************************************************//**
 * @brief               Send a byte
 * @param[in]   I2Cx: I2C peripheral selected, should be:
 *                              - LPC_I2C0
 *                              - LPC_I2C1
 *                              - LPC_I2C2
 * @param[in]   databyte: number of byte
 * @return              value of I2C status register after sending
 *********************************************************************/
static uint32_t I2C_SendByte (LPC_I2C_TypeDef *I2Cx, uint8_t databyte)
{
        /* Make sure start bit is not active */
        if (I2Cx->CONSET & I2C_I2CONSET_STA)
        {
                I2Cx->CONCLR = I2C_I2CONCLR_STAC;
        }

        I2Cx->DAT = databyte & I2C_I2DAT_BITMASK;

        I2Cx->CONCLR = I2C_I2CONCLR_SIC;

        while (!(I2Cx->CONSET & I2C_I2CONSET_SI));

        return (I2Cx->STAT & I2C_STAT_CODE_BITMASK);
}

/********************************************************************//**
 * @brief               Get a byte
 * @param[in]   I2Cx: I2C peripheral selected, should be:
 *                              - LPC_I2C0
 *                              - LPC_I2C1
 *                              - LPC_I2C2
 * @param[out]  retdat  pointer to return data
 * @param[in]   ack             assert acknowledge or not, should be: TRUE/FALSE
 * @return              value of I2C status register after sending
 *********************************************************************/
static uint32_t I2C_GetByte (LPC_I2C_TypeDef *I2Cx, uint8_t *retdat, Bool ack)
{
        if (ack == TRUE)
        {
                I2Cx->CONSET = I2C_I2CONSET_AA;
        }
        else
        {
                I2Cx->CONCLR = I2C_I2CONCLR_AAC;
        }

        I2Cx->CONCLR = I2C_I2CONCLR_SIC;

        while (!(I2Cx->CONSET & I2C_I2CONSET_SI));

        *retdat = (uint8_t) (I2Cx->DAT & I2C_I2DAT_BITMASK);

        return (I2Cx->STAT & I2C_STAT_CODE_BITMASK);
}

/*********************************************************************//**
 * @brief               Setup clock rate for I2C peripheral
 * @param[in]   I2Cx    I2C peripheral selected, should be:
 *                              - LPC_I2C0
 *                              - LPC_I2C1
 *                              - LPC_I2C2
 * @param[in]   target_clock : clock of SSP (Hz)
 * @return              None
 ***********************************************************************/
static void I2C_SetClock (LPC_I2C_TypeDef *I2Cx, uint32_t target_clock)
{
        uint32_t temp;

        temp = CLKPWR_GetCLK(CLKPWR_CLKTYPE_PER) / target_clock;

        /* Set the I2C clock value to register */
        I2Cx->SCLH = (uint32_t)(temp / 2);

        I2Cx->SCLL = (uint32_t)(temp - I2Cx->SCLH);
}


/* End of Private Functions --------------------------------------------------- */


/* Public Functions ----------------------------------------------------------- */
/** @addtogroup I2C_Public_Functions
 * @{
 */

/********************************************************************//**
 * @brief               Initializes the I2Cx peripheral with specified parameter.
 * @param[in]   I2Cx    I2C peripheral selected, should be
 *                              - LPC_I2C0
 *                              - LPC_I2C1
 *                              - LPC_I2C2
 * @param[in]   clockrate Target clock rate value to initialized I2C
 *                              peripheral (Hz)
 * @return              None
 *********************************************************************/
void I2C_Init(uint8_t i2cId, uint32_t clockrate)
{
        uint32_t clkSetting, clkRate;

        LPC_I2C_TypeDef* I2Cx = I2C_GetPointer(i2cId);

        switch (i2cId)
        {
                case I2C_0:
                        clkSetting = CLKPWR_PCONP_PCI2C0;
                        break;


                case I2C_1:
                        clkSetting = CLKPWR_PCONP_PCI2C1;
                        break;

                case I2C_2:
                        clkSetting = CLKPWR_PCONP_PCI2C2;
                        break;

                default:

                        return;
        }

        CLKPWR_ConfigPPWR (clkSetting, ENABLE);

        /* Set clock rate */
        clkRate= CLKPWR_GetCLK(CLKPWR_CLKTYPE_PER) / clockrate;

        /* Set the I2C clock value to register */
        I2Cx->SCLH = (uint32_t)(clkRate / 2);

        I2Cx->SCLL = (uint32_t)(clkRate - I2Cx->SCLH);

    /* Set I2C operation to default */
    I2Cx->CONCLR = (I2C_I2CONCLR_AAC | I2C_I2CONCLR_STAC | I2C_I2CONCLR_I2ENC);
}

/*********************************************************************//**
 * @brief               De-initializes the I2C peripheral registers to their
 *                  default reset values.
 * @param[in]   I2Cx    I2C peripheral selected, should be
 *                              - LPC_I2C0
 *                              - LPC_I2C1
 *                              - LPC_I2C2
 * @return              None
 **********************************************************************/
void I2C_DeInit(uint8_t i2cId)
{
        uint32_t clkSetting;

        LPC_I2C_TypeDef* I2Cx = I2C_GetPointer(i2cId);

        /* Disable I2C control */
        I2Cx->CONCLR = I2C_I2CONCLR_I2ENC;

        switch (i2cId)
        {
                case I2C_0:
                        clkSetting = CLKPWR_PCONP_PCI2C0;
                        break;


                case I2C_1:
                        clkSetting = CLKPWR_PCONP_PCI2C1;
                        break;

                case I2C_2:
                        clkSetting = CLKPWR_PCONP_PCI2C2;
                        break;

                default:

                        return;
        }

        CLKPWR_ConfigPPWR (clkSetting, DISABLE);
}

/*********************************************************************//**
 * @brief               Enable or disable I2C peripheral's operation
 * @param[in]   I2Cx I2C peripheral selected, should be
 *                              - LPC_I2C0
 *                              - LPC_I2C1
 *                              - LPC_I2C2
 * @param[in]   NewState New State of I2Cx peripheral's operation
 * @return              none
 **********************************************************************/
void I2C_Cmd(uint8_t i2cId, FunctionalState NewState)
{
        LPC_I2C_TypeDef* I2Cx = I2C_GetPointer(i2cId);

        if (NewState == ENABLE)
        {
                I2Cx->CONSET = I2C_I2CONSET_I2EN;
        }
        else
        {
                I2Cx->CONCLR = I2C_I2CONCLR_I2ENC;
        }
}

/*********************************************************************//**
 * @brief               Enable/Disable interrupt for I2C peripheral
 * @param[in]   I2Cx    I2C peripheral selected, should be:
 *                              - LPC_I2C0
 *                              - LPC_I2C1
 *                              - LPC_I2C2
 * @param[in]   NewState        New State of I2C peripheral interrupt in NVIC core
 *                              should be:
 *                              - ENABLE: enable interrupt for this I2C peripheral
 *                              - DISABLE: disable interrupt for this I2C peripheral
 * @return              None
 **********************************************************************/
void I2C_IntCmd (uint8_t i2cId, Bool NewState)
{
        IRQn_Type irq;

        switch (i2cId)
        {
                case I2C_0:
                        irq = I2C0_IRQn;
                        break;

                case I2C_1:
                        irq = I2C1_IRQn;
                        break;

                case I2C_2:
                        irq = I2C2_IRQn;
                        break;

                default:

                        return;
        }

        if (NewState)
        {
                NVIC_EnableIRQ(irq);
        }
        else
        {
                NVIC_DisableIRQ(irq);
        }

    return;
}


/*********************************************************************//**
 * @brief               General Master Interrupt handler for I2C peripheral
 * @param[in]   I2Cx    I2C peripheral selected, should be:
 *                              - LPC_I2C
 *                              - LPC_I2C1
 *                              - LPC_I2C2
 * @return              None
 **********************************************************************/
void I2C_MasterHandler(uint8_t i2cId)
{
        LPC_I2C_TypeDef* I2Cx = I2C_GetPointer(i2cId);

        uint8_t returnCode;
        I2C_M_SETUP_Type *txrx_setup;

        txrx_setup = (I2C_M_SETUP_Type *) i2cdat[i2cId].txrx_setup;

        returnCode = (I2Cx->STAT & I2C_STAT_CODE_BITMASK);

        // Save current status
        txrx_setup->status = returnCode;

        // there's no relevant information
        if (returnCode == I2C_I2STAT_NO_INF)
        {
                I2Cx->CONCLR = I2C_I2CONCLR_SIC;
                return;
        }

        /* ----------------------------- TRANSMIT PHASE --------------------------*/
        if (i2cdat[i2cId].dir == 0)
        {
                switch (returnCode)
                {
                        /* A start/repeat start condition has been transmitted -------------------*/
                        case I2C_I2STAT_M_TX_START:

                        case I2C_I2STAT_M_TX_RESTART:
                                I2Cx->CONCLR = I2C_I2CONCLR_STAC;
                                /*
                                 * If there's any transmit data, then start to
                                 * send SLA+W right now, otherwise check whether if there's
                                 * any receive data for next state.
                                 */
                                if ((txrx_setup->tx_data != NULL) && (txrx_setup->tx_length != 0))
                                {
                                        I2Cx->DAT = (txrx_setup->sl_addr7bit << 1);
                                        I2Cx->CONCLR = I2C_I2CONCLR_SIC;
                                }
                                else
                                {
                                        goto next_stage;
                                }

                                break;

                        /* SLA+W has been transmitted, ACK has been received ----------------------*/
                        case I2C_I2STAT_M_TX_SLAW_ACK:

                        /* Data has been transmitted, ACK has been received */
                        case I2C_I2STAT_M_TX_DAT_ACK:
                                /* Send more data */
                                if ((txrx_setup->tx_count < txrx_setup->tx_length) && (txrx_setup->tx_data != NULL))
                                {
                                        I2Cx->DAT =  *(uint8_t *)(txrx_setup->tx_data + txrx_setup->tx_count);

                                        txrx_setup->tx_count++;

                                        I2Cx->CONCLR = I2C_I2CONCLR_SIC;
                                }
                                // no more data, switch to next stage
                                else
                                {
        next_stage:
                                        // change direction
                                        i2cdat[i2cId].dir = 1;

                                        // Check if any data to receive
                                        if ((txrx_setup->rx_length != 0) && (txrx_setup->rx_data != NULL))
                                        {
                                                        // check whether if we need to issue an repeat start
                                                        if ((txrx_setup->tx_length != 0) && (txrx_setup->tx_data != NULL))
                                                        {
                                                                // Send out an repeat start command
                                                                I2Cx->CONSET = I2C_I2CONSET_STA;

                                                                I2Cx->CONCLR = I2C_I2CONCLR_AAC | I2C_I2CONCLR_SIC;
                                                        }
                                                        // Don't need issue an repeat start, just goto send SLA+R
                                                        else
                                                        {
                                                                goto send_slar;
                                                        }
                                        }
                                        // no more data send, the go to end stage now
                                        else
                                        {
                                                // success, goto end stage
                                                txrx_setup->status |= I2C_SETUP_STATUS_DONE;

                                                goto end_stage;
                                        }
                                }
                                break;

                        /* SLA+W has been transmitted, NACK has been received ----------------------*/
                        case I2C_I2STAT_M_TX_SLAW_NACK:

                        /* Data has been transmitted, NACK has been received -----------------------*/
                        case I2C_I2STAT_M_TX_DAT_NACK:
                                // update status
                                txrx_setup->status |= I2C_SETUP_STATUS_NOACKF;
                                goto retry;

                        /* Arbitration lost in SLA+R/W or Data bytes -------------------------------*/
                        case I2C_I2STAT_M_TX_ARB_LOST:
                                // update status
                                txrx_setup->status |= I2C_SETUP_STATUS_ARBF;

                        default:
                                goto retry;
                }
        }

        /* ----------------------------- RECEIVE PHASE --------------------------*/
        else if (i2cdat[i2cId].dir == 1)
        {
                switch (returnCode)
                {
                                /* A start/repeat start condition has been transmitted ---------------------*/
                        case I2C_I2STAT_M_RX_START:

                        case I2C_I2STAT_M_RX_RESTART:
                                I2Cx->CONCLR = I2C_I2CONCLR_STAC;
                                /*
                                 * If there's any receive data, then start to
                                 * send SLA+R right now, otherwise check whether if there's
                                 * any receive data for end of state.
                                 */
                                if ((txrx_setup->rx_data != NULL) && (txrx_setup->rx_length != 0))
                                {
        send_slar:
                                        I2Cx->DAT = (txrx_setup->sl_addr7bit << 1) | 0x01;
                                        I2Cx->CONCLR = I2C_I2CONCLR_SIC;
                                }
                                else
                                {
                                        // Success, goto end stage
                                        txrx_setup->status |= I2C_SETUP_STATUS_DONE;
                                        goto end_stage;
                                }

                                break;

                        /* SLA+R has been transmitted, ACK has been received -----------------*/
                        case I2C_I2STAT_M_RX_SLAR_ACK:
                                if (txrx_setup->rx_count < (txrx_setup->rx_length - 1))
                                {
                                        /*Data will be received,  ACK will be return*/
                                        I2Cx->CONSET = I2C_I2CONSET_AA;
                                }
                                else
                                {
                                        /*Last data will be received,  NACK will be return*/
                                        I2Cx->CONCLR = I2C_I2CONSET_AA;
                                }

                                I2Cx->CONCLR = I2C_I2CONCLR_SIC;
                                break;

                        /* Data has been received, ACK has been returned ----------------------*/
                        case I2C_I2STAT_M_RX_DAT_ACK:
                                // Note save data and increase counter first, then check later
                                /* Save data  */
                                if ((txrx_setup->rx_data != NULL) && (txrx_setup->rx_count < txrx_setup->rx_length))
                                {
                                        *(uint8_t *)(txrx_setup->rx_data + txrx_setup->rx_count) = (I2Cx->DAT & I2C_I2DAT_BITMASK);

                                        txrx_setup->rx_count++;
                                }
                                if (txrx_setup->rx_count < (txrx_setup->rx_length - 1))
                                {
                                        /*Data will be received,  ACK will be return*/
                                        I2Cx->CONSET = I2C_I2CONSET_AA;
                                }
                                else
                                {
                                        /*Last data will be received,  NACK will be return*/
                                        I2Cx->CONCLR = I2C_I2CONSET_AA;
                                }

                                I2Cx->CONCLR = I2C_I2CONCLR_SIC;
                                break;

                        /* Data has been received, NACK has been return -------------------------*/
                        case I2C_I2STAT_M_RX_DAT_NACK:
                                /* Save the last data */
                                if ((txrx_setup->rx_data != NULL) && (txrx_setup->rx_count < txrx_setup->rx_length))
                                {
                                        *(uint8_t *)(txrx_setup->rx_data + txrx_setup->rx_count) = (I2Cx->DAT & I2C_I2DAT_BITMASK);

                                        txrx_setup->rx_count++;
                                }
                                // success, go to end stage
                                txrx_setup->status |= I2C_SETUP_STATUS_DONE;

                                goto end_stage;

                        /* SLA+R has been transmitted, NACK has been received ------------------*/
                        case I2C_I2STAT_M_RX_SLAR_NACK:
                                // update status
                                txrx_setup->status |= I2C_SETUP_STATUS_NOACKF;
                                goto retry;

                        /* Arbitration lost ----------------------------------------------------*/
                        case I2C_I2STAT_M_RX_ARB_LOST:
                                // update status
                                txrx_setup->status |= I2C_SETUP_STATUS_ARBF;

                        default:
        retry:
                                // check if retransmission is available
                                if (txrx_setup->retransmissions_count < txrx_setup->retransmissions_max)
                                {
                                        // Clear tx count
                                        txrx_setup->tx_count = 0;

                                        I2Cx->CONSET = I2C_I2CONSET_STA;

                                        I2Cx->CONCLR = I2C_I2CONCLR_AAC | I2C_I2CONCLR_SIC;

                                        txrx_setup->retransmissions_count++;
                                }
                                // End of stage
                                else
                                {
        end_stage:
                                        // Disable interrupt
                                        I2C_IntCmd(i2cId, 0);

                                        // Send stop
                                        I2C_Stop(I2Cx);

                                        I2C_MasterComplete[i2cId] = TRUE;
                                }
                                break;
                }
        }
}


/*********************************************************************//**
 * @brief               General Slave Interrupt handler for I2C peripheral
 * @param[in]   I2Cx    I2C peripheral selected, should be:
 *                      - LPC_I2C0
 *                      - LPC_I2C1
 *                      - LPC_I2C2
 * @return              None
 **********************************************************************/
void I2C_SlaveHandler (uint8_t i2cId)
{
        LPC_I2C_TypeDef* I2Cx = I2C_GetPointer(i2cId);

        uint8_t returnCode;
        I2C_S_SETUP_Type *txrx_setup;
        uint32_t timeout;

        txrx_setup = (I2C_S_SETUP_Type *) i2cdat[i2cId].txrx_setup;

        returnCode = (I2Cx->STAT & I2C_STAT_CODE_BITMASK);
        // Save current status
        txrx_setup->status = returnCode;
        // there's no relevant information
        if (returnCode == I2C_I2STAT_NO_INF)
        {
                I2Cx->CONCLR = I2C_I2CONCLR_SIC;

                return;
        }


        switch (returnCode)
        {

                /* No status information */
                case I2C_I2STAT_NO_INF:
                        I2Cx->CONSET = I2C_I2CONSET_AA;
                        I2Cx->CONCLR = I2C_I2CONCLR_SIC;
                        break;

                /* Reading phase -------------------------------------------------------- */
                /* Own SLA+R has been received, ACK has been returned */
                case I2C_I2STAT_S_RX_SLAW_ACK:

                /* General call address has been received, ACK has been returned */
                case I2C_I2STAT_S_RX_GENCALL_ACK:
                        I2Cx->CONSET = I2C_I2CONSET_AA;
                        I2Cx->CONCLR = I2C_I2CONCLR_SIC;
                        break;

                /* Previously addressed with own SLA;
                 * DATA byte has been received;
                 * ACK has been returned */
                case I2C_I2STAT_S_RX_PRE_SLA_DAT_ACK:

                /* DATA has been received, ACK hasn been return */
                case I2C_I2STAT_S_RX_PRE_GENCALL_DAT_ACK:
                        /*
                         * All data bytes that over-flow the specified receive
                         * data length, just ignore them.
                         */
                        if ((txrx_setup->rx_count < txrx_setup->rx_length) && (txrx_setup->rx_data != NULL))
                        {
                                *(uint8_t *)(txrx_setup->rx_data + txrx_setup->rx_count) = (uint8_t)I2Cx->DAT;

                                txrx_setup->rx_count++;
                        }

                        I2Cx->CONSET = I2C_I2CONSET_AA;
                        I2Cx->CONCLR = I2C_I2CONCLR_SIC;

                        break;

                /* Previously addressed with own SLA;
                 * DATA byte has been received;
                 * NOT ACK has been returned */
                case I2C_I2STAT_S_RX_PRE_SLA_DAT_NACK:

                /* DATA has been received, NOT ACK has been returned */
                case I2C_I2STAT_S_RX_PRE_GENCALL_DAT_NACK:
                        I2Cx->CONCLR = I2C_I2CONCLR_SIC;
                        break;

                /*
                 * Note that: Return code only let us know a stop condition mixed
                 * with a repeat start condition in the same code value.
                 * So we should provide a time-out. In case this is really a stop
                 * condition, this will return back after time out condition. Otherwise,
                 * next session that is slave receive data will be completed.
                 */

                /* A Stop or a repeat start condition */
                case I2C_I2STAT_S_RX_STA_STO_SLVREC_SLVTRX:
                        // Temporally lock the interrupt for timeout condition
                        I2C_IntCmd(i2cId, 0);

                        I2Cx->CONCLR = I2C_I2CONCLR_SIC;
                        // enable time out
                        timeout = I2C_SLAVE_TIME_OUT;
                        while(1)
                        {
                                if (I2Cx->CONSET & I2C_I2CONSET_SI)
                                {
                                        // re-Enable interrupt
                                        I2C_IntCmd(i2cId, 1);

                                        break;
                                }
                                else
                                {
                                        timeout--;

                                        if (timeout == 0)
                                        {
                                                // timeout occur, it's really a stop condition
                                                txrx_setup->status |= I2C_SETUP_STATUS_DONE;

                                                goto s_int_end;
                                        }
                                }
                        }
                        break;

                /* Writing phase -------------------------------------------------------- */
                /* Own SLA+R has been received, ACK has been returned */
                case I2C_I2STAT_S_TX_SLAR_ACK:

                /* Data has been transmitted, ACK has been received */
                case I2C_I2STAT_S_TX_DAT_ACK:
                        /*
                         * All data bytes that over-flow the specified receive
                         * data length, just ignore them.
                         */
                        if ((txrx_setup->tx_count < txrx_setup->tx_length) && (txrx_setup->tx_data != NULL))
                        {
                                I2Cx->DAT = *(uint8_t *) (txrx_setup->tx_data + txrx_setup->tx_count);

                                txrx_setup->tx_count++;
                        }

                        I2Cx->CONSET = I2C_I2CONSET_AA;
                        I2Cx->CONCLR = I2C_I2CONCLR_SIC;

                        break;

                /* Data has been transmitted, NACK has been received,
                 * that means there's no more data to send, exit now */
                /*
                 * Note: Don't wait for stop event since in slave transmit mode,
                 * since there no proof lets us know when a stop signal has been received
                 * on slave side.
                 */
                case I2C_I2STAT_S_TX_DAT_NACK:
                        I2Cx->CONSET = I2C_I2CONSET_AA;
                        I2Cx->CONCLR = I2C_I2CONCLR_SIC;
                        txrx_setup->status |= I2C_SETUP_STATUS_DONE;
                        goto s_int_end;

                // Other status must be captured
                default:
        s_int_end:
                        // Disable interrupt
                        I2C_IntCmd(i2cId, 0);

                        I2Cx->CONCLR = I2C_I2CONCLR_AAC | I2C_I2CONCLR_SIC | I2C_I2CONCLR_STAC;

                        I2C_SlaveComplete[i2cId] = TRUE;
                        break;
        }
}

/*********************************************************************//**
 * @brief               Transmit and Receive data in master mode
 * @param[in]   I2Cx                    I2C peripheral selected, should be:
 *                              - LPC_I2C0
 *                              - LPC_I2C1
 *                              - LPC_I2C2
 * @param[in]   TransferCfg             Pointer to a I2C_M_SETUP_Type structure that
 *                                                              contains specified information about the
 *                                                              configuration for master transfer.
 * @param[in]   Opt                             a I2C_TRANSFER_OPT_Type type that selected for
 *                                                              interrupt or polling mode.
 * @return              SUCCESS or ERROR
 *
 * Note:
 * - In case of using I2C to transmit data only, either transmit length set to 0
 * or transmit data pointer set to NULL.
 * - In case of using I2C to receive data only, either receive length set to 0
 * or receive data pointer set to NULL.
 * - In case of using I2C to transmit followed by receive data, transmit length,
 * transmit data pointer, receive length and receive data pointer should be set
 * corresponding.
 **********************************************************************/
Status I2C_MasterTransferData(uint8_t i2cId, I2C_M_SETUP_Type *TransferCfg,
                                                                                                                                        I2C_TRANSFER_OPT_Type Opt)
{
        LPC_I2C_TypeDef* I2Cx = I2C_GetPointer(i2cId);

        uint8_t *txdat;
        uint8_t *rxdat;
        uint32_t CodeStatus;
        uint8_t tmp;

        // reset all default state
        txdat = (uint8_t *) TransferCfg->tx_data;
        rxdat = (uint8_t *) TransferCfg->rx_data;

        // Reset I2C setup value to default state
        TransferCfg->tx_count = 0;
        TransferCfg->rx_count = 0;
        TransferCfg->status = 0;

        if (Opt == I2C_TRANSFER_POLLING)
        {
                /* First Start condition -------------------------------------------------------------- */
                TransferCfg->retransmissions_count = 0;
retry:
                // reset all default state
                txdat = (uint8_t *) TransferCfg->tx_data;
                rxdat = (uint8_t *) TransferCfg->rx_data;

                // Reset I2C setup value to default state
                TransferCfg->tx_count = 0;
                TransferCfg->rx_count = 0;

                CodeStatus = 0;

                // Start command
                CodeStatus = I2C_Start(I2Cx);

                if ((CodeStatus != I2C_I2STAT_M_TX_START) && (CodeStatus != I2C_I2STAT_M_TX_RESTART))
                {
                        TransferCfg->retransmissions_count++;
                        if (TransferCfg->retransmissions_count > TransferCfg->retransmissions_max)
                        {
                                // save status
                                TransferCfg->status = CodeStatus;

                                goto error;
                        }
                        else
                        {
                                goto retry;
                        }
                }

                /* In case of sending data first --------------------------------------------------- */
                if ((TransferCfg->tx_length != 0) && (TransferCfg->tx_data != NULL))
                {
                        /* Send slave address + WR direction bit = 0 ----------------------------------- */
                        CodeStatus = I2C_SendByte(I2Cx, (TransferCfg->sl_addr7bit << 1));

                        if (CodeStatus != I2C_I2STAT_M_TX_SLAW_ACK)
                        {
                                TransferCfg->retransmissions_count++;

                                if (TransferCfg->retransmissions_count > TransferCfg->retransmissions_max)
                                {
                                        // save status
                                        TransferCfg->status = CodeStatus | I2C_SETUP_STATUS_NOACKF;

                                        goto error;
                                }
                                else
                                {
                                        goto retry;
                                }
                        }

                        /* Send a number of data bytes ---------------------------------------- */
                        while (TransferCfg->tx_count < TransferCfg->tx_length)
                        {
                                CodeStatus = I2C_SendByte(I2Cx, *txdat);

                                if (CodeStatus != I2C_I2STAT_M_TX_DAT_ACK)
                                {
                                        TransferCfg->retransmissions_count++;

                                        if (TransferCfg->retransmissions_count > TransferCfg->retransmissions_max)
                                        {
                                                // save status
                                                TransferCfg->status = CodeStatus | I2C_SETUP_STATUS_NOACKF;

                                                goto error;
                                        }
                                        else
                                        {
                                                goto retry;
                                        }
                                }

                                txdat++;

                                TransferCfg->tx_count++;
                        }
                }

                /* Second Start condition (Repeat Start) ------------------------------------------- */
                if ((TransferCfg->tx_length != 0) && (TransferCfg->tx_data != NULL) \
                                && (TransferCfg->rx_length != 0) && (TransferCfg->rx_data != NULL))
                {
                        CodeStatus = I2C_Start(I2Cx);

                        if ((CodeStatus != I2C_I2STAT_M_RX_START) \
                                        && (CodeStatus != I2C_I2STAT_M_RX_RESTART))
                        {
                                TransferCfg->retransmissions_count++;

                                if (TransferCfg->retransmissions_count > TransferCfg->retransmissions_max)
                                {
                                        // Update status
                                        TransferCfg->status = CodeStatus;
                                        goto error;
                                }
                                else
                                {
                                        goto retry;
                                }
                        }
                }

                /* Then, start reading after sending data -------------------------------------- */
                if ((TransferCfg->rx_length != 0) && (TransferCfg->rx_data != NULL))
                {
                        /* Send slave address + RD direction bit = 1 ----------------------------------- */

                        CodeStatus = I2C_SendByte(I2Cx, ((TransferCfg->sl_addr7bit << 1) | 0x01));

                        if (CodeStatus != I2C_I2STAT_M_RX_SLAR_ACK)
                        {
                                TransferCfg->retransmissions_count++;

                                if (TransferCfg->retransmissions_count > TransferCfg->retransmissions_max)
                                {
                                        // update status
                                        TransferCfg->status = CodeStatus | I2C_SETUP_STATUS_NOACKF;

                                        goto error;
                                }
                                else
                                {
                                        goto retry;
                                }
                        }

                        /* Receive a number of data bytes ------------------------------------------------- */
                        while (TransferCfg->rx_count < TransferCfg->rx_length)
                        {
                                /*
                                 * Note that: if data length is only one, the master should not
                                 * issue an ACK signal on bus after reading to avoid of next data frame
                                 * on slave side
                                 */
                                if (TransferCfg->rx_count < (TransferCfg->rx_length - 1))
                                {
                                        // Issue an ACK signal for next data frame
                                        CodeStatus = I2C_GetByte(I2Cx, &tmp, 1);

                                        if (CodeStatus != I2C_I2STAT_M_RX_DAT_ACK)
                                        {
                                                TransferCfg->retransmissions_count++;

                                                if (TransferCfg->retransmissions_count > TransferCfg->retransmissions_max)
                                                {
                                                        // update status
                                                        TransferCfg->status = CodeStatus;

                                                        goto error;
                                                }
                                                else
                                                {
                                                        goto retry;
                                                }
                                        }
                                }
                                else
                                {
                                        // Do not issue an ACK signal
                                        CodeStatus = I2C_GetByte(I2Cx, &tmp, 0);

                                        if (CodeStatus != I2C_I2STAT_M_RX_DAT_NACK)
                                        {
                                                TransferCfg->retransmissions_count++;

                                                if (TransferCfg->retransmissions_count > TransferCfg->retransmissions_max)
                                                {
                                                        // update status
                                                        TransferCfg->status = CodeStatus;

                                                        goto error;
                                                }
                                                else
                                                {
                                                        goto retry;
                                                }
                                        }
                                }

                                *rxdat++ = tmp;

                                TransferCfg->rx_count++;
                        }
                }

                /* Send STOP condition ------------------------------------------------- */
                I2C_Stop(I2Cx);

                return SUCCESS;

error:
                // Send stop condition
                I2C_Stop(I2Cx);

                return ERROR;
        }

        else if (Opt == I2C_TRANSFER_INTERRUPT)
        {
                // Setup tx_rx data, callback and interrupt handler
                i2cdat[i2cId].txrx_setup = (uint32_t) TransferCfg;

                // Set direction phase, write first
                i2cdat[i2cId].dir = 0;

                /* First Start condition -------------------------------------------------------------- */
                I2Cx->CONCLR = I2C_I2CONCLR_SIC;
                I2Cx->CONSET = I2C_I2CONSET_STA;

                I2C_IntCmd(i2cId, 1);

                return (SUCCESS);
        }

        return ERROR;
}

/*********************************************************************//**
 * @brief               Receive and Transmit data in slave mode
 * @param[in]   I2Cx                    I2C peripheral selected, should be
 *                              - LPC_I2C0
 *                              - LPC_I2C1
 *                              - LPC_I2C2
 * @param[in]   TransferCfg             Pointer to a I2C_S_SETUP_Type structure that
 *                                                              contains specified information about the
 *                                                              configuration for master transfer.
 * @param[in]   Opt                             I2C_TRANSFER_OPT_Type type that selected for
 *                                                              interrupt or polling mode.
 * @return              SUCCESS or ERROR
 *
 * Note:
 * The mode of slave's operation depends on the command sent from master on
 * the I2C bus. If the master send a SLA+W command, this sub-routine will
 * use receive data length and receive data pointer. If the master send a SLA+R
 * command, this sub-routine will use transmit data length and transmit data
 * pointer.
 * If the master issue an repeat start command or a stop command, the slave will
 * enable an time out condition, during time out condition, if there's no activity
 * on I2C bus, the slave will exit, otherwise (i.e. the master send a SLA+R/W),
 * the slave then switch to relevant operation mode. The time out should be used
 * because the return status code can not show difference from stop and repeat
 * start command in slave operation.
 * In case of the expected data length from master is greater than data length
 * that slave can support:
 * - In case of reading operation (from master): slave will return I2C_I2DAT_IDLE_CHAR
 * value.
 * - In case of writing operation (from master): slave will ignore remain data from master.
 **********************************************************************/
Status I2C_SlaveTransferData(uint8_t i2cId, I2C_S_SETUP_Type *TransferCfg,
                                                                                                                                        I2C_TRANSFER_OPT_Type Opt)
{
        LPC_I2C_TypeDef* I2Cx = I2C_GetPointer(i2cId);

        uint8_t *txdat;
        uint8_t *rxdat;
        uint32_t CodeStatus;
        uint32_t timeout;
        int32_t time_en;

        // reset all default state
        txdat = (uint8_t *) TransferCfg->tx_data;
        rxdat = (uint8_t *) TransferCfg->rx_data;
        // Reset I2C setup value to default state
        TransferCfg->tx_count = 0;
        TransferCfg->rx_count = 0;
        TransferCfg->status = 0;


        // Polling option
        if (Opt == I2C_TRANSFER_POLLING)
        {
                /* Set AA bit to ACK command on I2C bus */
                I2Cx->CONSET = I2C_I2CONSET_AA;

                /* Clear SI bit to be ready ... */
                I2Cx->CONCLR = (I2C_I2CONCLR_SIC | I2C_I2CONCLR_STAC);

                time_en = 0;
                timeout = 0;

                while (1)
                {
                        /* Check SI flag ready */
                        if (I2Cx->CONSET & I2C_I2CONSET_SI)
                        {
                                time_en = 0;

                                switch (CodeStatus = (I2Cx->STAT & I2C_STAT_CODE_BITMASK))
                                {
                                        /* No status information */
                                        case I2C_I2STAT_NO_INF:
                                                I2Cx->CONSET = I2C_I2CONSET_AA;
                                                I2Cx->CONCLR = I2C_I2CONCLR_SIC;
                                                break;

                                        /* Reading phase -------------------------------------------------------- */
                                        /* Own SLA+R has been received, ACK has been returned */
                                        case I2C_I2STAT_S_RX_SLAW_ACK:

                                        /* General call address has been received, ACK has been returned */
                                        case I2C_I2STAT_S_RX_GENCALL_ACK:
                                                I2Cx->CONSET = I2C_I2CONSET_AA;
                                                I2Cx->CONCLR = I2C_I2CONCLR_SIC;
                                                break;

                                        /* Previously addressed with own SLA;
                                         * DATA byte has been received;
                                         * ACK has been returned */
                                        case I2C_I2STAT_S_RX_PRE_SLA_DAT_ACK:

                                        /* DATA has been received, ACK hasn been return */
                                        case I2C_I2STAT_S_RX_PRE_GENCALL_DAT_ACK:
                                                /*
                                                 * All data bytes that over-flow the specified receive
                                                 * data length, just ignore them.
                                                 */
                                                if ((TransferCfg->rx_count < TransferCfg->rx_length) && (TransferCfg->rx_data != NULL))
                                                {
                                                        *rxdat++ = (uint8_t)I2Cx->DAT;

                                                        TransferCfg->rx_count++;
                                                }

                                                I2Cx->CONSET = I2C_I2CONSET_AA;
                                                I2Cx->CONCLR = I2C_I2CONCLR_SIC;
                                                break;

                                        /* Previously addressed with own SLA;
                                         * DATA byte has been received;
                                         * NOT ACK has been returned */
                                        case I2C_I2STAT_S_RX_PRE_SLA_DAT_NACK:

                                        /* DATA has been received, NOT ACK has been returned */
                                        case I2C_I2STAT_S_RX_PRE_GENCALL_DAT_NACK:
                                                I2Cx->CONCLR = I2C_I2CONCLR_SIC;
                                                break;

                                        /*
                                         * Note that: Return code only let us know a stop condition mixed
                                         * with a repeat start condition in the same code value.
                                         * So we should provide a time-out. In case this is really a stop
                                         * condition, this will return back after time out condition. Otherwise,
                                         * next session that is slave receive data will be completed.
                                         */

                                        /* A Stop or a repeat start condition */
                                        case I2C_I2STAT_S_RX_STA_STO_SLVREC_SLVTRX:
                                                I2Cx->CONCLR = I2C_I2CONCLR_SIC;
                                                // enable time out
                                                time_en = 1;
                                                timeout = 0;
                                                break;

                                        /* Writing phase -------------------------------------------------------- */
                                        /* Own SLA+R has been received, ACK has been returned */
                                        case I2C_I2STAT_S_TX_SLAR_ACK:

                                        /* Data has been transmitted, ACK has been received */
                                        case I2C_I2STAT_S_TX_DAT_ACK:
                                                /*
                                                 * All data bytes that over-flow the specified receive
                                                 * data length, just ignore them.
                                                 */
                                                if ((TransferCfg->tx_count < TransferCfg->tx_length) && (TransferCfg->tx_data != NULL))
                                                {
                                                        I2Cx->DAT = *txdat++;

                                                        TransferCfg->tx_count++;
                                                }

                                                I2Cx->CONSET = I2C_I2CONSET_AA;
                                                I2Cx->CONCLR = I2C_I2CONCLR_SIC;
                                                break;

                                        /* Data has been transmitted, NACK has been received,
                                         * that means there's no more data to send, exit now */
                                        /*
                                         * Note: Don't wait for stop event since in slave transmit mode,
                                         * since there no proof lets us know when a stop signal has been received
                                         * on slave side.
                                         */
                                        case I2C_I2STAT_S_TX_DAT_NACK:
                                                I2Cx->CONSET = I2C_I2CONSET_AA;
                                                I2Cx->CONCLR = I2C_I2CONCLR_SIC;
                                                // enable time out
                                                time_en = 1;
                                                timeout = 0;
                                                break;

                                        // Other status must be captured
                                        default:
                                                I2Cx->CONCLR = I2C_I2CONCLR_SIC;
                                                goto s_error;
                                }
                        }
                        else if (time_en)
                        {
                                if (timeout++ > I2C_SLAVE_TIME_OUT)
                                {
                                        // it's really a stop condition, goto end stage
                                        goto s_end_stage;
                                }
                        }
                }

s_end_stage:
                /* Clear AA bit to disable ACK on I2C bus */
                I2Cx->CONCLR = I2C_I2CONCLR_AAC;

                // Check if there's no error during operation
                // Update status
                TransferCfg->status = CodeStatus | I2C_SETUP_STATUS_DONE;
                return SUCCESS;

s_error:
                /* Clear AA bit to disable ACK on I2C bus */
                I2Cx->CONCLR = I2C_I2CONCLR_AAC;

                // Update status
                TransferCfg->status = CodeStatus;
                return ERROR;
        }

        else if (Opt == I2C_TRANSFER_INTERRUPT)
        {
                // Setup tx_rx data, callback and interrupt handler
                i2cdat[i2cId].txrx_setup = (uint32_t) TransferCfg;

                // Set direction phase, read first
                i2cdat[i2cId].dir = 1;

                // Enable AA
                I2Cx->CONSET = I2C_I2CONSET_AA;
                I2Cx->CONCLR = I2C_I2CONCLR_SIC | I2C_I2CONCLR_STAC;
                I2C_IntCmd(i2cId, 1);

                return (SUCCESS);
        }

        return ERROR;
}

/*********************************************************************//**
 * @brief               Set Own slave address in I2C peripheral corresponding to
 *                              parameter specified in OwnSlaveAddrConfigStruct.
 * @param[in]   I2Cx    I2C peripheral selected, should be
 *                              - LPC_I2C0
 *                              - LPC_I2C1
 *                              - LPC_I2C2
 * @param[in]   OwnSlaveAddrConfigStruct        Pointer to a I2C_OWNSLAVEADDR_CFG_Type
 *                              structure that contains the configuration information for the
*               specified I2C slave address.
 * @return              None
 **********************************************************************/
void I2C_SetOwnSlaveAddr(uint8_t i2cId, I2C_OWNSLAVEADDR_CFG_Type *OwnSlaveAddrConfigStruct)
{
        LPC_I2C_TypeDef* I2Cx = I2C_GetPointer(i2cId);

        uint32_t tmp;

        tmp = (((uint32_t)(OwnSlaveAddrConfigStruct->SlaveAddr_7bit << 1)) \
                        | ((OwnSlaveAddrConfigStruct->GeneralCallState == ENABLE) ? 0x01 : 0x00))& I2C_I2ADR_BITMASK;

        switch (OwnSlaveAddrConfigStruct->SlaveAddrChannel)
        {
                case 0:
                        I2Cx->ADR0 = tmp;

                        I2Cx->MASK0 = I2C_I2MASK_MASK((uint32_t) \
                                                        (OwnSlaveAddrConfigStruct->SlaveAddrMaskValue));
                        break;

                case 1:
                        I2Cx->ADR1 = tmp;

                        I2Cx->MASK1 = I2C_I2MASK_MASK((uint32_t) \
                                                        (OwnSlaveAddrConfigStruct->SlaveAddrMaskValue));
                        break;

                case 2:
                        I2Cx->ADR2 = tmp;

                        I2Cx->MASK2 = I2C_I2MASK_MASK((uint32_t) \
                                                        (OwnSlaveAddrConfigStruct->SlaveAddrMaskValue));
                        break;

                case 3:
                        I2Cx->ADR3 = tmp;

                        I2Cx->MASK3 = I2C_I2MASK_MASK((uint32_t) \
                                                        (OwnSlaveAddrConfigStruct->SlaveAddrMaskValue));
                        break;
        }
}


/*********************************************************************//**
 * @brief               Configures functionality in I2C monitor mode
 * @param[in]   I2Cx    I2C peripheral selected, should be
 *                              - LPC_I2C0
 *                              - LPC_I2C1
 *                              - LPC_I2C2
 * @param[in]   MonitorCfgType Monitor Configuration type, should be:
 *                              - I2C_MONITOR_CFG_SCL_OUTPUT: I2C module can 'stretch'
 *                              the clock line (hold it low) until it has had time to
 *                              respond to an I2C interrupt.
 *                              - I2C_MONITOR_CFG_MATCHALL: When this bit is set to '1'
 *                              and the I2C is in monitor mode, an interrupt will be
 *                              generated on ANY address received.
 * @param[in]   NewState New State of this function, should be:
 *                              - ENABLE: Enable this function.
 *                              - DISABLE: Disable this function.
 * @return              None
 **********************************************************************/
void I2C_MonitorModeConfig(uint8_t i2cId, uint32_t MonitorCfgType, FunctionalState NewState)
{
        LPC_I2C_TypeDef* I2Cx = I2C_GetPointer(i2cId);

        if (NewState == ENABLE)
        {
                I2Cx->MMCTRL |= MonitorCfgType;
        }
        else
        {
                I2Cx->MMCTRL &= (~MonitorCfgType) & I2C_I2MMCTRL_BITMASK;
        }
}


/*********************************************************************//**
 * @brief               Enable/Disable I2C monitor mode
 * @param[in]   I2Cx    I2C peripheral selected, should be
 *                              - LPC_I2C0
 *                              - LPC_I2C1
 *                              - LPC_I2C2
 * @param[in]   NewState New State of this function, should be:
 *                              - ENABLE: Enable monitor mode.
 *                              - DISABLE: Disable monitor mode.
 * @return              None
 **********************************************************************/
void I2C_MonitorModeCmd(uint8_t i2cId, FunctionalState NewState)
{
        LPC_I2C_TypeDef* I2Cx = I2C_GetPointer(i2cId);

        if (NewState == ENABLE)
        {
                I2Cx->MMCTRL |= I2C_I2MMCTRL_MM_ENA;
                I2Cx->CONSET = I2C_I2CONSET_AA;
                I2Cx->CONCLR = I2C_I2CONCLR_SIC | I2C_I2CONCLR_STAC;
        }
        else
        {
                I2Cx->MMCTRL &= (~I2C_I2MMCTRL_MM_ENA) & I2C_I2MMCTRL_BITMASK;
                I2Cx->CONCLR = I2C_I2CONCLR_SIC | I2C_I2CONCLR_STAC | I2C_I2CONCLR_AAC;
        }

        I2C_MonitorBufferIndex = 0;
}


/*********************************************************************//**
 * @brief               Get data from I2C data buffer in monitor mode.
 * @param[in]   I2Cx    I2C peripheral selected, should be
 *                              - LPC_I2C0
 *                              - LPC_I2C1
 *                              - LPC_I2C2
 * @return              None
 * Note:        In monitor mode, the I2C module may lose the ability to stretch
 * the clock (stall the bus) if the ENA_SCL bit is not set. This means that
 * the processor will have a limited amount of time to read the contents of
 * the data received on the bus. If the processor reads the DAT shift
 * register, as it ordinarily would, it could have only one bit-time to
 * respond to the interrupt before the received data is overwritten by
 * new data.
 **********************************************************************/
uint8_t I2C_MonitorGetDatabuffer(uint8_t i2cId)
{
        LPC_I2C_TypeDef* I2Cx = I2C_GetPointer(i2cId);

        return ((uint8_t)(I2Cx->DATA_BUFFER));
}

/*********************************************************************//**
 * @brief               Get data from I2C data buffer in monitor mode.
 * @param[in]   I2Cx    I2C peripheral selected, should be
 *                              - LPC_I2C0
 *                              - LPC_I2C1
 *                              - LPC_I2C2
 * @return              None
 * Note:        In monitor mode, the I2C module may lose the ability to stretch
 * the clock (stall the bus) if the ENA_SCL bit is not set. This means that
 * the processor will have a limited amount of time to read the contents of
 * the data received on the bus. If the processor reads the DAT shift
 * register, as it ordinarily would, it could have only one bit-time to
 * respond to the interrupt before the received data is overwritten by
 * new data.
 **********************************************************************/
BOOL_8 I2C_MonitorHandler(uint8_t i2cId, uint8_t *buffer, uint32_t size)
{
        LPC_I2C_TypeDef* I2Cx = I2C_GetPointer(i2cId);

        BOOL_8 ret=FALSE;

        I2Cx->CONCLR = I2C_I2CONCLR_SIC;

        buffer[I2C_MonitorBufferIndex] = (uint8_t)(I2Cx->DATA_BUFFER);

        I2C_MonitorBufferIndex++;

        if(I2C_MonitorBufferIndex >= size)
        {
                ret = TRUE;
        }
        return ret;
}
/*********************************************************************//**
 * @brief               Get status of Master Transfer
 * @param[in]   I2Cx    I2C peripheral selected, should be:
 *                              - LPC_I2C0
 *                              - LPC_I2C1
 *                              - LPC_I2C2
 * @return              Master transfer status, could be:
 *                              - TRUE  master transfer completed
 *                              - FALSE master transfer have not completed yet
 **********************************************************************/
uint32_t I2C_MasterTransferComplete(uint8_t i2cId)
{
        uint32_t retval;

        retval = I2C_MasterComplete[i2cId];

        I2C_MasterComplete[i2cId] = FALSE;

        return retval;
}

/*********************************************************************//**
 * @brief               Get status of Slave Transfer
 * @param[in]   I2Cx    I2C peripheral selected, should be:
 *                              - LPC_I2C0
 *                              - LPC_I2C1
 *                              - LPC_I2C2
 * @return              Complete status, could be: TRUE/FALSE
 **********************************************************************/
uint32_t I2C_SlaveTransferComplete(uint8_t i2cId)
{
        uint32_t retval;

        retval = I2C_SlaveComplete[i2cId];

        I2C_SlaveComplete[i2cId] = FALSE;

        return retval;
}



/**
 * @}
 */


/**
 * @}
 */

/* --------------------------------- End Of File ------------------------------ */
