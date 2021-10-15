/**
 * @file DcMotorWheel.h
 * 
 * @brief       DcMotorWheel
 * @note        なし
 * 
 * @version     1.0.0
 * @date        2021/10/15
 * 
 * @copyright   (C) 2021 Motoyuki Endo
 */
#ifndef __DCMOTORWHEEL_H
#define __DCMOTORWHEEL_H

#include <Arduino.h>
#include "DcMotor.h"


//----------------------------------------------------------------
//  <macro>
//----------------------------------------------------------------


//----------------------------------------------------------------
//  <enum>
//----------------------------------------------------------------
enum DcMotorWheelForwardDir_Tag
{
	DCMOTWHFWDDIR_CW				= 1		,
	DCMOTWHFWDDIR_CCW				= -1	,
};
typedef enum DcMotorWheelForwardDir_Tag			DcMotorWheelForwardDir;


//----------------------------------------------------------------
//  <struct>
//----------------------------------------------------------------
struct DcMotorWheelSmooth_Tag
{
	boolean isSmoothEnable;
	float coefficient;
};
typedef struct DcMotorWheelSmooth_Tag			DcMotorWheelSmooth;

struct DcMotorWheelConfig_Tag
{
	DcMotorWheelForwardDir wheelDir;
	DcMotorWheelSmooth smoothConfig;
};
typedef struct DcMotorWheelConfig_Tag			DcMotorWheelConfig;

struct DcMotorWheelConfigs_Tag
{
	int32_t num;
	DcMotorPort portConfig;
	DcMotorWheelConfig configs[];
};
typedef struct DcMotorWheelConfigs_Tag			DcMotorWheelConfigs;


//----------------------------------------------------------------
//  <class>
//----------------------------------------------------------------
/**
 * @brief       DcMotorWheel
 * @note        なし
 */
class DcMotorWheel
{
private:
	SemaphoreHandle_t _mutex;

	DcMotorWheelForwardDir _dir;

	boolean *_isSmoothEnable;
	float *_smtCoefficient;

	void SetSpeed( int32_t i_speed );                               // 回転速度設定
	void SmoothControl();                                           // なまし処理

public:
	int32_t reqSpeed;
	float targetSpeed;
	int32_t outSpeed;
	DcMotorWheelSmooth smooth;

	DcMotorWheel( DcMotorWheelConfig i_config );                    // コンストラクタ
	~DcMotorWheel( void );                                          // デストラクタ

	void RollWheel( int32_t i_speed );                              // 車輪回転
	void StopWheel( void );                                         // 車輪回転停止
	void ControlCycle( void );                                      // 制御周期ハンドラ
};

#endif
