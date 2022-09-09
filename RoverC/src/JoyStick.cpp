/**
 * @file JoyStick.cpp
 * 
 * @brief       JoyStick
 * @note        なし
 * 
 * @version     1.1.0
 * @date        2022/09/09
 * 
 * @copyright   (C) 2021-2022 Motoyuki Endo
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
const JoyStickDirectionTbl JoyStick::JOYSTICKDIRECTION_BT_TBL =
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

const JoyStickDirectionTbl JoyStick::JOYSTICKDIRECTION_ROS2_TBL =
{
    {
        { JOYSTKDIR_LEFT      ,  -30.0 ,   30.0 } ,
        { JOYSTKDIR_UPLEFT    ,   30.0 ,   60.0 } ,
        { JOYSTKDIR_UP        ,   60.0 ,  120.0 } ,
        { JOYSTKDIR_UPRIGHT   ,  120.0 ,  150.0 } ,
        { JOYSTKDIR_RIGHT     ,  150.0 ,  180.0 } ,
        { JOYSTKDIR_DOWNLEFT  ,  -60.0 ,  -30.0 } ,
        { JOYSTKDIR_DOWN      , -120.0 ,  -60.0 } ,
        { JOYSTKDIR_DOWNRIGHT , -150.0 , -120.0 } ,
        { JOYSTKDIR_RIGHT     , -180.0 , -150.0 } ,
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
	isConnectedBt = false;
	isBeforeConnectedBt = false;
	memset( &joyInfBt , 0 , sizeof(joyInfBt) );
	memset( &beforeJoyInfBt , 0 , sizeof(beforeJoyInfBt) );
	memset( &joyInfRos2 , 0 , sizeof(joyInfRos2) );
	memset( &beforeJoyInfRos2 , 0 , sizeof(beforeJoyInfRos2) );
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
	// char bt_mac_addr[17 + 1];

	// esp_read_mac( derived_mac_addr, ESP_MAC_BT );
	// snprintf( bt_mac_addr, sizeof(bt_mac_addr), "%02X:%02X:%02X:%02X:%02X:%02X",
	// 	derived_mac_addr[0], derived_mac_addr[1], derived_mac_addr[2],
	// 	derived_mac_addr[3], derived_mac_addr[4], derived_mac_addr[5] );

	// PS4.begin( bt_mac_addr );

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
void JoyStick::UpdateJoyStickInfoBt( ps4_t *i_ps4 )
{
	beforeJoyInfBt = joyInfBt;

	joyInfBt.lStickH        = MAPF( i_ps4->analog.stick.lx, -128.0, 127.0, -1.0, 1.0 );
	joyInfBt.lStickV        = MAPF( i_ps4->analog.stick.ly, -128.0, 127.0, -1.0, 1.0 );
	joyInfBt.rStickH        = MAPF( i_ps4->analog.stick.rx, -128.0, 127.0, -1.0, 1.0 );
	joyInfBt.rStickV        = MAPF( i_ps4->analog.stick.ry, -128.0, 127.0, -1.0, 1.0 );
	joyInfBt.l2Axes         = MAPF( i_ps4->analog.button.l2, 0.0, 255.0, 0.0, 1.0 );
	joyInfBt.r2Axes         = MAPF( i_ps4->analog.button.r2, 0.0, 255.0, 0.0, 1.0 );
	joyInfBt.lrAxes         = (i_ps4->button.left) + (-i_ps4->button.right);
	joyInfBt.updownAxes     = (i_ps4->button.up) + (-i_ps4->button.down);
	joyInfBt.squareButton   = i_ps4->button.square;
	joyInfBt.crossButton    = i_ps4->button.cross;
	joyInfBt.circleButton   = i_ps4->button.circle;
	joyInfBt.triangleButton = i_ps4->button.triangle;
	joyInfBt.l1Button       = i_ps4->button.l1;
	joyInfBt.r1Button       = i_ps4->button.r1;
	joyInfBt.battery        = i_ps4->status.battery;
	joyInfBt.charging       = i_ps4->status.charging;

	isBeforeConnectedBt = isConnectedBt;
	isConnectedBt = PS4.isConnected();
}
#endif


/**
 * @brief       JoyStickInfo更新
 * @note        なし
 * @param[in]   i_msg : joyメッセージ
 * @retval      なし
 */
#if JOYSTICK_ROS2_TYPE == JOYSTICK_ROS2_SUPPORT
void JoyStick::UpdateJoyStickInfoRos2( sensor_msgs__msg__Joy *i_msg )
{
	beforeJoyInfRos2 = joyInfRos2;

	joyInfRos2.lStickH        = i_msg->axes.data[0];
	joyInfRos2.lStickV        = i_msg->axes.data[1];
	joyInfRos2.rStickH        = i_msg->axes.data[3];
	joyInfRos2.rStickV        = i_msg->axes.data[4];
	joyInfRos2.l2Axes         = MAPF( -i_msg->axes.data[2], -1.0, 1.0, 0.0, 1.0 );
	joyInfRos2.r2Axes         = MAPF( -i_msg->axes.data[5], -1.0, 1.0, 0.0, 1.0 );
	joyInfRos2.lrAxes         = i_msg->axes.data[6];
	joyInfRos2.updownAxes     = i_msg->axes.data[7];
	joyInfRos2.squareButton   = i_msg->buttons.data[3];
	joyInfRos2.crossButton    = i_msg->buttons.data[0];
	joyInfRos2.circleButton   = i_msg->buttons.data[1];
	joyInfRos2.triangleButton = i_msg->buttons.data[2];
	joyInfRos2.l1Button       = i_msg->buttons.data[4];
	joyInfRos2.r1Button       = i_msg->buttons.data[5];
	joyInfRos2.battery         = 0;
	joyInfRos2.charging        = 0;
}
#endif


/**
 * @brief       JoyStick傾き取得
 * @note        なし
 * @param[in]   i_hStick : JoyStick水平位置
 * @param[in]   i_vStick : JoyStick垂直位置
 * @retval      JoyStick傾き量
 */
float JoyStick::GetJoyStickTilt( JoyStickConnectType i_type, float i_hStick , float i_vStick )
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
JoyStickDirection JoyStick::GetJoyStickDirection( JoyStickConnectType i_type, float i_hStick , float i_vStick )
{
	float tile;
	float angle;
	int32_t i;
	JoyStickDirectionTbl *stickDirTbl;
	JoyStickDirection stickDir;

	tile = 0.0;
	angle = 0.0;

	tile = GetJoyStickTilt( i_type , i_hStick , i_vStick );

	if( tile != 0.0 )
	{
		angle = std::atan2( i_vStick , i_hStick );
		angle = angle * 180.0 / ( atan(1.0) * 4.0 );	// rad2deg
	}

	if( i_type == JOYSTKCONTYPE_BT )
	{
		stickDirTbl = (JoyStickDirectionTbl *)&JOYSTICKDIRECTION_BT_TBL;
	}
	else
	{
		stickDirTbl = (JoyStickDirectionTbl *)&JOYSTICKDIRECTION_ROS2_TBL;
	}

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
