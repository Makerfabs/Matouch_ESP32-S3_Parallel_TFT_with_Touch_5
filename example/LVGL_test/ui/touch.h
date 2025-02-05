#include <Wire.h>
#include "TAMC_GT911.h"

#define TOUCH_GT911
#define TOUCH_GT911_SCL 18
#define TOUCH_GT911_SDA 17
#define TOUCH_GT911_INT -1
#define TOUCH_GT911_RST 38
#define TOUCH_GT911_ROTATION ROTATION_NORMAL

TAMC_GT911 ts = TAMC_GT911(TOUCH_GT911_SDA, TOUCH_GT911_SCL, TOUCH_GT911_INT, TOUCH_GT911_RST, 800, 480);

void touch_init()
{
  pinMode(TOUCH_GT911_RST, OUTPUT);
  digitalWrite(TOUCH_GT911_RST, LOW);
  delay(500);
  digitalWrite(TOUCH_GT911_RST, HIGH);
  delay(500);

  ts.begin();
  ts.setRotation(ROTATION_NORMAL);
}

void TouchonInterrupt(void)
{
  ts.isTouched = true;
}

