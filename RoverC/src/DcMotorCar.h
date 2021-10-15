/**
 * @file DcMotorCar.h
 * 
 * @brief       DcMotorCar
 * @note        なし
 * 
 * @version     1.0.0
 * @date        2021/10/15
 * 
 * @copyright   (C) 2021 Motoyuki Endo
 */
#ifndef __DCMOTORCAR_H
#define __DCMOTORCAR_H

#include <Arduino.h>
#include <M5StickCPlus.h>
#include "DcMotor.h"
#include "DcMotorWheel.h"
#include "DcMotorCar_Config.h"
#include "JoyStick.h"


//----------------------------------------------------------------
//  <macro>
//----------------------------------------------------------------
#define DCMOTORCAR_MAIN_CYCLE					(10 * 1000)						// 10ms
#define DCMOTORCAR_JOYCTRL_CYCLE				(100)							// 100ms


//----------------------------------------------------------------
//  <enum>
//----------------------------------------------------------------


//----------------------------------------------------------------
//  <union>
//----------------------------------------------------------------


//----------------------------------------------------------------
//  <class>
//----------------------------------------------------------------
/**
 * @brief       DcMotorCar
 * @note        なし
 */
class DcMotorCar
{
private:
	SemaphoreHandle_t _mutex;

	int32_t _numWheel;
	int32_t *_speedMotor;
	DcMotorWheel _flWheel;
	DcMotorWheel _frWheel;
	DcMotorWheel _rlWheel;
	DcMotorWheel _rrWheel;
	DcMotor _drvMotor;

	int32_t _reqMaxValue;

	boolean _isLcdUpdate;

	JoyStick _joy;
	uint32_t _JoyCtrlCycle;

	void SetMotorSpeed( void );                                         // モーター速度設定
	void JoyControl( void );                                            // JoyStickコントロール
#ifdef _SERIAL_DEBUG_
	void SerialDebug( void );                                           // シリアルデバッグ
#endif

public:
	DcMotorCar( void );                                                 // コンストラクタ
	~DcMotorCar( void );                                                // デストラクタ

	void Init( void );                                                  // イニシャライズ
	void MainLoop( void );                                              // メインループ
	void MainCycle( void );                                             // 制御周期ハンドラ
	void BleJoyCtrlCycle( void );                                       // BleJoyStickコントロール周期
};

#endif
