# m5stack-roverc-toy

## はじめに

---

本ソフトはM5StackのRoveCをPS4コントローラーで動かすソフトである

- Bluetooth接続の場合

  PS4コントローラをBluetoothで接続して動かす

- micro-ROS接続(WiFi)の場合

  microROSのJoyメッセージでRoveCを動かす  
  microROSのTwistメッセージでRoveCを動かす  
  RoveCからのIMUのデータをmicroROSのImuメッセージとして配信する  

## 動作環境

---

- RoverC-Pro
- M5StickC PLUS
- PS4 Controller
- Arduino IDE
- Visual Studio Code
- PlatformIO
- SixaxisPairTool
- ROS2 Foxy
- micro-ROS
- micro-ROS for Arduino

- Arduino Library
  - M5StickCPlus
  - PS4-esp32
  - micro_ros_arduino

- ROS2 Package
  - joy_linux
  - micro_ros_agent

## ビルド

---

- コンフィグ

  - Bluetooth接続の場合

    1. M5StickC PLUSのBluetoothのMACアドレスを確認しメモする

    1. メモしたMACアドレスを下記コンフィグファイルに記入する

        .\RoverC\src\JoyStick_Config.h
        ```c++
        #define BLUETOOTH_MAC_ADDRESS			"1A:2B:3C:01:01:01"
        ```

        ※上記 "1A:2B:3C:01:01:01" を書き換える

  - micro-ROS接続(WiFi)の場合

    1. WiFiの設定とmicro-ROS Agentの設定を下記コンフィグファイルに記入する

        1行目から下記順番で記入する
        - SSID
        - PASS
        - micro-ROS Agent IP
        - micro-ROS Agent Port

        .\RoverC\data\config.txt
        ```txt
        SSID
        PASS
        192.168.1.100
        8888
        ```

        ※WiFiの設定は、上記 SSID と PASS を書き換える  
        ※micro-ROS Agentの設定は、上記 192.168.1.100 と 8888 を書き換える  
        ※SSIDやPASSは暗号化をしていないため自己責任でご使用ください  

    1. PlatformIOでコンフィグファイルを書き込む

        PlatformIOの下記メニューを選択してファイルを書き込む

        ```txt
        PROJECT TASKS -> m5stack-c -> Platform -> Upload Filesystem Image
        ```

        ※M5StickC PLUSの不揮発メモリに格納されるため、本コンフィグが不要となった場合はすみやかに削除ください  

- ビルド

  1. パーティションをPlatformIOのConfiguration Fileで指定する

      .\RoverC\platformio.ini
      ```ini
      board_build.partitions = C:\User\usr\Arduino IDE\portable\packages\m5stack\hardware\esp32\2.0.4\tools\partitions\min_spiffs.csv
      ```

      ※上記 board_build.partitions で指定するパスを書き換える  

  1. 参照するライブラリのパスをPlatformIOのConfiguration Fileで指定する

      .\RoverC\platformio.ini
      ```ini
      lib_extra_dirs = 
      	C:\User\usr\Arduino IDE\portable\packages\m5stack
      	C:\User\usr\Arduino IDE\portable\sketchbook\libraries\M5StickCPlus
      	C:\User\usr\Arduino IDE\portable\sketchbook\libraries\PS4-esp32
      ```

      ※上記 lib_extra_dirs で指定するパスを書き換える  

  1. PlatformIOでソフトをビルドする

  1. PlatformIOでソフトをアップロードする

## 起動方法

---

  - Bluetooth接続の場合

    1. SixaxisPairToolでPS4コントローラにMasterのMACアドレスを書き込む

    1. M5StickC PLUSとRoverC-Proの電源を入れる

    1. PS4コントローラのPSボタンを押す

  - micro-ROS接続(WiFi)の場合

    1. PS4コントローラのPSボタンを押す

        ※事前にROS2ネットワーク内のPCにPS4コントローラをペアリングしていること

    1. Joyメッセージを配信するノードを起動する

        ```bash
        ros2 run joy_linux joy_linux_node --ros-args -p autorepeat_rate:=0.0 -p coalesce_interval:=0.1
        ```

        ※PS4コントローラをペアリングしたPCにて起動する  
        ※デフォルトの設定の場合配信周期が速すぎてM5StickC PLUSが処理しきれないため上記パラメータにて起動する  

    1. micro-ROS Agentを起動する

        ```bash
        ros2 run micro_ros_agent micro_ros_agent udp4 --port 8888
        ```

        ※ROS2ネットワーク内のPCにて起動する

    1. M5StickC PLUSとRoverC-Proの電源を入れる

## 操作方法

---

- アクセル

  - 最高速度ダウン : L1
  - 最高速度アップ : R1

  - 速度ダウン : stick tilt less
  - 速度アップ : stick tilt many

- 前進 : L stick ↑ & R stick ↑

  ```txt
    ↑A-----B↑
     |  ↑  |
     |  |  |
    ↑C-----D↑
  ```

- 後進 : L stick ↓ & R stick ↓

  ```txt
    ↓A-----B↓
     |  |  |
     |  ↓  |
    ↓C-----D↓
  ```

- 右進 : L stick → & R stick →

  ```txt
    ↑A-----B↓
     |  →  |
     |  →  |
    ↓C-----D↑
  ```

- 左進 : L stick ← & R stick ←

  ```txt
    ↓A-----B↑
     |  ←  |
     |  ←  |
    ↑C-----D↓
  ```

- 左斜前進 : L stick ↖ & R stick ↖

  ```txt
    =A-----B↑
     |   ↖ |
     | ↖   |
    ↑C-----D=
  ```

- 左斜後進 : L stick ↙ & R stick ↙

  ```txt
    ↓A-----B=
     | ↙   |
     |   ↙ |
    =C-----D↓
  ```

- 右斜前進 : L stick ↗ & R stick ↗

  ```txt
    ↑A-----B=
     | ↗   |
     |   ↗ |
    =C-----D↑
  ```

- 右斜後進 : L stick ↘ & R stick ↘

  ```txt
    =A-----B↓
     |   ↘ |
     | ↘   |
    ↓C-----D=
  ```

- 左回転 : L stick ↓ & R stick ↑

  ```txt
    ↓A-----B↑
     |  =  |
     |  =  |
    ↓C-----D↑
  ```

- 右回転 : L stick ↑ & R stick ↓

  ```txt
    ↑A-----B↓
     |  =  |
     |  =  |
    ↑C-----D↓
  ```

- 左前回転 : R stick ↑

  ```txt
    =A-----B↑
     |  |  |
     |  ↓  |
    =C-----D↑
  ```

- 左後回転 : R stick ↓

  ```txt
    =A-----B↓
     |  |  |
     |  ↓  |
    =C-----D↓
  ```

- 右前回転 : L stick ↑

  ```txt
    ↑A-----B=
     |  |  |
     |  ↓  |
    ↑C-----D=
  ```

- 右後回転 : L stick ↓

  ```txt
    ↓A-----B=
     |  |  |
     |  ↓  |
    ↓C-----D=
  ```

## 参考サイト

- [PS4-esp32](https://github.com/aed3/PS4-esp32)
- [M5StackにPS4コントローラをBluetooth接続](https://qiita.com/Geek493/items/8402ad875b88822e75ab)
- [micro-ROS](https://micro.ros.org/)
- [micro-ROS for Arduino](https://github.com/micro-ROS/micro_ros_arduino)
