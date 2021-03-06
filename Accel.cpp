#include <Wire.h> // I2C用
#include "Accel.h"

// val の値を min と max の値に収まるようにする
// val < min         :=> min
// min <= val <= max :=> val
// max < val         :=> max
float Accel::clamp(float val, float min, float max) {
  if (val < min) { return min; }
  if (max < val) { return max; }

  return val;
}

float Accel::x() {
  return _x[_latest_frame];
};

float Accel::y() {
  return _y[_latest_frame];
};

float Accel::z() {
  return _z[_latest_frame];
};


bool Accel::active() {
  return _active;
};

bool Accel::freefall() {
  return _freefall;
};

bool Accel::tap() {
  return _tap;
};

bool Accel::doubletap() {
  return _doubletap;
};

void Accel::init() {
  adxl.powerOn();

  adxl.setRangeSetting(2); // 測定範囲 (何G まで測定するか)

  // 動作したかを監視する軸の設定 (1 == on; 0 == off)
  //各軸の判定の論理和
  adxl.setActivityX(0);
  adxl.setActivityY(0);
  adxl.setActivityZ(0);

  // 動作してないを監視する軸の設定 (1 == on; 0 == off)
  // 各軸の判定の論理積
  adxl.setInactivityX(0);
  adxl.setInactivityY(0);
  adxl.setInactivityZ(0);

  // タップされたことを検視する軸の設定 (1 == on; 0 == off)
  adxl.setTapDetectionOnX(0);
  adxl.setTapDetectionOnY(0);
  adxl.setTapDetectionOnZ(0);

  // setting all interupts to take place on int pin 1
  // I had issues with int pin 2, was unable to reset it
  adxl.setInterruptMapping(ADXL345_INT_SINGLE_TAP_BIT, ADXL345_INT1_PIN);
  adxl.setInterruptMapping(ADXL345_INT_DOUBLE_TAP_BIT, ADXL345_INT1_PIN);
  adxl.setInterruptMapping(ADXL345_INT_FREE_FALL_BIT,  ADXL345_INT1_PIN);
  adxl.setInterruptMapping(ADXL345_INT_ACTIVITY_BIT,   ADXL345_INT1_PIN);
  adxl.setInterruptMapping(ADXL345_INT_INACTIVITY_BIT, ADXL345_INT1_PIN);

  // register interupt actions - 1 == on; 0 == off
  adxl.setInterrupt(ADXL345_INT_SINGLE_TAP_BIT, 0);
  adxl.setInterrupt(ADXL345_INT_DOUBLE_TAP_BIT, 0);
  adxl.setInterrupt(ADXL345_INT_FREE_FALL_BIT,  0);
  adxl.setInterrupt(ADXL345_INT_ACTIVITY_BIT,   0);
  adxl.setInterrupt(ADXL345_INT_INACTIVITY_BIT, 0);
}

void Accel::updateAccel() {
  // count が大きくなるほどノイズに強くなる (が遅くなる)
  int sumx = 0, sumy = 0, sumz = 0;
  int rawx = 0, rawy = 0, rawz = 0;

  // 水準器
  for(int i=0; i<_COUNT; i++) {
    adxl.readAccel(&rawx, &rawy, &rawz);
    sumx += rawx;
    sumy += rawy;
    sumz += rawz;
  }

  float divider_x = 245.0 * _COUNT; // 1G で x が取る値
  float divider_y = 245.0 * _COUNT; // 1G で y が取る値
  float divider_z = 225.0 * _COUNT; // 1G で z が取る値

  // sum(x, y, z) は -10240 - 10240 をとる
  // sum_(x, y, z) の値を divider_(x, y, z) で割り、正規化して -1.0 - 1.0 に縮める
  // by kyontan
  // NOTE: 割る値 は適宜調整してください
  float x = clamp(sumx / divider_x, -1.0, 1.0);
  float y = clamp(sumy / divider_y, -1.0, 1.0);
  float z = clamp(sumz / divider_z, -1.0, 1.0);

  shiftValue(x, y, z);
}

void Accel::shiftValue(float nx, float ny, float nz) {
  // 位置をシフト
  _frame_a      = (_frame_a      + 1) % n_frames;
  _frame_b      = (_frame_b      + 1) % n_frames;
  _latest_frame = (_latest_frame + 1) % n_frames;

  // 最後位置は"最初位置-1"
  // 最初位置が0の時は、最後位置は"全フレーム数-1"
  _x[_latest_frame] = nx;
  _y[_latest_frame] = ny;
  _z[_latest_frame] = nz;
  _millis[_latest_frame] = millis();

  float new_size = nx*nx + ny*ny + nz*nz;
  _diff[_latest_frame] = abs(new_size - _last_size);
  _last_size = new_size;
}

void Accel::resetFlags() {
  _active = false;
  _freefall = false;
  _tap = false;
  _doubletap = false;
}

