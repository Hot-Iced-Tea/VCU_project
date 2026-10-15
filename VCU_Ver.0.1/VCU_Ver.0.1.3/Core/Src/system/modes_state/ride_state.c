/*
 * ride_state.c
 *
 *  Created on: Sep 13, 2026
 *      Author: Think
 */
#include "cfg/GPIO_config.h"

#include "IO_control/input_ctrl.h"
#include "IO_control/output_ctrl.h"

#include "system/modes_state/ride_state.h"
#include "system/modes_state/ride_mode.h"

#include "system/system.h"

#include <stdint.h>


/* =========================================================
 * Private function
 * ========================================================= */
static void system_rideNextState(void);


/* =========================================================
 * RIDE INITIALIZATION
 * ========================================================= */
void system_rideInit(void)
{
    /*
     * Nyalakan controller relay
     * ketika masuk RIDE.
     */
    output_setState(&ignition_rly, GPIO_PIN_SET);


    /*
     * Reset mode berkendara.
     *
     * Setiap kali masuk RIDE:
     *
     * ECO
     *
     * bukan mode terakhir.
     */
    rideMode_Init();
}


/* =========================================================
 * RIDE TASK
 * ========================================================= */
void system_rideTask(void)
{
    /*
     * Proses switch mode.
     *
     * rideMode_Init() TIDAK boleh dipanggil
     * di sini karena Task berjalan terus menerus.
     */
    rideMode_Task();


    /*
     * Periksa apakah RIDE harus dihentikan.
     */
    system_rideNextState();
}


/* =========================================================
 * RIDE NEXT STATE
 * ========================================================= */
static void system_rideNextState(void)
{
    /*
     * Keluar dari RIDE apabila:
     *
     * 1. Side stand turun
     * ATAU
     * 2. Brake aktif
     */
    if ((input_getState(&sideStand_sw) == GPIO_PIN_RESET) ||
        (CAN_lowBrake == 1U))
    {
        /*
         * Matikan controller relay.
         */
        output_setState(&ignition_rly, GPIO_PIN_RESET);


        /*
         * Kembali ke STANDBY.
         */
        system_setState(SYSTEM_STANDBY);

        return;
    }
}
