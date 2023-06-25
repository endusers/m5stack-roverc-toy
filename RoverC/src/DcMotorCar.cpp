/**
 * @file DcMotorCar.cpp
 * 
 * @brief       DcMotorCar
 * @note        なし
 * 
 * @version     1.1.1
 * @date        2023/06/25
 * 
 * @copyright   (C) 2021-2023 Motoyuki Endo
 */
#include "DcMotorCar.h"


//----------------------------------------------------------------
//  <macro>
//----------------------------------------------------------------
#define ARRAY_LENGTH(in_array)      (sizeof(in_array)/sizeof(in_array[0]))
#define UNUSED_VARIABLE(in_x)       (void)(in_x)

#define SAFE_DELETE(x) if(x){ delete(x); x = NULL;}           //delete開放処理
#define SAFE_DELETE_ARRAY(x) if(x){ delete [] (x); x = NULL;} //delete[]開放処理

#define OFF                         (0)
#define ON                          (1)

#define RCLRETUNUSED(ret) (void)(ret)
#define RCLRETCHECK(ret) if( ret != RCL_RET_OK ){ return false; }

#define ROS_INFO(...)                                                            \
do{                                                                              \
    rcl_ret_t ret;                                                               \
    snprintf( _logMsg.msg.data, _logMsg.msg.capacity, __VA_ARGS__ );             \
    _logMsg.msg.size = strlen(_logMsg.msg.data);                                 \
    ret = rcl_publish( &_pubLog, &_logMsg, NULL );                               \
    RCLRETUNUSED( ret );                                                         \
}while(0)                                                                        \


//----------------------------------------------------------------
//  <function>
//----------------------------------------------------------------
/**
 * @brief       コンストラクタ
 * @note        なし
 * @param       なし
 * @retval      なし
 */
DcMotorCar::DcMotorCar( void )
	: _flWheel( Config.configs[0] ) , _frWheel( Config.configs[1] ) , _rlWheel( Config.configs[2] ) , _rrWheel( Config.configs[3] )
	, _drvMotor( Config.portConfig )
{
	_mutex = xSemaphoreCreateMutex();
	xSemaphoreGive( _mutex );

	_mutex_joy = xSemaphoreCreateMutex();
	xSemaphoreGive( _mutex_joy );

	_mutex_ros = xSemaphoreCreateMutex();
	xSemaphoreGive( _mutex_ros );

	_numWheel = Config.num;
	_speedMotor = new int32_t[_numWheel];

	_imuInfPubCycle = 0;
	_imuMsg.linear_acceleration.x = 0.0;
	_imuMsg.linear_acceleration.y = 0.0;
	_imuMsg.linear_acceleration.z = 0.0;
	_imuMsg.angular_velocity.x = 0.0;
	_imuMsg.angular_velocity.y = 0.0;
	_imuMsg.angular_velocity.z = 0.0;

	_logMsg.stamp.sec = 0;
	_logMsg.stamp.nanosec = 0;
	_logMsg.level = rcl_interfaces__msg__Log__INFO;
	_logMsg.name.capacity = strlen(DCMOTORCAR_NODE_NAME);
	_logMsg.name.data = (char *)DCMOTORCAR_NODE_NAME;
	_logMsg.name.size = strlen(_logMsg.name.data);
	_logMsg.msg.capacity = 128;
	_logMsg.msg.data = new char[_logMsg.msg.capacity];
	_logMsg.msg.size = 0;
	_logMsg.file.capacity = strlen("");
	_logMsg.file.data = (char *)"";
	_logMsg.file.size = strlen(_logMsg.file.data);
	_logMsg.function.capacity = strlen("");
	_logMsg.function.data = (char *)"";
	_logMsg.function.size = strlen(_logMsg.function.data);
	_logMsg.line = (uint32_t)NULL;

	_joyMsg.axes.capacity = 8;
	_joyMsg.axes.data = new float[_joyMsg.axes.capacity];
	_joyMsg.axes.size = _joyMsg.axes.capacity;
	_joyMsg.buttons.capacity = 13;
	_joyMsg.buttons.data = new int32_t[_joyMsg.buttons.capacity];
	_joyMsg.buttons.size = _joyMsg.buttons.capacity;

	_twistMsg.linear.x = 0.0;
	_twistMsg.linear.y = 0.0;
	_twistMsg.linear.z = 0.0;
	_twistMsg.angular.x = 0.0;
	_twistMsg.angular.y = 0.0;
	_twistMsg.angular.z = 0.0;

	SetMotorSpeed();
}


/**
 * @brief       デストラクタ
 * @note        なし
 * @param       なし
 * @retval      なし
 */
