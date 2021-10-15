/**
 * @file DcMotorWheel.cpp
 * 
 * @brief       DcMotorWheel
 * @note        なし
 * 
 * @version     1.0.0
 * @date        2021/10/15
 * 
 * @copyright   (C) 2021 Motoyuki Endo
 */
#include "DcMotorWheel.h"


//----------------------------------------------------------------
//  <macro>
//----------------------------------------------------------------
#define ARRAY_LENGTH(in_array)      (sizeof(in_array)/sizeof(in_array[0]))
#define UNUSED_VARIABLE(in_x)       (void)(in_x)

#define SAFE_DELETE(x) if(x){ delete(x); x = NULL;}           //delete開放処理
#define SAFE_DELETE_ARRAY(x) if(x){ delete [] (x); x = NULL;} //delete[]開放処理

#define OFF                         (0)
#define ON                          (1)

#define SIGN(x) ((x>0)-(x<0))


//----------------------------------------------------------------
//  <function>
//----------------------------------------------------------------
/**
 * @brief       コンストラクタ
 * @note        なし
 * @param[in]   i_config : コンフィグ設定
 * @retval      なし
 */
DcMotorWheel::DcMotorWheel( DcMotorWheelConfig i_config )
{
	_mutex = xSemaphoreCreateMutex();
	xSemaphoreGive( _mutex );

	_dir = i_config.wheelDir;

	smooth = i_config.smoothConfig;
	_isSmoothEnable = &smooth.isSmoothEnable;
	_smtCoefficient = &smooth.coefficient;

	reqSpeed = 0;
	targetSpeed = 0.0;
	outSpeed = 0;
}


/**
 * @brief       デストラクタ
 * @note        なし
 * @param       なし
 * @retval      なし
 */
DcMotorWheel::~DcMotorWheel( void )
{
	// DO_NOTHING
}


/**
 * @brief       車輪回転
 * @note        なし
 * @param[in]   i_speed : 回転速度
 * @retval      なし
 */
void DcMotorWheel::RollWheel( int32_t i_speed )
{
	xSemaphoreTake( _mutex , portMAX_DELAY );
	reqSpeed = i_speed;
	xSemaphoreGive( _mutex );

	if( *_isSmoothEnable )
	{
		// DO_NOTHING
	}
	else
	{
		SetSpeed( (int32_t)reqSpeed );
	}
}


/**
 * @brief       車輪回転停止
 * @note        なし
 * @param       なし
 * @retval      なし
 */
void DcMotorWheel::StopWheel( void )
{
	xSemaphoreTake( _mutex , portMAX_DELAY );
	reqSpeed = 0;
	targetSpeed = 0.0;
	xSemaphoreGive( _mutex );
}


/**
 * @brief       制御周期ハンドラ
 * @note        なし
 * @param       なし
 * @retval      なし
 */
void DcMotorWheel::ControlCycle( void )
{
	SmoothControl();

	if( *_isSmoothEnable )
	{
		SetSpeed( (int32_t)targetSpeed );
	}
	else
	{
		// DO_NOTHING
	}
}


/**
 * @brief       回転速度設定
 * @note        なし
 * @param[in]   i_speed : 要求回転速度
 * @retval      なし
 */
void DcMotorWheel::SetSpeed( int32_t i_speed )
{
	int32_t limSpeed;

	limSpeed = constrain( i_speed , -100 , 100 );

	outSpeed = (int32_t)( limSpeed * _dir );
}


/**
 * @brief       なまし処理
 * @note        なし
 * @param       なし
 * @retval      なし
 */
void DcMotorWheel::SmoothControl( void )
{
	xSemaphoreTake( _mutex , portMAX_DELAY );
	if( *_isSmoothEnable )
	{
		targetSpeed = targetSpeed + ( ( (float)reqSpeed - targetSpeed ) * (*_smtCoefficient) );
		if( (int32_t)targetSpeed == 0 )
		{
			targetSpeed = 0.0;
		}
	}
	else{
		targetSpeed = (float)reqSpeed;
	}
	xSemaphoreGive( _mutex );
}
