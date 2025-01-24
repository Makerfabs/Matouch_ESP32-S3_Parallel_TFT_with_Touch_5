/*
Arduino IDE 2.3.4
ESP32 Board v2.0.16
GFX Library for Arduino v1.3.1
JPEGDEC v1.2.7
TAMC_GT911 v1.0.2

Tools:
Flash size: 16MB
Partition Scheme: 16M Flash(3MB APP / 9.9MB FATFS)
PSRAM: OPI PSRAM
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

int next_image=0;
int Screen_index=0;
int last_x=0;
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

String image_couplets[3] = {
    "/image_couplet1.jpg",
    "/image_couplet2.jpg",
    "/image_couplet3.jpg"
};

String image_red[3] = {
    "/image_red1.jpg",
    "/image_red2.jpg",
    "/image_red3.jpg"
};

String image_food[4] = {
    "/image_food1.jpg",
    "/image_food2.jpg",
    "/image_food3.jpg",
    "/image_food4.jpg"
};
//---- Main --------------------------------------------------

void setup()
{
    Serial.begin(115200);

    gfx->begin();
    gfx->fillScreen(BLACK);
    
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, LOW);
    sd_init();
    touch_init();
    jpegDraw("/image_master.jpg", jpegDrawCallback, true, 0, 0, gfx->width(), gfx->height());
    Serial.println("setup done");
    print_value();
    delay(1000);
}

void loop()
{
  read_touch();
  delay(10);
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
        Serial.println("Card Mount Failed");
        return;
    }
}


void read_touch()
{
  tp.read();
  if (tp.isTouched)
  {
    delay(100);
    if (tp.isTouched)
    {
      if(Screen_index==0)
      {
        if(tp.points[0].x>=400)
        {
          last_x=tp.points[0].x;
          if(tp.points[0].y<=110){Screen_index=1;}
          else if(tp.points[0].y>110 && tp.points[0].y<220){Screen_index=2;}
          else if(tp.points[0].y>=220){Screen_index=3;}
          change_screen();
        }
      }

      else
      {
        if(tp.points[0].x>=400 && tp.points[0].x!=last_x)
        {
          next_image++;
          if(Screen_index==2 && next_image==4){next_image=0;}
          else if(Screen_index!=2 &&next_image==3){next_image=0;}
          next_one(Screen_index,next_image);
        }
        else if(tp.points[0].x<=400)
        {
          back_master_screen();
        }
        last_x=tp.points[0].x;
      }
      print_value();
      tp.isTouched = false;
    }
  }
}


void change_screen()
{
  if(Screen_index==1)
  {
    jpegDraw(image_couplets[0].c_str(), jpegDrawCallback, true, 0, 0, gfx->width(), gfx->height());
  }
  else if(Screen_index==2)
  {
    jpegDraw(image_food[0].c_str(), jpegDrawCallback, true, 0, 0, gfx->width(), gfx->height());
  }
  else if(Screen_index==3)
  {
    jpegDraw(image_red[0].c_str(), jpegDrawCallback, true, 0, 0, gfx->width(), gfx->height());
  }
}


void next_one(int screen, int image_length)
{
  if(screen==1)
  {
    jpegDraw(image_couplets[image_length].c_str(), jpegDrawCallback, true, 0, 0, gfx->width(), gfx->height());
  }
  else if(screen==2)
  {
    jpegDraw(image_food[image_length].c_str(), jpegDrawCallback, true, 0, 0, gfx->width(), gfx->height());
  }
  else if(screen==3)
  {
    jpegDraw(image_red[image_length].c_str(), jpegDrawCallback, true, 0, 0, gfx->width(), gfx->height());
  }
}

void back_master_screen()
{
  jpegDraw("/image_master.jpg", jpegDrawCallback, true, 0, 0, gfx->width(), gfx->height());
  Screen_index=0;
  next_image=0;
}


void print_value()
{
  Serial.print("  x: ");
  Serial.print(tp.points[0].x);
  Serial.print("  y: ");
  Serial.println(tp.points[0].y);
  Serial.print(" Screen_index: ");
  Serial.println(Screen_index);
  Serial.print(" next_image: ");
  Serial.println(next_image);
  Serial.println();
}