DcMotorCar::~DcMotorCar( void )
{
	SAFE_DELETE_ARRAY( _speedMotor );
	SAFE_DELETE_ARRAY( _logMsg.msg.data );
	SAFE_DELETE_ARRAY( _joyMsg.axes.data );
	SAFE_DELETE_ARRAY( _joyMsg.buttons.data );
}


/**
 * @brief       イニシャライズ
 * @note        なし
 * @param       なし
 * @retval      なし
 */
void DcMotorCar::Init( void )
{
#ifdef _SERIAL_DEBUG_
	Serial.begin( 115200 );
	_ctrlCycleTime = (uint32_t)millis();
	_btCycleTime = (uint32_t)millis();
	_rosCycleTime = (uint32_t)millis();
	_ctrlCycleCnt = 0;
	_btCycleCnt = 0;
	_rosCycleCnt = 0;
#endif

	M5.begin();
	M5.Imu.Init();

	_drvMotor.InitMotor();

	_reqMaxValue = 0;

	_isLcdUpdate = true;

	_joy.Init();
	_JoyCtrlCycle = 0;

	_wifiRetryTime = (uint32_t)millis();

	_rosConState = ROS_CNST_WIFI_DISCONNECTED;
	_rosAgentPingCnt = 0;
	_rosMgrCtrlCycle = (uint32_t)millis();
	_imuInfPubCycle = (uint32_t)millis();

	M5.Lcd.setRotation( 1 );
	M5.Lcd.setCursor( 0, 5 );
	M5.Lcd.println( " InitFinish." );
}


/**
 * @brief       WiFiイニシャライズ
 * @note        なし
 * @param       なし
 * @retval      なし
 */
void DcMotorCar::WiFiInit( void )
{
	NvmConfig nvm;

	WiFi.begin( (char *)nvm.ssid.c_str(), (char *)nvm.pass.c_str() );
}


/**
 * @brief       Rosイニシャライズ
 * @note        なし
 * @param       なし
 * @retval      なし
 */
void DcMotorCar::RosInit( void )
{
#if ROS_AGENT_COMMODE == ROS_AGENT_COMMODE_UDP
	NvmConfig nvm;

	_locator.address.fromString( (char *)nvm.rosAgentIp.c_str() );
	_locator.port = atoi((char *)nvm.rosAgentPort.c_str());

	rmw_uros_set_custom_transport(
		false,
		(void *) &_locator,
		arduino_wifi_transport_open,
		arduino_wifi_transport_close,
		arduino_wifi_transport_write,
		arduino_wifi_transport_read
	);
#endif
#if ROS_AGENT_COMMODE == ROS_AGENT_COMMODE_SERIAL
	set_microros_transports();
#endif
}


/**
 * @brief       Rosエンティティ生成
 * @note        なし
 * @param       なし
 * @retval      true 正常終了
 * @retval      false 異常終了
 */
boolean DcMotorCar::RosCreateEntities( void )
{
	rcl_ret_t ret;

	_imuInfPubCycle = (uint32_t)millis();

	_allocator = rcl_get_default_allocator();
	ret = rclc_support_init( &_support, 0, NULL, &_allocator );
	RCLRETCHECK( ret );

	ret = rclc_node_init_default( &_node, (const char *)DCMOTORCAR_NODE_NAME, "", &_support );
	RCLRETCHECK( ret );

	ret = rclc_publisher_init_best_effort(
		&_pubLog,
		&_node,
		ROSIDL_GET_MSG_TYPE_SUPPORT( rcl_interfaces, msg, Log ),
		"rosout" );
	RCLRETCHECK( ret );

	ret = rclc_publisher_init_best_effort(
		&_pubImu,
		&_node,
		ROSIDL_GET_MSG_TYPE_SUPPORT( sensor_msgs, msg, Imu ),
		"roverc_imu" );
	RCLRETCHECK( ret );

#if JOYSTICK_ROS2_TYPE == JOYSTICK_ROS2_SUPPORT
	ret = rclc_subscription_init_default(
		&_subJoy,
		&_node,
		ROSIDL_GET_MSG_TYPE_SUPPORT( sensor_msgs, msg, Joy ),
		"joy" );
	RCLRETCHECK( ret );
#endif

	ret = rclc_subscription_init_default(
		&_subTwist,
		&_node,
		ROSIDL_GET_MSG_TYPE_SUPPORT( geometry_msgs, msg, Twist ),
		"cmd_vel" );
	RCLRETCHECK( ret );

	ret = rclc_executor_init( &_executor, &_support.context, 2, &_allocator );
	RCLRETCHECK( ret );

#if JOYSTICK_ROS2_TYPE == JOYSTICK_ROS2_SUPPORT
	ret = rclc_executor_add_subscription_with_context(
		&_executor,
		&_subJoy,
		&_joyMsg,
		&DcMotorCar::SubscribeJoyCbkWrap, this,
		ON_NEW_DATA );
	RCLRETCHECK( ret );
#endif

	ret = rclc_executor_add_subscription_with_context(
		&_executor,
		&_subTwist,
		&_twistMsg,
		&DcMotorCar::SubscribeTwistCbkWrap, this,
		ON_NEW_DATA );
	RCLRETCHECK( ret );

	return true;
}


