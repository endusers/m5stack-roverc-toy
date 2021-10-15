/**
 * @file DcMotorCar_Config.cpp
 * 
 * @brief       DcMotorCar_Config
 * @note        なし
 * 
 * @version     1.0.0
 * @date        2021/10/15
 * 
 * @copyright   (C) 2021 Motoyuki Endo
 */
#include "DcMotorCar_Config.h"


//----------------------------------------------------------------
//  <table>
//----------------------------------------------------------------
const DcMotorWheelConfigs Config =
{
//  num
    4 ,
//  portConfig
//  | pinNoSDA | pinNoSCL | addrI2C | headRegNum |
    { 0 ,        26,        0x38,     0x00       },
//  configs
    {
        // Config_FL
        {
        //  wheelDir
            DCMOTWHFWDDIR_CW ,
        //  smoothConfig
        //  | isSmoothEnable | coefficient |
            { true           , 0.2         } ,
        },
        // Config_FR
        {
        //  wheelDir
            DCMOTWHFWDDIR_CW ,
        //  smoothConfig
        //  | isSmoothEnable | coefficient |
            { true           , 0.2         } ,
        },
        // Config_RL
        {
        //  wheelDir
            DCMOTWHFWDDIR_CW ,
        //  smoothConfig
        //  | isSmoothEnable | coefficient |
            { true           , 0.2         } ,
        },
        // Config_RR
        {
        //  wheelDir
            DCMOTWHFWDDIR_CW ,
        //  smoothConfig
        //  | isSmoothEnable | coefficient |
            { true           , 0.2         } ,
        },
    } ,
};
