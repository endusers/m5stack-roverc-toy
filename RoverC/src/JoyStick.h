/**
 * @file JoyStick.h
 * 
 * @brief       JoyStick
 * @note        なし
 * 
 * @version     1.0.0
 * @date        2021/10/15
 * 
 * @copyright   (C) 2021 Motoyuki Endo
 */
#ifndef __JOYSTICK_H
#define __JOYSTICK_H

#include "JoyStick_Config.h"
#include <Arduino.h>
#if JOYSTICK_BLUETOOTH_TYPE == JOYSTICK_BLUETOOTH_SUPPORT
#include <PS4Controller.h>
#endif


//----------------------------------------------------------------
//  <macro>
//----------------------------------------------------------------
#define JOYSTICKDIRECTIONTBL_NUM        (9)


//----------------------------------------------------------------
//  <enum>
//----------------------------------------------------------------
enum JoyStickDirection_Tag
{
	JOYSTKDIR_NONE					= 0	,
	JOYSTKDIR_UP						,
	JOYSTKDIR_DOWN						,
	JOYSTKDIR_LEFT						,
	JOYSTKDIR_RIGHT						,
	JOYSTKDIR_UPLEFT					,
	JOYSTKDIR_UPRIGHT					,
	JOYSTKDIR_DOWNLEFT					,
	JOYSTKDIR_DOWNRIGHT					,
};
typedef enum JoyStickDirection_Tag				JoyStickDirection;


//----------------------------------------------------------------
//  <union>
//----------------------------------------------------------------


//----------------------------------------------------------------
//  <struct>
//----------------------------------------------------------------
struct JoyInfo_Tag
{
	float lStickH;
	float lStickV;
	float rStickH;
	float rStickV;
	float l2Axes;
	float r2Axes;
	float lrAxes;
	float updownAxes;
	int32_t squareButton;
	int32_t crossButton;
	int32_t circleButton;
	int32_t triangleButton;
	int32_t l1Button;
	int32_t r1Button;
	int32_t battery;
	int32_t charging;
};
typedef struct JoyInfo_Tag						JoyInfo;

struct JoyStickDirectionTbl_Tag
{
	struct
	{
		JoyStickDirection dir;
		float minAngle;
		float maxAngle;
	}data[JOYSTICKDIRECTIONTBL_NUM];
};
typedef struct JoyStickDirectionTbl_Tag			JoyStickDirectionTbl;


//----------------------------------------------------------------
//  <class>
//----------------------------------------------------------------
/**
 * @brief       JoyStick
 * @note        なし
 */
class JoyStick
{
private:
	static const JoyStickDirectionTbl JOYSTICKDIRECTION_BLE_TBL;

public:
	boolean isConnectedBle;
	boolean isBeforeConnectedBle;
	JoyInfo joyInfBle;
	JoyInfo beforeJoyInfBle;

	JoyStick( void );                                                           // コンストラクタ
	~JoyStick( void );                                                          // デストラクタ

	void Init( void );                                                          // イニシャライズ
#if JOYSTICK_BLUETOOTH_TYPE == JOYSTICK_BLUETOOTH_SUPPORT
	void UpdateJoyStickInfoBle( ps4_t *i_ps4 );                                 // JoyStickInfo更新
#endif
	float GetJoyStickTilt( float i_hStick , float i_vStick );                   // JoyStick傾き取得
	JoyStickDirection GetJoyStickDirection( float i_hStick , float i_vStick );  // JoyStick方向取得
};

#endif