void Accel::updateFlags() {
  // 検出をゆるくするため、直近数フレームのどこかで動いていれば active と判断する
  if (0.1 < abs(_diff[_latest_frame] - _diff[(_latest_frame + n_frames - 1) % n_frames]) ||
      0.1 < abs(_diff[_latest_frame] - _diff[(_latest_frame + n_frames - 2) % n_frames]) ||
      0.1 < abs(_diff[_latest_frame] - _diff[(_latest_frame + n_frames - 3) % n_frames]) ||
      0.1 < abs(_diff[_latest_frame] - _diff[(_latest_frame + n_frames - 4) % n_frames])) {
    _active = true;

    if (debug){
      Serial.print("active: _latest_frame = ");
      Serial.print(_latest_frame);
      Serial.print("  ");
      Serial.print(_diff[_latest_frame]);
      Serial.print(", ");
      Serial.print(_diff[(_latest_frame - 1) % n_frames]);
      Serial.print(", ");
      Serial.print(_diff[(_latest_frame - 2) % n_frames]);
      Serial.print(", ");
      Serial.print(_diff[(_latest_frame - 3) % n_frames]);
      Serial.print(", ");
      Serial.println(_diff[(_latest_frame - 4) % n_frames]);
    }
  }

  if (_last_size < 0.2) {
    _freefall = true;
  }

  /**
  * 設定すべき値は 以下の8つです ([]内は単位)
  *
  * - ThFrameA [frame]
  * - ThFrameB [frame]
  *
  * - ThMaxAtFrameA [(m/s^2)^2]
  * - ThMinAtFrameB [(m/s^2)^2]
  * - ThMaxAtLatestFrame [(m/s^2)^2]
  *
  * - ThMaximumSingleTapSpace [ms]
  * - ThMaximumDoubleTapSpace [ms]
  *
  * 前提として、加速度 (x, y, z) をそれぞれn_frames回(フレーム)分保持する
  *
  * 変化量 = |現在のフレームの加速度|^2 - |直前フレームの加速度|^2 とする。
  * タップ検知: 以下の3条件を満たした時にタップ検知とする
  * - (ThFrameA フレーム前の変化量) < ThMaxAtFrameA
  * - ThMinAtFrameB < (ThFrameB での変化量)
  * - (最新フレームでの変化量) < ThMaxAtLatestFrame
  *
  * ダブルタップ検知:
  * - ThMaximumSingleTapSpace[ms] 以内にあったタップは 同タップとする (ダブルタップとは検知しない)
  * - ThMaximumSingleTapSpace[ms] 以上離れたタップを ダブルタップとする
  * - ThMaximumDoubleTapSpace[ms] 以上離れたタップは 異なるタップとする  (ダブルタップとは検知しない)
  */
  // (_frame_a+(int)(n_frames/2))%30   (最初+10) % 30 が FrameB
  if (_diff[_frame_a] < _ThMaxAtFrameA && _ThMinAtFrameB < _diff[_frame_b] && _diff[_latest_frame] < _ThMaxAtLatestFrame) {
    long int t_diff = millis() - _t_lasttap;

    if (_ThMaximumSingleTapSpace < t_diff) {
      _tap = true;

      if (t_diff < _ThMaximumDoubleTapSpace) {
        _doubletap = true;
      }

      _t_lasttap = millis();
    }

    if (debug) {
      Serial.print("time diff: ");
      Serial.print(t_diff);
      Serial.print("  ");

      if (_doubletap) {
        Serial.print("doubletapped, _diff[_frame_b] = ");
        Serial.println(_diff[_frame_b], 2);
      } else if (_tap) {
        Serial.print("tapped, _diff[_frame_b] = ");
        Serial.println(_diff[_frame_b], 2);
      } else {
        Serial.println("ignored");
      }
    }
  }
}

void Accel::debugPrint(int i) {
  if (i = -1){
    i = _latest_frame;
  }

  Serial.print("(");
  Serial.print(_x[i], 2);
  Serial.print(", ");
  Serial.print(_y[i], 2);
  Serial.print(", ");
  Serial.print(_z[i], 2);
  Serial.print(", ");
  Serial.print(_diff[i], 2);
  Serial.println(")");
}

void Accel::debugPrintThreshold() {
  Serial.print("_ThMaxAtFrameA: ");
  Serial.print(_ThMaxAtFrameA);
  Serial.print(", _ThMinAtFrameB: ");
  Serial.print(_ThMinAtFrameB);
  Serial.print(", _ThMaxAtLatestFrame: ");
  Serial.print(_ThMaxAtLatestFrame);
  Serial.print(", _ThMaximumSingleTapSpace: ");
  Serial.print(_ThMaximumSingleTapSpace);
  Serial.print(", _ThMaximumDoubleTapSpace: ");
  Serial.println(_ThMaximumDoubleTapSpace);
}

void Accel::debugInputThreshold() {
  if (0 < Serial.available()) {
    uint8_t j = 1, len = Serial.available();
    int8_t p[5] = {};
    char str[len+5];

    for (uint8_t i = 0; i < len; i++) {
      str[i] = Serial.read();
      if ((str[i] < '0' || '9' < str[i]) && str[i] != '.') {
        str[i] = '\0';
        if (0 < i && str[i-1] == '\0') {
          j--;
        }

        p[j] = i + 1;
        j++;
      }
    }

    if (j == 5) {
      _ThMaxAtFrameA = atof(&(str[p[0]]));
      _ThMinAtFrameB = atof(&(str[p[1]]));
      _ThMaxAtLatestFrame = atof(&(str[p[2]]));
      _ThMaximumSingleTapSpace = atol(&(str[p[3]]));
      _ThMaximumDoubleTapSpace = atol(&(str[p[4]]));

      debugPrintThreshold();
    }
  }
}