/**
 * @brief       Rosエンティティ破棄
 * @note        なし
 * @param       なし
 * @retval      なし
 */
void DcMotorCar::RosDestroyEntities( void )
{
	rcl_ret_t ret;
	rmw_context_t *rmw_context;

	rmw_context = rcl_context_get_rmw_context( &_support.context );
	(void)rmw_uros_set_context_entity_destroy_session_timeout( rmw_context, 0 );

	ret = rcl_publisher_fini( &_pubLog, &_node );
	RCLRETUNUSED( ret );
	ret = rcl_publisher_fini( &_pubImu, &_node );
	RCLRETUNUSED( ret );
#if JOYSTICK_ROS2_TYPE == JOYSTICK_ROS2_SUPPORT
	ret = rcl_subscription_fini( &_subJoy, &_node );
	RCLRETUNUSED( ret );
#endif
	ret = rcl_subscription_fini( &_subTwist, &_node );
	RCLRETUNUSED( ret );
	ret = rclc_executor_fini( &_executor );
	RCLRETUNUSED( ret );
	ret = rcl_node_fini( &_node );
	RCLRETUNUSED( ret );
	ret = rclc_support_fini( &_support );
	RCLRETUNUSED( ret );
}


/**
 * @brief       メインループ
 * @note        なし
 * @param       なし
 * @retval      なし
 */
void DcMotorCar::MainLoop( void )
{
	boolean isStop;
#ifdef _SERIAL_DEBUG_
	SerialDebug();
#endif
	if( _isLcdUpdate )
	{
		_isLcdUpdate = false;
		M5.Lcd.fillScreen( BLACK );
		M5.Lcd.setCursor( 0, 5 );

		if( _joy.isConnectedBt )
		{
			M5.Lcd.println( " PS4 Controller Connected." );
			M5.Lcd.printf( " Battery Level : %d\n", _joy.joyInfBt.battery );
			M5.Lcd.printf( " MaxSpeed : %d\n", _reqMaxValue );
			M5.Lcd.printf( "\n\n" );
		}
		else
		{
			M5.Lcd.println( " PS4 Controller Not Connected." );
			M5.Lcd.printf( "\n\n\n\n" );
		}

		if( _rosConState == ROS_CNST_AGENT_CONNECTED )
		{
			M5.Lcd.println( " micro-ROS Agent Connected." );
		}
		else
		{
			M5.Lcd.println( " micro-ROS Agent Not Connected." );
		}

		isStop = true;

		if( _joy.isConnectedBt )
		{
			isStop = false;
		}
		if( _rosConState == ROS_CNST_AGENT_CONNECTED )
		{
			isStop = false;
		}

		if( isStop )
		{
			xSemaphoreTake( _mutex , portMAX_DELAY );
			_flWheel.StopWheel();
			_frWheel.StopWheel();
			_rlWheel.StopWheel();
			_rrWheel.StopWheel();
			xSemaphoreGive( _mutex );
		}
	}
}


/**
 * @brief       制御周期ハンドラ
 * @note        なし
 * @param       なし
 * @retval      なし
 */
void DcMotorCar::MainCycle( void )
{
	xSemaphoreTake( _mutex , portMAX_DELAY );
	_flWheel.ControlCycle();
	_frWheel.ControlCycle();
	_rlWheel.ControlCycle();
	_rrWheel.ControlCycle();
	xSemaphoreGive( _mutex );
	SetMotorSpeed();
	_drvMotor.RunMotor( _numWheel, &_speedMotor[0] );

#ifdef _SERIAL_DEBUG_
	{	// DEBUG
		uint32_t time;
		time = (uint32_t)millis();
		_ctrlCycleCnt = time - _ctrlCycleTime;
		_ctrlCycleTime = time;
	}
#endif
}


/**
 * @brief       JoyStickコントロール周期
 * @note        なし
 * @param       なし
 * @retval      なし
 */
