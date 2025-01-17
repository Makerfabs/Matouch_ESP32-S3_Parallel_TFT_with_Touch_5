/*
Arduino IDE v2.3.4
ESP32 Board v2.0.16
Arduino_GFX_Library v.1.3.1
TAMC_GT911 v1.0.2

Tools:
PSRAM:"OPI PSRAM"
*/

#include <Arduino_GFX_Library.h>
#include "TAMC_GT911.h"


#define GFX_BL 10
const int freq = 50000;
const int channel = 0;
const int resolution = 8;

//Touch
#define TOUCH_SDA 17
#define TOUCH_SCL 18
#define TOUCH_INT -1
#define TOUCH_RST 38
#define TOUCH_WIDTH 800
#define TOUCH_HEIGHT 480

TAMC_GT911 tp = TAMC_GT911(TOUCH_SDA, TOUCH_SCL, TOUCH_INT, TOUCH_RST, TOUCH_WIDTH, TOUCH_HEIGHT);

void TouchonInterrupt(void)
{
  tp.isTouched = true;
}

// int ColorArray[]={BLACK,NAVY,DARKGREEN,DARKCYAN,MAROON,PURPLE,OLIVE,LIGHTGREY,DARKGREY,BLUE,GREEN,CYAN,MAGENTA,YELLOW,WHITE,ORANGE,GREENYELLOW,PINK};
int ColorArray[] = {BLACK, BLUE, GREEN, WHITE, RED, ORANGE};

Arduino_ESP32RGBPanel *bus = new Arduino_ESP32RGBPanel(
    GFX_NOT_DEFINED /* CS */, GFX_NOT_DEFINED /* SCK */, GFX_NOT_DEFINED /* SDA */,
    40 /* DE */, 41 /* VSYNC */, 39 /* HSYNC */, 42 /* PCLK */,
    45 /* R0 */, 48 /* R1 */, 47 /* R2 */, 21 /* R3 */, 14 /* R4 */,
    5 /* G0 */, 6 /* G1 */, 7 /* G2 */, 15 /* G3 */, 16 /* G4 */, 4 /* G5 */,
    8 /* B0 */, 3 /* B1 */, 46 /* B2 */, 9 /* B3 */, 1 /* B4 */
);

Arduino_RPi_DPI_RGBPanel *gfx = new Arduino_RPi_DPI_RGBPanel(
   bus,
   800 /* width */, 0 /* hsync_polarity */, 8 /* hsync_front_porch */, 4 /* hsync_pulse_width */, 8 /* hsync_back_porch */,
   480 /* height */, 0 /* vsync_polarity */, 8 /* vsync_front_porch */, 4 /* vsync_pulse_width */, 8 /* vsync_back_porch */,
   1 /* pclk_active_neg */, 16000000 /* prefer_speed */, true /* auto_flush */);


void setup()
{
  Serial.begin(115200);

  gfx->begin();
  pinMode(GFX_BL, OUTPUT);
  digitalWrite(GFX_BL, LOW);

  ledcSetup(channel, freq, resolution);
  ledcAttachPin(GFX_BL, channel);

  for(int i=250;i>0;i--)//0-250--->亮--暗
  {
    ledcWrite(channel, i);
    delay(8);
  }

  touch_init();

  gfx->fillScreen(WHITE);
  delay(1000);
  gfx->fillScreen(GREEN);
  delay(1000);
  gfx->fillScreen(BLACK);
  delay(1000);
  gfx->fillScreen(BLUE);
  delay(1000);
  gfx->fillScreen(RED);
  delay(1000);
  gfx->setTextSize(8);
  gfx->setTextColor(WHITE);
  gfx->setCursor(100, 150);
  gfx->print("Touch me!!!");
  Serial.println("setup done");

  while(1)
  {
      tp.read();
      if (tp.isTouched)
      {
          for (int i = 0; i < tp.touches; i++)
          {
            // gfx->fillScreen(WHITE);
            int temp = random(0, 6);
            gfx->fillScreen(ColorArray[temp]);

            gfx->setTextSize(4);
            if (temp == 4)
              gfx->setTextColor(WHITE);
            else
              gfx->setTextColor(RED);
              gfx->setCursor(320, 400);
              gfx->print("X: ");
              gfx->println(String(tp.points[i].x));
              gfx->setCursor(320, 440);
              gfx->print("Y: ");
              gfx->println(String(tp.points[i].y));

              Serial.print("Touch ");
              Serial.print(i + 1);
              Serial.print(": ");

              Serial.print("  x: ");
              Serial.print(tp.points[i].x);
              Serial.print("  y: ");
              Serial.print(tp.points[i].y);
              Serial.print("  size: ");
              Serial.println(tp.points[i].size);
              Serial.println(' ');
          }
          tp.isTouched = false;
          if (tp.touches > 2)
              break;
      }
      delay(100);
  }
}

void loop(void)
{
}

void touch_init()
{
  pinMode(TOUCH_RST, OUTPUT);
  digitalWrite(TOUCH_RST, LOW);
  delay(500);
  digitalWrite(TOUCH_RST, HIGH);
  delay(500);
  tp.begin();
  tp.setRotation(ROTATION_NORMAL);
}