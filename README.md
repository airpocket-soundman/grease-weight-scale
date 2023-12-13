# grease-weight-scale
grease_weight_scale

# Hardware

## devices
M5Stack Basic
HX711
1kg loadcell

## housing
3D printed parts

## other parts

### stripbord
20x80mm(6x28hall) stripboard x 1pcs

### pinheaders
2x15 pinheader x 1


### connectors
XH2.54 6P male,female x 1set
XH2.54 4P mail,female x 1set


### screws
M5x15 x 2 ロードセル固定用
M4x15 x 2 plate固定用
m3x10 x 4 top固定用
m3x10 x 4 guide固定用
m2x10 x 4 基板固定用

# Software

Ver 2.0.2 UIFlow版最終。連続稼働時に停止する場合有り。
Ver 3.0.0 ArduinoIDE版へ移行

# schematic

## M5Stack to HX711
|M5Stack|HX711|
|-|-|
|5V|VCC|
|GND|GND|
|36|DT|
|26|SCK|

## HX711 to Loadcell
|HX711|Loadcell|
|-|-|
|E+|RED|
|E-|BLACK|
|A-|WHITE|
|A+|GREEN|