#if JOYSTICK_BLUETOOTH_TYPE == JOYSTICK_BLUETOOTH_SUPPORT
void DcMotorCar::BtJoyCtrlCycle( void )
{
	boolean isUpdate;
	int32_t beforeMaxValue;
	uint32_t getTime;
	uint32_t reqMaxValue;

	isUpdate = false;
	beforeMaxValue = 0;
	reqMaxValue = 0;

	getTime = (uint32_t)millis();

	if( getTime > _JoyCtrlCycle )
	{
		_JoyCtrlCycle = getTime + DCMOTORCAR_JOYCTRL_CYCLE;

		xSemaphoreTake( _mutex_joy , portMAX_DELAY );
		beforeMaxValue = _reqMaxValue;
		_joy.UpdateJoyStickInfoBt( &PS4.data );
		if( _joy.isConnectedBt )
		{
			JoyControl( JOYSTKCONTYPE_BT );
		}
		reqMaxValue = _reqMaxValue;
		xSemaphoreGive( _mutex_joy );

		if( _joy.isBeforeConnectedBt != _joy.isConnectedBt ){
			isUpdate = true;
		}
		if( _joy.beforeJoyInfBt.battery != _joy.joyInfBt.battery ){
			isUpdate = true;
		}
		if( beforeMaxValue != reqMaxValue )
		{
			isUpdate = true;
		}

		if( isUpdate )
		{
			_isLcdUpdate = isUpdate;
		}

#ifdef _SERIAL_DEBUG_
		{	// DEBUG
			uint32_t time;
			time = (uint32_t)millis();
			_btCycleCnt = time - _btCycleTime;
			_btCycleTime = time;
		}
#endif
	}
}
#endif


/**
 * @brief       ROSコントロール周期
 * @note        なし
 * @param       なし
 * @retval      なし
 */
void DcMotorCar::RosCtrlCycle( void )
{
	xSemaphoreTake( _mutex_ros , portMAX_DELAY );
	if( _rosConState == ROS_CNST_AGENT_CONNECTED  )
	{
		PublishImuInfo();
		rclc_executor_spin_some( &_executor, RCL_MS_TO_NS(10) );
	}
	xSemaphoreGive( _mutex_ros );

#ifdef _SERIAL_DEBUG_
	{	// DEBUG
		uint32_t time;
		time = (uint32_t)millis();
		_rosCycleCnt = time - _rosCycleTime;
		_rosCycleTime = time;
	}
#endif
}


/**
 * @brief       ROS管理コントール周期
 * @note        なし
 * @param       なし
 * @retval      なし
 */
