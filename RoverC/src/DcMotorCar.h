/**
 * @file DcMotorCar.h
 * 
 * @brief       DcMotorCar
 * @note        なし
 * 
 * @version     1.1.0
 * @date        2022/09/09
 * 
 * @copyright   (C) 2021-2022 Motoyuki Endo
 */
#ifndef __DCMOTORCAR_H
#define __DCMOTORCAR_H

#include <Arduino.h>
#include <M5StickCPlus.h>
#include <micro_ros_arduino.h>
#include <stdio.h>
#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <rcl_interfaces/msg/log.h>
#include <sensor_msgs/msg/imu.h>
#include <sensor_msgs/msg/joy.h>
#include <geometry_msgs/msg/twist.h>
#include <rmw_microros/rmw_microros.h>
#include "NvmConfig.h"
#include "DcMotor.h"
#include "DcMotorWheel.h"
#include "DcMotorCar_Config.h"
#include "JoyStick.h"


//----------------------------------------------------------------
//  <macro>
//----------------------------------------------------------------
#define DCMOTORCAR_MAIN_CYCLE					(10 * 1000)						// 10ms
#define DCMOTORCAR_JOYCTRL_CYCLE				(100)							// 100ms
#define DCMOTORCAR_ROSMGRCTRL_CYCLE				(1000)							// 1000ms
#define DCMOTORCAR_IMUINF_SENDCYCLE				(100)							// 100ms

#define DCMOTORCAR_NODE_NAME					"micro_ros_roverc_node"

#define ROS_AGENT_PING_TIMEOUT					(50)							// 50ms
#define ROS_AGENT_PING_RETRY_CNTMAX				(10)							// 10count


//----------------------------------------------------------------
//  <enum>
//----------------------------------------------------------------
enum RosConnectionState_Tag
{
	ROS_CNST_WAITING_AGENT				= 0 ,
	ROS_CNST_AGENT_AVAILABLE				,
	ROS_CNST_AGENT_CONNECTED				,
	ROS_CNST_AGENT_DISCONNECTED				,
};
typedef enum RosConnectionState_Tag			RosConnectionState;


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
	SemaphoreHandle_t _mutex_joy;
	SemaphoreHandle_t _mutex_ros;

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

	rcl_allocator_t _allocator;
	rclc_support_t _support;
	rcl_node_t _node;
	rclc_executor_t _executor;
	rcl_publisher_t _pubImu;
	rcl_publisher_t _pubLog;
	rcl_subscription_t _subJoy;
	rcl_subscription_t _subTwist;

	rcl_interfaces__msg__Log _logMsg;
	sensor_msgs__msg__Imu _imuMsg;
	sensor_msgs__msg__Joy _joyMsg;
	geometry_msgs__msg__Twist _twistMsg;

	RosConnectionState _rosConState;
	uint32_t _rosAgentPingCnt;
	uint32_t _rosMgrCtrlCycle;
	uint32_t _imuInfPubCycle;

	void PublishImuInfo( void );                                        // IMUセンサ情報配信
#if JOYSTICK_ROS2_TYPE == JOYSTICK_ROS2_SUPPORT
	static void SubscribeJoyCbkWrap( const void *arg, void *obj );      // Joy情報購読ハンドラ
	void SubscribeJoyCbk( const void *msgin );                          // Joy情報購読
#endif
	static void SubscribeTwistCbkWrap( const void *arg, void *obj );    // Twist情報購読ハンドラ
	void SubscribeTwistCbk( const void *msgin );                        // Twist情報購読
	void SetMotorSpeed( void );                                         // モーター速度設定
	void JoyControl( JoyStickConnectType i_type );                      // JoyStickコントロール
#ifdef _SERIAL_DEBUG_
	void SerialDebug( void );                                           // シリアルデバッグ
#endif

public:
	DcMotorCar( void );                                                 // コンストラクタ
	~DcMotorCar( void );                                                // デストラクタ

	void Init( void );                                                  // イニシャライズ
	void RosInit( void );                                               // Rosイニシャライズ
	boolean RosCreateEntities( void );                                  // Rosエンティティ生成
	void RosDestroyEntities( void );                                    // Rosエンティティ破棄
	void MainLoop( void );                                              // メインループ
	void MainCycle( void );                                             // 制御周期ハンドラ
#if JOYSTICK_BLUETOOTH_TYPE == JOYSTICK_BLUETOOTH_SUPPORT
	void BtJoyCtrlCycle( void );                                        // JoyStickコントロール周期
#endif
	void RosCtrlCycle( void );                                          // ROSコントロール周期
	void RosMgrCtrlCycle( void );                                       // ROS管理コントロール周期
};

#endif
