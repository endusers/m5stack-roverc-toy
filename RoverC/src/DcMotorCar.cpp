/**
 * @file DcMotorCar.cpp
 * 
 * @brief       DcMotorCar
 * @note        なし
 * 
 * @version     1.0.0
 * @date        2021/10/15
 * 
 * @copyright   (C) 2021 Motoyuki Endo
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

	_numWheel = Config.num;
	_speedMotor = new int32_t[_numWheel];

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
#endif

	M5.begin();
	M5.Imu.Init();

	_drvMotor.InitMotor();

	_reqMaxValue = 0;

	_isLcdUpdate = true;

	_joy.Init();
	_JoyCtrlCycle = 0;

	M5.Lcd.setRotation( 1 );
	M5.Lcd.setCursor( 0, 5 );
	M5.Lcd.println( " InitFinish." );
}


/**
 * @brief       メインループ
 * @note        なし
 * @param       なし
 * @retval      なし
 */
void DcMotorCar::MainLoop( void )
{
#ifdef _SERIAL_DEBUG_
	SerialDebug();
#endif
	if( _isLcdUpdate )
	{
		_isLcdUpdate = false;
		M5.Lcd.fillScreen( BLACK );
		M5.Lcd.setCursor( 0, 5 );

		if( _joy.isConnectedBle )
		{
			M5.Lcd.println( " PS4 Controller Connected." );
			M5.Lcd.printf( " Battery Level : %d\n", _joy.joyInfBle.battery );
			M5.Lcd.printf( " MaxSpeed : %d\n", _reqMaxValue );
			M5.Lcd.printf( "\n\n" );
		}
		else
		{
			M5.Lcd.println( " PS4 Controller Not Connected." );
			M5.Lcd.printf( "\n\n\n\n" );
		}

		if( !_joy.isConnectedBle )
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
}


/**
 * @brief       BleJoyStickコントロール周期
 * @note        なし
 * @param       なし
 * @retval      なし
 */
void DcMotorCar::BleJoyCtrlCycle( void )
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

		beforeMaxValue = _reqMaxValue;
		_joy.UpdateJoyStickInfoBle( &PS4.data );
		if( _joy.isConnectedBle )
		{
			JoyControl();
		}
		reqMaxValue = _reqMaxValue;

		if( _joy.isBeforeConnectedBle != _joy.isConnectedBle ){
			isUpdate = true;
		}
		if( _joy.beforeJoyInfBle.battery != _joy.joyInfBle.battery ){
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
	}
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
void DcMotorCar::JoyControl( void )
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

	joyInf = &_joy.joyInfBle;
	beforeJoyInf = &_joy.beforeJoyInfBle;

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

	lJoyTilt = _joy.GetJoyStickTilt( joyInf->lStickH , joyInf->lStickV );
	rJoyTilt = _joy.GetJoyStickTilt( joyInf->rStickH , joyInf->rStickV );

	if( lJoyTilt >= 0.2 )
	{
		lJoyDir = _joy.GetJoyStickDirection( joyInf->lStickH , joyInf->lStickV );
	}

	if( rJoyTilt >= 0.2 )
	{
		rJoyDir = _joy.GetJoyStickDirection( joyInf->rStickH , joyInf->rStickV );
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

		print_cycle = getTime + 10;
	}
}
#endif