void DcMotorCar::RosMgrCtrlCycle( void )
{
	uint32_t getTime;
	RosConnectionState beforeState;

	getTime = (uint32_t)millis();

	if( getTime > _rosMgrCtrlCycle )
	{
		_rosMgrCtrlCycle = getTime + DCMOTORCAR_ROSMGRCTRL_CYCLE;
		beforeState = _rosConState;

		switch( _rosConState )
		{
			case ROS_CNST_WIFI_DISCONNECTED :
#if DCMOTORCAR_JOYCONNECT_MODE == DCMOTORCAR_JOYCONNECT_PREFERBLUETOOTH
				if( !_joy.isConnectedBt )
				{
#endif
#if ROS_AGENT_COMMODE == ROS_AGENT_COMMODE_UDP
					WiFiInit();
					_wifiRetryTime = (uint32_t)millis() + 3000;
					_rosConState = ROS_CNST_WAITING_WIFI;
#endif
#if ROS_AGENT_COMMODE == ROS_AGENT_COMMODE_SERIAL
					_rosConState = ROS_CNST_WIFI_CONNECTED;
#endif
#if DCMOTORCAR_JOYCONNECT_MODE == DCMOTORCAR_JOYCONNECT_PREFERBLUETOOTH
				}
#endif
				break;
			case ROS_CNST_WAITING_WIFI :
#if ROS_AGENT_COMMODE == ROS_AGENT_COMMODE_UDP
				if( WiFi.status() == WL_CONNECTED )
				{
					_rosConState = ROS_CNST_WIFI_CONNECTED;
				}
				else
				{
					if( (uint32_t)millis() > _wifiRetryTime )
					{
						WiFi.disconnect( true, true );
						_rosConState = ROS_CNST_WIFI_DISCONNECTED;
					}
				}
#endif
				break;
			case ROS_CNST_WIFI_CONNECTED :
				RosInit();
				_rosConState = ROS_CNST_WAITING_AGENT;
				break;
			case ROS_CNST_WAITING_AGENT :
				if( rmw_uros_ping_agent(ROS_AGENT_PING_TIMEOUT, 10) == RMW_RET_OK )
				{
					_rosAgentPingCnt = 0;
					_rosConState = ROS_CNST_AGENT_AVAILABLE;
				}
				else{
#if ROS_AGENT_COMMODE == ROS_AGENT_COMMODE_UDP
					if( WiFi.status() != WL_CONNECTED )
					{
						WiFi.disconnect( true, true );
						_rosConState = ROS_CNST_WIFI_DISCONNECTED;
					}
#endif
#if DCMOTORCAR_JOYCONNECT_MODE == DCMOTORCAR_JOYCONNECT_PREFERBLUETOOTH
					if( _joy.isConnectedBt )
					{
#if ROS_AGENT_COMMODE == ROS_AGENT_COMMODE_UDP
						WiFi.disconnect( true, true );
#endif
						_rosConState = ROS_CNST_WIFI_DISCONNECTED;
					}
#endif
				}
				break;
			case ROS_CNST_AGENT_AVAILABLE :
				if( RosCreateEntities() )
				{
					_rosConState = ROS_CNST_AGENT_CONNECTED;
				}
				else
				{
					_rosConState = ROS_CNST_AGENT_DISCONNECTED;
				}
				break;
			case ROS_CNST_AGENT_CONNECTED :
				if( rmw_uros_ping_agent(ROS_AGENT_PING_TIMEOUT, 3) != RMW_RET_OK )
				{
					_rosAgentPingCnt++;
					if( _rosAgentPingCnt >= ROS_AGENT_PING_RETRY_CNTMAX )
					{
						_rosConState = ROS_CNST_AGENT_DISCONNECTED;
					}
				}
				else
				{
					xSemaphoreTake( _mutex_ros , portMAX_DELAY );
#if ROS_AGENT_COMMODE == ROS_AGENT_COMMODE_UDP
					if( WiFi.status() != WL_CONNECTED )
					{
						RosDestroyEntities();
						WiFi.disconnect( true, true );
						_rosConState = ROS_CNST_WIFI_DISCONNECTED;
					}
#endif
#if DCMOTORCAR_JOYCONNECT_MODE == DCMOTORCAR_JOYCONNECT_PREFERBLUETOOTH
					if( _joy.isConnectedBt )
					{
						RosDestroyEntities();
#if ROS_AGENT_COMMODE == ROS_AGENT_COMMODE_UDP
						WiFi.disconnect( true, true );
#endif
						_rosConState = ROS_CNST_WIFI_DISCONNECTED;
					}
#endif
					xSemaphoreGive( _mutex_ros );
					_rosAgentPingCnt = 0;
				}
				break;
			case ROS_CNST_AGENT_DISCONNECTED :
				RosDestroyEntities();
				_rosConState = ROS_CNST_WAITING_AGENT;
				break;
			default :
				// DO_NOTHING
				break;
		}
		if( beforeState != _rosConState )
		{
			if( _rosConState == ROS_CNST_AGENT_CONNECTED )
			{
				_isLcdUpdate = true;
			}
			if( beforeState == ROS_CNST_AGENT_CONNECTED )
			{
				_isLcdUpdate = true;
			}
		}
	}
}


/**
 * @brief       IMUセンサ情報配信
 * @note        なし
 * @param       なし
 * @retval      なし
 */
void DcMotorCar::PublishImuInfo( void )
{
	rcl_ret_t ret;
	uint32_t getTime;
	float accX = 0.0;
	float accY = 0.0;
	float accZ = 0.0;
	float gyroX = 0.0;
	float gyroY = 0.0;
	float gyroZ = 0.0;

	getTime = (uint32_t)millis();

	if( getTime > _imuInfPubCycle )
	{
		M5.IMU.getGyroData( &gyroX, &gyroY, &gyroZ );
		M5.IMU.getAccelData( &accX, &accY, &accZ );

		_imuMsg.linear_acceleration.x = accX * 9.8;	// G_TO_M/S2
		_imuMsg.linear_acceleration.y = accY * 9.8;	// G_TO_M/S2
		_imuMsg.linear_acceleration.z = accZ * 9.8;	// G_TO_M/S2
		_imuMsg.angular_velocity.x = gyroX *  ( M_PI / 180.0 );	// DEG_TO_RAD
		_imuMsg.angular_velocity.y = gyroY *  ( M_PI / 180.0 );	// DEG_TO_RAD
		_imuMsg.angular_velocity.z = gyroZ *  ( M_PI / 180.0 );	// DEG_TO_RAD

		ret = rcl_publish( &_pubImu, &_imuMsg, NULL );
		RCLRETUNUSED( ret );

		_imuInfPubCycle = getTime + DCMOTORCAR_IMUINF_SENDCYCLE;
	}
}


