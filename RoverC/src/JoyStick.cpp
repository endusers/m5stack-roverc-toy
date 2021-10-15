/**
 * @file JoyStick.cpp
 * 
 * @brief       JoyStick
 * @note        なし
 * 
 * @version     1.0.0
 * @date        2021/10/15
 * 
 * @copyright   (C) 2021 Motoyuki Endo
 */
#include "JoyStick.h"


//----------------------------------------------------------------
//  <macro>
//----------------------------------------------------------------
#define ARRAY_LENGTH(in_array)      (sizeof(in_array)/sizeof(in_array[0]))
#define UNUSED_VARIABLE(in_x)       (void)(in_x)

#define SAFE_DELETE(x) if(x){ delete(x); x = NULL;}           //delete開放処理
#define SAFE_DELETE_ARRAY(x) if(x){ delete [] (x); x = NULL;} //delete[]開放処理

#define OFF                         (0)
#define ON                          (1)

#define MAPF(x,in_min,in_max,out_min,out_max)\
    ((x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min)


//----------------------------------------------------------------
//  <constant>
//----------------------------------------------------------------
const JoyStickDirectionTbl JoyStick::JOYSTICKDIRECTION_BLE_TBL =
{
    {
        { JOYSTKDIR_RIGHT     ,  -30.0 ,   30.0 } ,
        { JOYSTKDIR_UPRIGHT   ,   30.0 ,   60.0 } ,
        { JOYSTKDIR_UP        ,   60.0 ,  120.0 } ,
        { JOYSTKDIR_UPLEFT    ,  120.0 ,  150.0 } ,
        { JOYSTKDIR_LEFT      ,  150.0 ,  180.0 } ,
        { JOYSTKDIR_DOWNRIGHT ,  -60.0 ,  -30.0 } ,
        { JOYSTKDIR_DOWN      , -120.0 ,  -60.0 } ,
        { JOYSTKDIR_DOWNLEFT  , -150.0 , -120.0 } ,
        { JOYSTKDIR_LEFT      , -180.0 , -150.0 } ,
    }
};


//----------------------------------------------------------------
//  <function>
//----------------------------------------------------------------
/**
 * @brief       コンストラクタ
 * @note        なし
 * @param       なし
 * @retval      なし
 */
JoyStick::JoyStick( void )
{
	isConnectedBle = false;
	isBeforeConnectedBle = false;
	memset( &joyInfBle , 0 , sizeof(joyInfBle) );
	memset( &beforeJoyInfBle , 0 , sizeof(beforeJoyInfBle) );
}


/**
 * @brief       デストラクタ
 * @note        なし
 * @param       なし
 * @retval      なし
 */
JoyStick::~JoyStick( void )
{
	// DO_NOTHING
}


/**
 * @brief       イニシャライズ
 * @note        なし
 * @param       なし
 * @retval      なし
 */
void JoyStick::Init( void )
{
#if JOYSTICK_BLUETOOTH_TYPE == JOYSTICK_BLUETOOTH_SUPPORT
	// uint8_t derived_mac_addr[6];
	// char ble_mac_addr[17 + 1];

	// esp_read_mac( derived_mac_addr, ESP_MAC_BT );
	// snprintf( ble_mac_addr, sizeof(ble_mac_addr), "%02X:%02X:%02X:%02X:%02X:%02X",
	// 	derived_mac_addr[0], derived_mac_addr[1], derived_mac_addr[2],
	// 	derived_mac_addr[3], derived_mac_addr[4], derived_mac_addr[5] );

	// PS4.begin( ble_mac_addr );

	PS4.begin( BLUETOOTH_MAC_ADDRESS );
#endif
}


/**
 * @brief       JoyStickInfo更新
 * @note        なし
 * @param[in]   i_msg : Ps4コントローラ情報
 * @retval      なし
 */
#if JOYSTICK_BLUETOOTH_TYPE == JOYSTICK_BLUETOOTH_SUPPORT
void JoyStick::UpdateJoyStickInfoBle( ps4_t *i_ps4 )
{
	beforeJoyInfBle = joyInfBle;

	joyInfBle.lStickH        = MAPF( i_ps4->analog.stick.lx, -128.0, 127.0, -1.0, 1.0 );
	joyInfBle.lStickV        = MAPF( i_ps4->analog.stick.ly, -128.0, 127.0, -1.0, 1.0 );
	joyInfBle.rStickH        = MAPF( i_ps4->analog.stick.rx, -128.0, 127.0, -1.0, 1.0 );
	joyInfBle.rStickV        = MAPF( i_ps4->analog.stick.ry, -128.0, 127.0, -1.0, 1.0 );
	joyInfBle.l2Axes         = MAPF( i_ps4->analog.button.l2, 0.0, 255.0, 0.0, 1.0 );
	joyInfBle.r2Axes         = MAPF( i_ps4->analog.button.r2, 0.0, 255.0, 0.0, 1.0 );
	joyInfBle.lrAxes         = (i_ps4->button.left) + (-i_ps4->button.right);
	joyInfBle.updownAxes     = (i_ps4->button.up) + (-i_ps4->button.down);
	joyInfBle.squareButton   = i_ps4->button.square;
	joyInfBle.crossButton    = i_ps4->button.cross;
	joyInfBle.circleButton   = i_ps4->button.circle;
	joyInfBle.triangleButton = i_ps4->button.triangle;
	joyInfBle.l1Button       = i_ps4->button.l1;
	joyInfBle.r1Button       = i_ps4->button.r1;
	joyInfBle.battery        = i_ps4->status.battery;
	joyInfBle.charging       = i_ps4->status.charging;

	isBeforeConnectedBle = isConnectedBle;
	isConnectedBle = PS4.isConnected();
}
#endif


/**
 * @brief       JoyStick傾き取得
 * @note        なし
 * @param[in]   i_hStick : JoyStick水平位置
 * @param[in]   i_vStick : JoyStick垂直位置
 * @retval      JoyStick傾き量
 */
float JoyStick::GetJoyStickTilt( float i_hStick , float i_vStick )
{
	float tile;

	tile = 0.0;

	tile = std::pow( i_vStick , 2.0 ) + std::pow( i_hStick , 2.0 );
	if( tile != 0.0 )
	{
		tile = std::sqrt( tile );
	}

	return tile;
}


/**
 * @brief       JoyStick方向取得
 * @note        なし
 * @param[in]   i_hStick : JoyStick水平位置
 * @param[in]   i_vStick : JoyStick垂直位置
 * @retval      JoyStick方向
 */
JoyStickDirection JoyStick::GetJoyStickDirection( float i_hStick , float i_vStick )
{
	float tile;
	float angle;
	int32_t i;
	JoyStickDirectionTbl *stickDirTbl;
	JoyStickDirection stickDir;

	tile = 0.0;
	angle = 0.0;

	tile = GetJoyStickTilt( i_hStick , i_vStick );

	if( tile != 0.0 )
	{
		angle = std::atan2( i_vStick , i_hStick );
		angle = angle * 180.0 / ( atan(1.0) * 4.0 );	// rad2deg
	}

	stickDirTbl = (JoyStickDirectionTbl *)&JOYSTICKDIRECTION_BLE_TBL;

	stickDir = JOYSTKDIR_NONE;
	if( tile != 0.0 )
	{
		for( i = 0; i < JOYSTICKDIRECTIONTBL_NUM; i++ )
		{
			if( (angle > stickDirTbl->data[i].minAngle) && (angle <= stickDirTbl->data[i].maxAngle) )
			{
				stickDir = stickDirTbl->data[i].dir;
				break;
			}
		}
	}

	return stickDir;
}
