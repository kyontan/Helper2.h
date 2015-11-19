// #include "Helper2_protected.h"
#include "Adafruit_NeoPixel.h"

#pragma once

extern Adafruit_NeoPixel AN_LED;

class LEDClass {
public:
  LEDClass(int8_t num):
    led_num_(num),
    status_(false)
  {
    rgb_[0] = 100;
    rgb_[1] = 100;
    rgb_[2] = 100;
    // SetRGBFromHLS();
  }
  // LEDClass(const LED& origin){
  //   
  // }
    
  // return led status
  bool on();
  bool off();
  bool getStatus();
  
  // led の色相を変更する
  // 色相: 0.0-1.0
  void color(double hue);
  void color(uint8_t red, uint8_t green, uint8_t blue);
  
  // led の明るさを変更する
  // 明るさ: 0.0-1.0
  void brightness(double brightness);
  
  // led の彩度を変更する
  // 彩度 0.0-1.0
  void saturation(double saturation);
  
  // led の色をランダムに点灯する
  void randomcolor();
  
private:
  // led status on = true, off = false
  bool status_;
  
  // led_num_ = {0, 1, }
  int8_t led_num_;
  
  // led color (default = white)
  uint8_t rgb_[3];
  double hsl_[3];
  
  void SetHLSFromRGB();
  void SetRGBFromHLS();
};

// class LED {
//   public:
//     byte x, y;
// 
//     // LED() { this->isFirst = true; };
// 
// 
//     // led の色相を変更する
//     // 色相: 0.0-1.0
//     void color(int8_t led, double hue);
// 
// 
//     void brightness(int8_t led, double brightness);
// 
//     // led の彩度を変更する
//     // 彩度 0.0-1.0
//     void saturation(int8_t led, double saturation);
// 
//     // led の色をランダムに点灯する
//     void randomcolor(int8_t led);
// 
//     // void debug_show();
// 
//     LED(const LED &other) {};
//   private:
//     byte no;
// };
