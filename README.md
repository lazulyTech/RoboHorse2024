# 機馬 ドゥラズリ

情報はこれから追加していきます

## レースの様子
- [予選](https://youtu.be/tr46X8UxwqI) 2着
- [敗者復活戦](https://youtu.be/tr46X8UxwqI?t=78) 1着
- [決勝戦](https://youtu.be/tr46X8UxwqI?t=132) 1着

## マイコン
- ロボット側: Seeed Studio XIAO ESP32C3
- コントローラ側: M5StickC Plus

## 準備
1. Controller/Addr.h、Robot/Addr.hの追加
    下のような形式でそれぞれのAddr.hを追加してください。
```c
#ifndef ADDR_H
#define ADDR_H

// Controller/Addr.hであれば、ロボット側のSeeed Studio XIAO ESP32C3のBluetooth MACアドレス
// Robot/Addr.hであれば、コントローラ側のM5StickC PlusのBluetooth MACアドレス
uint8_t macAddr[1][6] = {{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}};

#endif //ADDR_H
```

2. Arduino IDEでコンパイル・書き込み

## 回路