/**
 * @brief       Joy情報購読ハンドラ
 * @note        なし
 * @param[in]   arg : Joy情報
 * @param[in]   obj : コールバックのthisポインタ
 * @retval      なし
 */
#if JOYSTICK_ROS2_TYPE == JOYSTICK_ROS2_SUPPORT
void DcMotorCar::SubscribeJoyCbkWrap( const void *arg, void *obj )
{
	return reinterpret_cast<DcMotorCar*>(obj)->SubscribeJoyCbk(arg);
}
#endif


/**
 * @brief       Joy情報購読
 * @note        なし
 * @param[in]   msgin : Joy情報
 * @retval      なし
 */
#if JOYSTICK_ROS2_TYPE == JOYSTICK_ROS2_SUPPORT
void DcMotorCar::SubscribeJoyCbk( const void *msgin )
{
	int32_t beforeMaxValue;
	uint32_t reqMaxValue;

	xSemaphoreTake( _mutex_joy , portMAX_DELAY );
	beforeMaxValue = _reqMaxValue;
	_joy.UpdateJoyStickInfoRos2( (sensor_msgs__msg__Joy *)msgin );
	JoyControl( JOYSTKCONTYPE_ROS2 );
	reqMaxValue = _reqMaxValue;
	xSemaphoreGive( _mutex_joy );

	if( beforeMaxValue != reqMaxValue )
	{
		ROS_INFO( "MaxSpeed : %i" , reqMaxValue );
	}
}
#endif


/**
 * @brief       Twist情報購読ハンドラ
 * @note        なし
 * @param[in]   arg : Twist情報
 * @param[in]   obj : コールバックのthisポインタ
 * @retval      なし
 */
void DcMotorCar::SubscribeTwistCbkWrap( const void *arg, void *obj )
{
	return reinterpret_cast<DcMotorCar*>(obj)->SubscribeTwistCbk(arg);
}


/**
 * @brief       Twist情報購読
 * @note        なし
 * @param[in]   msgin : Twist情報
 * @retval      なし
 */
void DcMotorCar::SubscribeTwistCbk( const void *msgin )
{
	const geometry_msgs__msg__Twist *msg = (const geometry_msgs__msg__Twist *)msgin;

	float lx;
	float ly;
	float k;
	float x;
	float y;
	float z;
	float w1;
	float w2;
	float w3;
	float w4;
	int32_t flSpeed;
	int32_t frSpeed;
	int32_t rlSpeed;
	int32_t rrSpeed;

	lx = 0.0215;
	ly = 0.0215;

	x = msg->linear.x;
	y = msg->linear.y;
	z = msg->angular.z;

	lx = lx * 15.0;                                 // tentative
	ly = ly * 15.0;                                 // tentative
	x = x * 50.0;                                   // tentative
	y = y * 50.0;                                   // tentative
	z = z * 100.0;                                  // tentative

	k = lx + ly;

	w1 = x - y + z * (-k);
	w2 = x + y + z * k;  
	w3 = x + y + z * (-k);
	w4 = x - y + z * k; 

	w1 = constrain( w1 , -100.0 , 100.0 );
	w2 = constrain( w2 , -100.0 , 100.0 );
	w3 = constrain( w3 , -100.0 , 100.0 );
	w4 = constrain( w4 , -100.0 , 100.0 );

	flSpeed = (int32_t)round( w1 );
	frSpeed = (int32_t)round( w2 );
	rlSpeed = (int32_t)round( w3 );
	rrSpeed = (int32_t)round( w4 );

	xSemaphoreTake( _mutex , portMAX_DELAY );
	_flWheel.RollWheel( flSpeed );
	_frWheel.RollWheel( frSpeed );
	_rlWheel.RollWheel( rlSpeed );
	_rrWheel.RollWheel( rrSpeed );
	xSemaphoreGive( _mutex );
}


/**
 * @brief       モーター速度設定
 * @note        なし
 * @param       なし
 * @retval      なし
 */
void DcMotorCar::SetMotorSpeed( void )
{
	xSemaphoreTake( _mutex , portMAX_DELAY );
	_speedMotor[0] = _flWheel.outSpeed;
	_speedMotor[1] = _frWheel.outSpeed;
	_speedMotor[2] = _rlWheel.outSpeed;
	_speedMotor[3] = _rrWheel.outSpeed;
	xSemaphoreGive( _mutex );
}


