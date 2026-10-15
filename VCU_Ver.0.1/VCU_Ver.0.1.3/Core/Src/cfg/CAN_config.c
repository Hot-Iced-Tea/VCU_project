/*
 * CAN_config.c
 *
 *  Created on: Sep 16, 2026
 *      Author: fahmif_imoto
 */


#include "cfg/CAN_config.h"

extern FDCAN_HandleTypeDef hfdcan1;


/* ============================================================
 * PRIVATE FUNCTIONS
 * ============================================================ */

static void CAN_Config_Filter(void);
static void CAN_Config_GlobalFilter(void);
static void CAN_Config_Start(void);


/* ============================================================
 * INITIALIZATION
 * ============================================================ */

void CAN_config_Init(void)
{
    CAN_Config_Filter();

    CAN_Config_GlobalFilter();

    CAN_Config_Start();
}


/* ============================================================
 * FILTER CONFIGURATION
 * ============================================================ */

static void CAN_Config_Filter(void)
{
    FDCAN_FilterTypeDef std_filter = {0};
    FDCAN_FilterTypeDef ext_filter = {0};


    /* --------------------------------------------------------
     * STANDARD CAN
     * Accept ALL Standard IDs
     * -------------------------------------------------------- */

    std_filter.IdType =
        FDCAN_STANDARD_ID;

    std_filter.FilterIndex =
        CAN_STD_FILTER_INDEX;

    std_filter.FilterType =
        FDCAN_FILTER_MASK;

    std_filter.FilterConfig =
        FDCAN_FILTER_TO_RXFIFO0;

    std_filter.FilterID1 =
        CAN_STD_FILTER_ID1;

    std_filter.FilterID2 =
        CAN_STD_FILTER_ID2;


    if (HAL_FDCAN_ConfigFilter(
            &hfdcan1,
            &std_filter) != HAL_OK)
    {
        Error_Handler();
    }


    /* --------------------------------------------------------
     * EXTENDED CAN
     * Accept ALL Extended IDs
     * -------------------------------------------------------- */

    ext_filter.IdType =
        FDCAN_EXTENDED_ID;

    ext_filter.FilterIndex =
        CAN_EXT_FILTER_INDEX;

    ext_filter.FilterType =
        FDCAN_FILTER_MASK;

    ext_filter.FilterConfig =
        FDCAN_FILTER_TO_RXFIFO0;

    ext_filter.FilterID1 =
        CAN_EXT_FILTER_ID1;

    ext_filter.FilterID2 =
        CAN_EXT_FILTER_ID2;


    if (HAL_FDCAN_ConfigFilter(
            &hfdcan1,
            &ext_filter) != HAL_OK)
    {
        Error_Handler();
    }
}


/* ============================================================
 * GLOBAL FILTER
 * ============================================================ */

static void CAN_Config_GlobalFilter(void)
{
    if (HAL_FDCAN_ConfigGlobalFilter(
            &hfdcan1,

            FDCAN_ACCEPT_IN_RX_FIFO0,
            FDCAN_ACCEPT_IN_RX_FIFO0,

            FDCAN_REJECT_REMOTE,
            FDCAN_REJECT_REMOTE) != HAL_OK)
    {
        Error_Handler();
    }
}


/* ============================================================
 * START FDCAN
 * ============================================================ */

static void CAN_Config_Start(void)
{
    if (HAL_FDCAN_Start(&hfdcan1) != HAL_OK)
    {
        Error_Handler();
    }


    if (HAL_FDCAN_ActivateNotification(
            &hfdcan1,
            FDCAN_IT_RX_FIFO0_NEW_MESSAGE,
            0U) != HAL_OK)
    {
        Error_Handler();
    }
}

