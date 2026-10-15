/*
 * charge_state.c
 *
 *  Created on: Sep 11, 2026
 *      Author: fahmif_imoto
 */

#include "cfg/GPIO_config.h"
#include "IO_control/output_ctrl.h"
#include "system/modes_state/charge_state.h"
#include "system/system.h"

//static void system_chargerUnplugged(void);
static void system_chargeProcess(void);

void system_chargeInit(void){

}

void system_chargeTask(void){
	if (charger == 0){
		system_setState(chargeReturnState);
		return;
	}

	system_chargeProcess();
}

static void system_chargeProcess(void){

}

//static void system_chargerUnplugged(void){
//	if(charger == 0){
//		system_setState(chargeReturnState);
//	}
//}
