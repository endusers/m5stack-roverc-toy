/**
 * @file DcMotor.h
 * 
 * @brief       DCモータ
 * @note        なし
 * 
 * @version     1.0.0
 * @date        2021/10/15
 * 
 * @copyright   (C) 2021 Motoyuki Endo
 */
#ifndef __DCMOTOR_H
#define __DCMOTOR_H

#include <Arduino.h>
#include <M5StickCPlus.h>


//----------------------------------------------------------------
//  <macro>
//----------------------------------------------------------------
#define DCMOT_I2CBUFF_SIZE						(16)


//----------------------------------------------------------------
//  <enum>
//----------------------------------------------------------------


//----------------------------------------------------------------
//  <struct>
//----------------------------------------------------------------
struct DcMotorPort_Tag
{
	int8_t pinNoSDA;
	int8_t pinNoSCL;
	uint8_t addrI2C;
	uint8_t headRegNum;
};
typedef struct DcMotorPort_Tag					DcMotorPort;


//----------------------------------------------------------------
//  <class>
//----------------------------------------------------------------
/**
 * @brief       DCモータ
 * @note        なし
 */
class DcMotor
{
private:
	DcMotorPort _port;
	int8_t *_pinNoSDA;
	int8_t *_pinNoSCL;
	uint8_t *_addrI2C;
	uint8_t *_headRegNum;
	uint8_t _i2cbuf[DCMOT_I2CBUFF_SIZE];

public:
	DcMotor( DcMotorPort i_port );                                  // コンストラクタ
	~DcMotor( void );                                               // デストラクタ

	void InitMotor( void );                                         // モータ初期化
	void RunMotor( int32_t i_num , int32_t *i_speed );              // モータ駆動
};

#endif
