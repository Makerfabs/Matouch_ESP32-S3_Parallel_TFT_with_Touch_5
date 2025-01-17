/*
Arduino IDE 2.3.4
ESP32 Board v2.0.16
GFX Library for Arduino v1.3.1
JPEGDEC v1.2.7
TAMC_GT911 v1.0.2

Tools:
PSRAM:"OPI PSRAM"
*/

#include <Arduino_GFX_Library.h>
#include <SD_MMC.h>
#include "TAMC_GT911.h"
#include <Wire.h>
#include "JpegFunc.h"

#define TFT_BL 10

#define I2S_DOUT 19
#define I2S_BCLK 20
#define I2S_LRC 2

// microSD card
#define PIN_SD_CMD 11
#define PIN_SD_CLK 12
#define PIN_SD_D0 13

int i=0;

//Touch
#define TOUCH_SDA 17
#define TOUCH_SCL 18
#define TOUCH_INT -1
#define TOUCH_RST 38
#define TOUCH_WIDTH 800
#define TOUCH_HEIGHT 480
#define DEBOUNCE_TIME 50 // 防抖时间
volatile unsigned long lastInterruptTime = 0; // 上次中断触发时间

TAMC_GT911 tp = TAMC_GT911(TOUCH_SDA, TOUCH_SCL, TOUCH_INT, TOUCH_RST, TOUCH_WIDTH, TOUCH_HEIGHT);

void TouchonInterrupt(void)
{
  unsigned long currentTime = millis(); // 获取当前时间

  // 防抖
  if (currentTime - lastInterruptTime > DEBOUNCE_TIME) {
    tp.isTouched = true;
    lastInterruptTime = currentTime; // 更新中断触发时间
  }
}

Arduino_ESP32RGBPanel *bus = new Arduino_ESP32RGBPanel(
    GFX_NOT_DEFINED /* CS */, GFX_NOT_DEFINED /* SCK */, GFX_NOT_DEFINED /* SDA */,
    40 /* DE */, 41 /* VSYNC */, 39 /* HSYNC */, 42 /* PCLK */,
    45 /* R0 */, 48 /* R1 */, 47 /* R2 */, 21 /* R3 */, 14 /* R4 */,
    5 /* G0 */, 6 /* G1 */, 7 /* G2 */, 15 /* G3 */, 16 /* G4 */, 4 /* G5 */,
    8 /* B0 */, 3 /* B1 */, 46 /* B2 */, 9 /* B3 */, 1 /* B4 */
);

// Uncomment for ST7262 IPS LCD 800x480
 Arduino_RPi_DPI_RGBPanel *gfx = new Arduino_RPi_DPI_RGBPanel(
   bus,
   800 /* width */, 0 /* hsync_polarity */, 8 /* hsync_front_porch */, 4 /* hsync_pulse_width */, 8 /* hsync_back_porch */,
   480 /* height */, 0 /* vsync_polarity */, 8 /* vsync_front_porch */, 4 /* vsync_pulse_width */, 8 /* vsync_back_porch */,
   1 /* pclk_active_neg */, 16000000 /* prefer_speed */, true /* auto_flush */);


String image_list[6] = {

    "/image01.jpg",
    "/image02.jpg",
    "/image03.jpg",
    "/image04.jpg",
    "/image05.jpg",
    "/logo.jpg"
};

//---- Main --------------------------------------------------

void setup()
{
    Serial.begin(115200);

    gfx->begin();
    gfx->fillScreen(WHITE);
    
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, LOW);
    sd_init();
    touch_init();
    Serial.println("setup done");
    jpegDraw(image_list[5].c_str(), jpegDrawCallback, true, 0, 0, gfx->width(), gfx->height());
    delay(1000);
}

void loop()
{
  tp.read();
  if (tp.isTouched)
  {
      i++;
      //Serial.println(i);
      tp.isTouched = false;
  }
  jpegDraw(image_list[i%5].c_str(), jpegDrawCallback, true, 0, 0, gfx->width(), gfx->height());
}

static int jpegDrawCallback(JPEGDRAW *pDraw)
{
    gfx->draw16bitBeRGBBitmap(pDraw->x, pDraw->y, pDraw->pPixels, pDraw->iWidth, pDraw->iHeight);
    return 1;
}

//---- Device init --------------------------------------------------
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

void sd_init()
{
    SD_MMC.setPins(PIN_SD_CLK, PIN_SD_CMD, PIN_SD_D0);
    if (!SD_MMC.begin("/sdcard", true, true))
    {
        USBSerial.println("Card Mount Failed");
        return;
    }
}