/**
 * @brief       JoyStickコントロール
 * @note        なし
 * @param       なし
 * @retval      なし
 */
void DcMotorCar::JoyControl( JoyStickConnectType i_type )
{
	JoyInfo *joyInf;
	JoyInfo *beforeJoyInf;
	JoyStickDirection lJoyDir;
	JoyStickDirection rJoyDir;
	float lJoyTilt;
	float rJoyTilt;
	int32_t flSpeed;
	int32_t frSpeed;
	int32_t rlSpeed;
	int32_t rrSpeed;
	int32_t lSpeed;
	int32_t rSpeed;

	if( i_type == JOYSTKCONTYPE_BT )
	{
		joyInf = &_joy.joyInfBt;
		beforeJoyInf = &_joy.beforeJoyInfBt;
	}
	else
	{
		joyInf = &_joy.joyInfRos2;
		beforeJoyInf = &_joy.beforeJoyInfRos2;
	}
	lJoyDir = JOYSTKDIR_NONE;
	rJoyDir = JOYSTKDIR_NONE;
	lJoyTilt = 0.0;
	rJoyTilt = 0.0;
	flSpeed = 0;
	frSpeed = 0;
	rlSpeed = 0;
	rrSpeed = 0;
	lSpeed = 0;
	rSpeed = 0;

	if( beforeJoyInf->l1Button != joyInf->l1Button )
	{
		if( joyInf->l1Button == ON )
		{
			_reqMaxValue = _reqMaxValue - 10;
		}
	}

	if( beforeJoyInf->r1Button != joyInf->r1Button )
	{
		if( joyInf->r1Button == ON )
		{
			_reqMaxValue = _reqMaxValue + 10;
		}
	}

	_reqMaxValue = constrain( _reqMaxValue , -0 , 100 );

	lJoyTilt = _joy.GetJoyStickTilt( i_type , joyInf->lStickH , joyInf->lStickV );
	rJoyTilt = _joy.GetJoyStickTilt( i_type , joyInf->rStickH , joyInf->rStickV );

	if( lJoyTilt >= 0.2 )
	{
		lJoyDir = _joy.GetJoyStickDirection( i_type , joyInf->lStickH , joyInf->lStickV );
	}

	if( rJoyTilt >= 0.2 )
	{
		rJoyDir = _joy.GetJoyStickDirection( i_type , joyInf->rStickH , joyInf->rStickV );
	}

	lSpeed = (int32_t)round( _reqMaxValue * lJoyTilt );
	rSpeed = (int32_t)round( _reqMaxValue * rJoyTilt );

	// 前進
	if( (lJoyDir == JOYSTKDIR_UP) && (rJoyDir == JOYSTKDIR_UP) )
	{
		flSpeed = lSpeed;
		frSpeed = rSpeed;
		rlSpeed = lSpeed;
		rrSpeed = rSpeed;
	}
	// 後進
	else if( (lJoyDir == JOYSTKDIR_DOWN) && (rJoyDir == JOYSTKDIR_DOWN) )
	{
		flSpeed = -lSpeed;
		frSpeed = -rSpeed;
		rlSpeed = -lSpeed;
		rrSpeed = -rSpeed;
	}
	// 左進
	else if( (lJoyDir == JOYSTKDIR_LEFT) && (rJoyDir == JOYSTKDIR_LEFT) )
	{
		flSpeed = -lSpeed;
		frSpeed = rSpeed;
		rlSpeed = lSpeed;
		rrSpeed = -rSpeed;
	}
	// 右進
	else if( (lJoyDir == JOYSTKDIR_RIGHT) && (rJoyDir == JOYSTKDIR_RIGHT) )
	{
		flSpeed = lSpeed;
		frSpeed = -rSpeed;
		rlSpeed = -lSpeed;
		rrSpeed = rSpeed;
	}
	// 左斜前進
	else if( (lJoyDir == JOYSTKDIR_UPLEFT) && (rJoyDir == JOYSTKDIR_UPLEFT) )
	{
		flSpeed = 0;
		frSpeed = rSpeed;
		rlSpeed = lSpeed;
		rrSpeed = 0;
	}
	// 右斜前進
	else if( (lJoyDir == JOYSTKDIR_UPRIGHT) && (rJoyDir == JOYSTKDIR_UPRIGHT) )
	{
		flSpeed = lSpeed;
		frSpeed = 0;
		rlSpeed = 0;
		rrSpeed = rSpeed;
	}
	// 左斜後進
	else if( (lJoyDir == JOYSTKDIR_DOWNLEFT) && (rJoyDir == JOYSTKDIR_DOWNLEFT) )
	{
		flSpeed = -lSpeed;
		frSpeed = 0;
		rlSpeed = 0;
		rrSpeed = -rSpeed;
	}
	// 右斜後進
	else if( (lJoyDir == JOYSTKDIR_DOWNRIGHT) && (rJoyDir == JOYSTKDIR_DOWNRIGHT) )
	{
		flSpeed = 0;
		frSpeed = -rSpeed;
		rlSpeed = -lSpeed;
		rrSpeed = 0;
	}
	// 左旋回
	else if( (lJoyDir == JOYSTKDIR_DOWN) && (rJoyDir == JOYSTKDIR_UP) )
	{
		flSpeed = -lSpeed;
		frSpeed = rSpeed;
		rlSpeed = -lSpeed;
		rrSpeed = rSpeed;
	}
	// 右旋回
	else if( (lJoyDir == JOYSTKDIR_UP) && (rJoyDir == JOYSTKDIR_DOWN) )
	{
		flSpeed = lSpeed;
		frSpeed = -rSpeed;
		rlSpeed = lSpeed;
		rrSpeed = -rSpeed;
	}
	// 右前回転
	else if( (lJoyDir == JOYSTKDIR_NONE) && (rJoyDir == JOYSTKDIR_UP) )
	{
		flSpeed = 0;
		frSpeed = rSpeed;
		rlSpeed = 0;
		rrSpeed = rSpeed;
	}
	// 右後回転
	else if( (lJoyDir == JOYSTKDIR_NONE) && (rJoyDir == JOYSTKDIR_DOWN) )
	{
		flSpeed = 0;
		frSpeed = -rSpeed;
		rlSpeed = 0;
		rrSpeed = -rSpeed;
	}
	// 左前回転
	else if( (lJoyDir == JOYSTKDIR_UP) && (rJoyDir == JOYSTKDIR_NONE) )
	{
		flSpeed = lSpeed;
		frSpeed = 0;
		rlSpeed = lSpeed;
		rrSpeed = 0;
	}
	// 左後回転
	else if( (lJoyDir == JOYSTKDIR_DOWN) && (rJoyDir == JOYSTKDIR_NONE) )
	{
		flSpeed = -lSpeed;
		frSpeed = 0;
		rlSpeed = -lSpeed;
		rrSpeed = 0;
	}
	// 方向指定無
	else if( (lJoyDir == JOYSTKDIR_NONE) && (rJoyDir == JOYSTKDIR_NONE) )
	{
		flSpeed = 0;
		frSpeed = 0;
		rlSpeed = 0;
		rrSpeed = 0;
	}
	// 方向未確定
	else
	{
		// DO_NOTHING
	}

	xSemaphoreTake( _mutex , portMAX_DELAY );
	_flWheel.RollWheel( flSpeed );
	_frWheel.RollWheel( frSpeed );
	_rlWheel.RollWheel( rlSpeed );
	_rrWheel.RollWheel( rrSpeed );
	xSemaphoreGive( _mutex );
}


