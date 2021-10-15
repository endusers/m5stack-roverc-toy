/**
 * @file DcMotor.cpp
 * 
 * @brief       DCモータ
 * @note        なし
 * 
 * @version     1.0.0
 * @date        2021/10/15
 * 
 * @copyright   (C) 2021 Motoyuki Endo
 */
#include "DcMotor.h"


//----------------------------------------------------------------
//  <macro>
//----------------------------------------------------------------
#define ARRAY_LENGTH(in_array)      (sizeof(in_array)/sizeof(in_array[0]))
#define UNUSED_VARIABLE(in_x)       (void)(in_x)

#define SAFE_DELETE(x) if(x){ delete(x); x = NULL;}           //delete開放処理
#define SAFE_DELETE_ARRAY(x) if(x){ delete [] (x); x = NULL;} //delete[]開放処理

#define OFF                         (0)
#define ON                          (1)


//----------------------------------------------------------------
//  <function>
//----------------------------------------------------------------
/**
 * @brief       コンストラクタ
 * @note        なし
 * @param[in]   i_port : モータポート設定
 * @retval      なし
 */
DcMotor::DcMotor( DcMotorPort i_port )
{
	_port = i_port;

	_pinNoSDA = &_port.pinNoSDA;
	_pinNoSCL = &_port.pinNoSCL;
	_addrI2C = &_port.addrI2C;
	_headRegNum = &_port.headRegNum;

	memset( &_i2cbuf[0] , 0 , sizeof(_i2cbuf) );
}


/**
 * @brief       デストラクタ
 * @note        なし
 * @param       なし
 * @retval      なし
 */
DcMotor::~DcMotor( void )
{
	// DO_NOTHING
}

/**
 * @brief       モータ初期化
 * @note        なし
 * @param       なし
 * @retval      なし
 */
void DcMotor::InitMotor( void )
{
	Wire.begin( *_pinNoSDA, *_pinNoSCL );
}


/**
 * @brief       モータ駆動
 * @note        なし
 * @param[in]   i_num : モータ数
 * @param[in]   *i_speed : 速度
 * @retval      なし
 */
void DcMotor::RunMotor( int32_t i_num , int32_t *i_speed )
{
	int32_t i;
	int32_t j;
	int32_t length;

	length = i_num + 1;
	if( length > ARRAY_LENGTH(_i2cbuf) ){
		length = ARRAY_LENGTH(_i2cbuf);
	}

	_i2cbuf[0] = *_headRegNum;
	for( i = 1, j = 0; i < length; i++, j++ ){
		_i2cbuf[i] = (int8_t)constrain( i_speed[j] , -100 , 100 );
	}

	Wire.beginTransmission( *_addrI2C );
	Wire.write( &_i2cbuf[0], length );
	Wire.endTransmission();
}
