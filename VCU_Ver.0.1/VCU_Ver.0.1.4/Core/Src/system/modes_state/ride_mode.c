/*
 * ride_mode.c
 *
 *  Created on: Sep 18, 2026
 *      Author: fahmif_imoto
 */

#include "system/modes_state/ride_mode.h"

#include "cfg/GPIO_config.h"
#include "IO_control/input_ctrl.h"
#include  "IO_control/output_ctrl.h"

#include "stm32h5xx_hal.h"


/* ============================================================
 * CONFIGURATION
 * ============================================================ */

#define RIDE_MODE_REVERSE_MAX_SPEED_KMH    2U
#define RIDE_MODE_LONG_PRESS_MS            1000U
#define RIDE_MODE_DEBOUNCE_MS              50U


/* ============================================================
 * VIRTUAL CAN DATA
 * ============================================================ */

volatile uint16_t CAN_vehicleSpeed_kmh = 0U;


/* ============================================================
 * CURRENT RIDE MODE
 * ============================================================ */

static rideMode_t currentRideMode;
static rideMode_t lastForwardMode;


/* ============================================================
 * BUTTON STATE
 *
 * Hardware polarity:
 *
 * GPIO_PIN_SET   = PRESSED
 * GPIO_PIN_RESET = RELEASED
 * ============================================================ */

static GPIO_PinState speedMode_stableState;
static GPIO_PinState speedMode_lastRawState;

static uint32_t speedMode_stateChangeTick;

static uint8_t speedMode_pressed;
static uint8_t reverse_active;

static uint32_t speedMode_pressStartTick;


/* ============================================================
 * DEBUG VARIABLES
 * ============================================================ */

volatile uint8_t debug_rideMode = 0U;

volatile uint8_t debug_speedModeState = 0U;
volatile uint8_t debug_speedModeRawState = 0U;

volatile uint8_t debug_speedModePressed = 0U;
volatile uint8_t debug_reverseActive = 0U;

volatile uint32_t debug_pressDuration = 0U;
volatile uint32_t debug_debounceDuration = 0U;


/* ============================================================
 * PRIVATE FUNCTION PROTOTYPES
 * ============================================================ */

static void rideMode_SetOutput(rideMode_t mode);

static void rideMode_NextForwardMode(void);


/* ============================================================
 * INITIALIZATION
 * ============================================================ */

void rideMode_Init(void)
{
    GPIO_PinState initialState;


    /*
     * Default mode when entering RIDE.
     */
    currentRideMode = RideMode_ECO;

    lastForwardMode = RideMode_ECO;
    output_setState(&speedMode_High, GPIO_PIN_RESET);
    output_setState(&speedMode_Low, GPIO_PIN_SET);
    output_setState(&speedMode_Reverse, GPIO_PIN_RESET);


    /*
     * Reset button variables.
     */
    speedMode_pressed = 0U;

    reverse_active = 0U;

    speedMode_pressStartTick = 0U;


    /*
     * Read initial button state.
     */
    initialState = input_getState(&speedMode_sw);


    /*
     * Initialize debounce states.
     */
    speedMode_stableState = initialState;

    speedMode_lastRawState = initialState;

    speedMode_stateChangeTick = HAL_GetTick();


    /*
     * Set ECO.
     */
    rideMode_SetOutput(RideMode_ECO);


    /*
     * Debug.
     */
    debug_rideMode = (uint8_t)currentRideMode;

    debug_speedModeState =
        (speedMode_stableState == GPIO_PIN_SET) ? 1U : 0U;

    debug_speedModeRawState =
        (speedMode_lastRawState == GPIO_PIN_SET) ? 1U : 0U;

    debug_speedModePressed = 0U;

    debug_reverseActive = 0U;

    debug_pressDuration = 0U;

    debug_debounceDuration = 0U;
}


/* ============================================================
 * RIDE MODE TASK
 * ============================================================ */

