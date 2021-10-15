# m5stack-roverc-toy

## はじめに

---

本ソフトはM5StackのRoveCをPS4コントローラーで動かすソフトである

## 動作環境

---

- RoverC-Pro
- M5StickC PLUS
- PS4 Controller
- Arduino IDE
- Visual Studio Code
- PlatformIO
- SixaxisPairTool

- Arduino Library
  - M5StickCPlus
  - PS4-esp32

## ビルド

---

- コンフィグ

  1. M5StickC PLUSのBluetoothのMACアドレスを確認しメモする

  1. メモしたMACアドレスを下記コンフィグファイルに記入する

      .\RoverC\src\JoyStick_Config.h
      ```c++
      #define BLUETOOTH_MAC_ADDRESS			"1A:2B:3C:01:01:01"
      ```

      ※上記 "1A:2B:3C:01:01:01" を書き換える

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

  1. SixaxisPairToolでPS4コントローラにMasterのMACアドレスを書き込む

  1. M5StickC PLUSとRoverC-Proの電源を入れる

  1. PS4コントローラのPSボタンを押す

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