#ifdef _SERIAL_DEBUG_
/**
 * @brief       シリアルデバッグ
 * @note        なし
 * @param       なし
 * @retval      なし
 */
void DcMotorCar::SerialDebug( void )
{
	static uint32_t print_cycle = 0;
	uint32_t getTime;

	getTime = (uint32_t)millis();

	if( getTime > print_cycle )
	{
		// Serial.print(_flWheel.reqSpeed);
		// Serial.print(",");
		// Serial.print(_flWheel.targetSpeed);
		// Serial.print(",");
		// Serial.print(_frWheel.reqSpeed);
		// Serial.print(",");
		// Serial.print(_rlWheel.reqSpeed);
		// Serial.print(",");
		// Serial.print(_rrWheel.reqSpeed);
		// Serial.print(",");
		// Serial.print("\n");

		// Serial.print(esp_get_free_heap_size());
		// Serial.print(",");
		// Serial.print(_rosAgentPingCnt);
		// Serial.print(",");
		// Serial.print(_rosConState);
		// Serial.print(",");
		// Serial.print(_ctrlCycleTime);
		// Serial.print(",");
		// Serial.print(_btCycleTime);
		// Serial.print(",");
		// Serial.print(_rosCycleTime);
		// Serial.print(",");
		// Serial.print(_ctrlCycleCnt);
		// Serial.print(",");
		// Serial.print(_btCycleCnt);
		// Serial.print(",");
		// Serial.print(_rosCycleCnt);
		// Serial.print("\n");

		print_cycle = getTime + 10;
	}
}
#endif
