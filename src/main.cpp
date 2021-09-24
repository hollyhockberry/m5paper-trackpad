// Copyright (c) 2021 Inaba
// This software is released under the MIT License.
// http://opensource.org/licenses/mit-license.php

#include <M5EPD.h>
#include <BleMouse.h>

namespace {

BleMouse bleMouse;
int fingers = 0;
int point[2] = {0};
unsigned long pressed_time;

bool isShortTap() {
  return (::millis() - pressed_time) <= 200;
}
}  // namespace

void setup() {
  M5.begin();
  M5.TP.SetRotation(180);
  bleMouse.begin();
}

void loop() {
  if (!M5.TP.avaliable()) {
    return;
  }

  if (M5.TP.isFingerUp()) {
    if (fingers != 0) {
      if (isShortTap()) {
        bleMouse.click(fingers == 1 ? MOUSE_LEFT : MOUSE_RIGHT);
      }
      fingers = 0;
    }
  } else {
    const tp_finger_t item = M5.TP.readFinger(0);
    if (fingers != M5.TP.getFingerNum()) {
      if (fingers == 0) {
        pressed_time = ::millis();
      }
      fingers = M5.TP.getFingerNum();
    } else if (bleMouse.isConnected()) {
      const int dx = item.x - point[0];
      const int dy = item.y - point[1];
      if (dx != 0 || dy != 0) {
        if (fingers == 1) {
          bleMouse.move(dx, dy, 0);
        } else if (fingers == 2) {
          bleMouse.move(0, 0, -dy/20, dx/20);
        }
      }
    }
    point[0] = item.x;
    point[1] = item.y;
  }
  M5.TP.update();
}