void rideMode_Task(void)
{
    GPIO_PinState rawState;
    uint32_t currentTick;


    /*
     * Read raw input.
     */
    rawState = input_getState(&speedMode_sw);

    currentTick = HAL_GetTick();


    /*
     * Debug raw state.
     */
    debug_speedModeRawState =
        (rawState == GPIO_PIN_SET) ? 1U : 0U;


    /* ========================================================
     * DEBOUNCE
     * ======================================================== */

    /*
     * Raw input changed.
     */
    if (rawState != speedMode_lastRawState)
    {
        speedMode_lastRawState = rawState;

        speedMode_stateChangeTick = currentTick;
    }


    /*
     * Calculate how long raw input has remained unchanged.
     */
    debug_debounceDuration =
        currentTick - speedMode_stateChangeTick;


    /*
     * If raw state has remained stable long enough,
     * accept it as the new stable state.
     */
    if ((rawState != speedMode_stableState) &&
        ((currentTick - speedMode_stateChangeTick)
         >= RIDE_MODE_DEBOUNCE_MS))
    {
        speedMode_stableState = rawState;


        /* ====================================================
         * VALID PRESS
         *
         * RESET -> SET
         *
         * RELEASED -> PRESSED
         * ==================================================== */

        if (speedMode_stableState == GPIO_PIN_SET)
        {
            speedMode_pressed = 1U;

            reverse_active = 0U;

            speedMode_pressStartTick = currentTick;

            debug_pressDuration = 0U;
        }


        /* ====================================================
         * VALID RELEASE
         *
         * SET -> RESET
         *
         * PRESSED -> RELEASED
         * ==================================================== */

        else
        {
            /*
             * Only process release if there was
             * a valid press before it.
             */
            if (speedMode_pressed == 1U)
            {
                /*
                 * If reverse was activated,
                 * return to previous forward mode.
                 */
                if (reverse_active == 1U)
                {
                    currentRideMode = lastForwardMode;

                    rideMode_SetOutput(currentRideMode);
                }


                /*
                 * Otherwise this was a short press.
                 */
                else
                {
                    rideMode_NextForwardMode();
                }
            }


            /*
             * Reset press state.
             */
            speedMode_pressed = 0U;

            reverse_active = 0U;

            debug_pressDuration = 0U;
        }
    }


    /* ========================================================
     * LONG PRESS
     * ======================================================== */

    if ((speedMode_pressed == 1U) &&
        (speedMode_stableState == GPIO_PIN_SET))
    {
        /*
         * Calculate button hold duration.
         */
        debug_pressDuration =
            currentTick - speedMode_pressStartTick;


        /*
         * Check long press.
         */
        if ((currentTick - speedMode_pressStartTick)
            >= RIDE_MODE_LONG_PRESS_MS)
        {
            /*
             * Reverse only allowed below 2 km/h.
             */
            if ((CAN_vehicleSpeed_kmh <
                 RIDE_MODE_REVERSE_MAX_SPEED_KMH) &&
                (reverse_active == 0U))
            {
                /*
                 * Save previous forward mode.
                 */
                if (currentRideMode != RideMode_REVERSE)
                {
                    lastForwardMode = currentRideMode;
                }


                /*
                 * Enter reverse.
                 */
                currentRideMode = RideMode_REVERSE;

                reverse_active = 1U;


                /*
                 * Virtual output.
                 */
                rideMode_SetOutput(RideMode_REVERSE);
            }
        }
    }
    else
    {
        /*
         * No active press.
         */
        if (speedMode_pressed == 0U)
        {
            debug_pressDuration = 0U;
        }
    }


    /* ========================================================
     * DEBUG UPDATE
     * ======================================================== */

    debug_speedModeState =
        (speedMode_stableState == GPIO_PIN_SET) ? 1U : 0U;

    debug_speedModePressed = speedMode_pressed;

    debug_reverseActive = reverse_active;

    debug_rideMode = (uint8_t)currentRideMode;
}


/* ============================================================
 * NEXT FORWARD MODE
 *
 * ECO -> DRIVE -> SPORT -> ECO
 * ============================================================ */

static void rideMode_NextForwardMode(void)
{
    switch (currentRideMode)
    {
        case RideMode_ECO		: currentRideMode = RideMode_DRIVE;
            break;
        case RideMode_DRIVE		: currentRideMode = RideMode_SPORT;
            break;
        case RideMode_SPORT		: currentRideMode = RideMode_ECO;
            break;
        case RideMode_REVERSE	: currentRideMode = lastForwardMode;
            break;
        default					: currentRideMode = RideMode_ECO;
            break;
    }


    /*
     * Save forward mode.
     */
    if (currentRideMode != RideMode_REVERSE)
    {
        lastForwardMode = currentRideMode;
    }


    /*
     * Virtual output.
     */
    rideMode_SetOutput(currentRideMode);
}


/* ============================================================
 * MODE OUTPUT
 *
 * Currently virtual.
 * ============================================================ */

static void rideMode_SetOutput(rideMode_t mode)
{
    /*
     * GPIO outputs are intentionally not used yet.
     *
     * speedMode_High
     * speedMode_Low
     * speedMode_Reverse
     * speedMode_Cruise
     *
     * are not physically configured yet.
     */

    switch (mode)
    {
        case RideMode_ECO:
            output_setState(&speedMode_High, GPIO_PIN_RESET);
            output_setState(&speedMode_Low, GPIO_PIN_SET);
            output_setState(&speedMode_Reverse, GPIO_PIN_RESET);
            break;


        case RideMode_DRIVE:
            output_setState(&speedMode_High, GPIO_PIN_RESET);
            output_setState(&speedMode_Low, GPIO_PIN_RESET);
            output_setState(&speedMode_Reverse, GPIO_PIN_RESET);
            break;


        case RideMode_SPORT:
            output_setState(&speedMode_High, GPIO_PIN_SET);
            output_setState(&speedMode_Low, GPIO_PIN_RESET);
            output_setState(&speedMode_Reverse, GPIO_PIN_RESET);
            break;


        case RideMode_REVERSE:
            output_setState(&speedMode_Reverse, GPIO_PIN_SET);
            break;


        default:

            currentRideMode = RideMode_ECO;
            output_setState(&speedMode_High, GPIO_PIN_RESET);
            output_setState(&speedMode_Low, GPIO_PIN_RESET);
            output_setState(&speedMode_Reverse, GPIO_PIN_RESET);
            break;
    }
}


/* ============================================================
 * GET CURRENT MODE
 * ============================================================ */

rideMode_t rideMode_Get(void)
{
    return currentRideMode;
}


/* ============================================================
 * GET LAST FORWARD MODE
 * ============================================================ */

rideMode_t rideMode_GetLastForwardMode(void)
{
    return lastForwardMode;
}
