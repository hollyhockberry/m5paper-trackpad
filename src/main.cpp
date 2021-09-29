// Copyright (c) 2021 Inaba
// This software is released under the MIT License.
// http://opensource.org/licenses/mit-license.php

#include <M5EPD.h>
#include <BleMouse.h>
#include <driver/adc.h>

enum MODE {
  NONE, SWIPE, TAP, DRUG, DTAP,
};

BleMouse bleMouse;

bool down;
MODE mode;

int lastFingers;
int pos[2] = {0};
int eventIndex;
u_long events[4];

void pushTouchEvent(bool down) {
  if (!down && (eventIndex <= 0)) {
    return;
  }
  if (down && (eventIndex >= 4)) {
    eventIndex = 0;
  }
  events[eventIndex++] = ::millis();
}

MODE judgeMode() {
  if (eventIndex <= 0) {
    return NONE;
  }
  const u_long now = ::millis();
  const u_long last = events[eventIndex - 1];
  if ((now - last) < 150) {
    return NONE;
  }
  const MODE m = static_cast<MODE>(eventIndex);
  eventIndex = 0;
  return m;
}

void setup() {
  M5.begin();
  M5.TP.SetRotation(180);
  // To avoid noise on GPIO36
  ::adc_power_acquire();
  bleMouse.begin();

  mode = NONE;
  down = false;
  eventIndex = 0;
  lastFingers = 0;
}

void loop() {
  if (!M5.TP.avaliable()) {
    if (mode == NONE) {
      mode = judgeMode();
      if (bleMouse.isConnected()) {
        if (mode == DRUG) {
          bleMouse.press();
        } else if (mode == TAP) {
          bleMouse.click(lastFingers == 1 ? MOUSE_LEFT : MOUSE_RIGHT);
          mode = NONE;
        } else if (mode == DTAP) {
          bleMouse.click();
          bleMouse.click();
          mode = NONE;
        }
      }
    }
    return;
  }

  M5.TP.update();
  if (!M5.TP.isFingerUp()) {
    const tp_finger_t finger = M5.TP.readFinger(0);
    lastFingers = M5.TP.getFingerNum();
    if (!down) {
      pushTouchEvent(true);
      down = true;
    } else if (bleMouse.isConnected()) {
      const int dx = finger.x - pos[0];
      const int dy = finger.y - pos[1];
      if (dx != 0 && dy != 0) {
        if (lastFingers == 1) {
          bleMouse.move(dx, dy);
        } else {
          bleMouse.move(0, 0, -dy / 20, dx / 20);
        }
      }
    }
    pos[0] = finger.x;
    pos[1] = finger.y;
  } else if (down) {
    pushTouchEvent(false);
    mode = NONE;
    down = false;
    if (bleMouse.isConnected()) {
      bleMouse.release();
    }
  }
}
