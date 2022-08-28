/**
 * @file NvmConfig.cpp
 * 
 * @brief       NvmConfig
 * @note        なし
 * 
 * @version     1.0.0
 * @date        2022/08/15
 * 
 * @copyright   (C) 2022 Motoyuki Endo
 */
#include "NvmConfig.h"


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
NvmConfig::NvmConfig( void )
{
	SPIFFS.begin();
	ReadConfig();
}


/**
 * @brief       デストラクタ
 * @note        なし
 * @param       なし
 * @retval      なし
 */
NvmConfig::~NvmConfig( void )
{
	// DO_NOTHING
}

/**
 * @brief       コンフィグ読み出し
 * @note        なし
 * @param       なし
 * @retval      なし
 */
void NvmConfig::ReadConfig( void )
{
	File fp = SPIFFS.open( "/config.txt", "r" );

	if( fp )
	{
		ssid = fp.readStringUntil('\n');
		ssid.trim();
		pass = fp.readStringUntil('\n');
		pass.trim();

		rosAgentIp = fp.readStringUntil('\n');
		rosAgentIp.trim();
		rosAgentPort = fp.readStringUntil('\n');
		rosAgentPort.trim();

		fp.close();
	}
}
