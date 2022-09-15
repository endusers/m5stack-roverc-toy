/**
 * @file DcMotorCar_Config.h
 * 
 * @brief       DcMotorCar_Config
 * @note        なし
 * 
 * @version     1.1.0
 * @date        2022/09/10
 * 
 * @copyright   (C) 2021-2022 Motoyuki Endo
 */
#ifndef __DCMOTORCAR_CONFIG_H
#define __DCMOTORCAR_CONFIG_H

#include <Arduino.h>
#include "DcMotor.h"
#include "DcMotorWheel.h"


//----------------------------------------------------------------
//  <macro>
//----------------------------------------------------------------
#define _SERIAL_DEBUG_

#define DCMOTORCAR_JOYCONNECT_PREFERBLUETOOTH	0
#define DCMOTORCAR_JOYCONNECT_DUAL				1
#define DCMOTORCAR_JOYCONNECT_MODE				DCMOTORCAR_JOYCONNECT_PREFERBLUETOOTH


//----------------------------------------------------------------
//  <enum>
//----------------------------------------------------------------


//----------------------------------------------------------------
//  <struct>
//----------------------------------------------------------------


//----------------------------------------------------------------
//  <table>
//----------------------------------------------------------------
extern const DcMotorWheelConfigs Config;

#endif
