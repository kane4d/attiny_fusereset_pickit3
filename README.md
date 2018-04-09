# attiny_fusereset_pickit3
Microchip PICkit3でattiny85のfuseをリセット(low:0x62, high:0xdf)するCLIアプリケーションです。

未確認ですが
* attiny24
* attiny25
* attiny44
* attiny45
* attiny84

PICKit3のファームウエアにはMPLAB IPEで使うファームウエアではなくて、PICKit2互換モードで動作するMicrochip PICkit3 with scripting firmwareを利用します。

ファームウエア、ソース、説明書は[PICkit™ 3 Programming App and Scripting Tool](http://ww1.microchip.com/downloads/en/DeviceDoc/PICkit3%20Programmer%20Application%20v3.10.zip "download from archive area")をダウンロードしてください。

![pk3gui3](https://github.com/kane4d/attiny_fusereset_pickit3/blob/master/images/pk3gui2.png?raw=true)

## HVISP pin接続
|pickit3|attiny85|
|:------|:-------|
|1.Vpp/MCLR +12V|1.Rst|
|2.Vdd +5V|8.Vdd|
|3.Gnd|4.Gnd|
|4.PGD|6.SII/MISO|
|5.PGC|5.SDI/MOSI|
|6.AUX|2.SCI/CLK|

PICKit3ではI/Oとして利用できるPinが3本しかないので、attty85のSDO(Pin7)は未使用です。

## 書き込み動作
下記の順番で動作します。
1. PICKit3 Status LED(RED) On
1. Vdd on(+5V)
1. Vpp on(+12V)
1. Low fuse write 0x62
1. High fuse write 0xdf
1. Low fuse read
1. High fuse read
1. Vpp off(0V)
1. Vdd off(0V)
1. PICKit3 Status LED(RED) Off

## ご注意
電圧が印加中はStatus LEDが点灯するようにプログラムしています。Status LEDを確認してからチップの取り外しをしてください。

PICKit2互換モードでは一部説明書と違う仕様の部分があります。
特にVddとVppの電圧設定は全く別物です。

## 参照
* [hidapi](https://github.com/signal11/hidapi)
* avrdude6.3/pickit2.c
* avrdude6.3/pickit2.h
* PICkit2SourceGuidePCv2-61FWv2-32.pdf PICKit2互換モード説明書
* pickit3_gui/PICkit3 Programmer Firmware Source v2.00.05/PICkit3OS/pk3_scripting.c
