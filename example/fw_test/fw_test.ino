#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

#include <Arduino_GFX_Library.h>
#include <SD.h>
#include <TAMC_GT911.h>
#include <Wire.h>
#include "Audio.h"
#include <SD_MMC.h>
#include "FS.h"

//jpeg
#include "JpegFunc.h"

#define I2C_SDA_PIN 17
#define I2C_SCL_PIN 18

//microSD card
// microSD card
#define PIN_SD_CMD 11
#define PIN_SD_CLK 12
#define PIN_SD_D0  13


//I2S
#define I2S_DOUT      19
#define I2S_BCLK      20
#define I2S_LRCK      2

String stations[] ={
        "0n-80s.radionetz.de:8000/0n-70s.mp3",
        "mediaserv30.live-streams.nl:8000/stream",
        "www.surfmusic.de/m3u/100-5-das-hitradio,4529.m3u",
        "stream.1a-webradio.de/deutsch/mp3-128/vtuner-1a",
        "mp3.ffh.de/radioffh/hqlivestream.aac", //  128k aac
        "www.antenne.de/webradio/antenne.m3u",
        "listen.rusongs.ru/ru-mp3-128",
        "edge.audio.3qsdn.com/senderkw-mp3",
        "macslons-irish-pub-radio.com/media.asx",
};

#define JPEG_FILENAME_LOGO "/logo.jpg"
//#define JPEG_FILENAME_COVER "/cover.jpg"

#define JPEG_FILENAME_COVER_01 "/cover01.jpg"

#define JPEG_FILENAME_01 "/image01.jpg"
#define JPEG_FILENAME_02 "/image02.jpg"
#define JPEG_FILENAME_03 "/image03.jpg"
#define JPEG_FILENAME_04 "/image04.jpg"
#define JPEG_FILENAME_05 "/image05.jpg"
#define JPEG_FILENAME_06 "/image06.jpg"
#define JPEG_FILENAME_07 "/image07.jpg"

int music_index = 1;

#define AUDIO_FILENAME_01   "/Dear users.mp3"
//Dear_users.mp3



Audio audio;

#define TOUCH_INT -1
#define TOUCH_RST 38

#define TOUCH_ROTATION ROTATION_NORMAL
#define TOUCH_MAP_X1 800
#define TOUCH_MAP_X2 0
#define TOUCH_MAP_Y1 480
#define TOUCH_MAP_Y2 0

#define GFX_BL 10
#define TFT_BL GFX_BL
const int freq = 50000;
const int channel = 0;
const int resolution = 8;

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


int touch_last_x = 0, touch_last_y = 0;

TAMC_GT911 ts = TAMC_GT911(I2C_SDA_PIN, I2C_SCL_PIN, TOUCH_INT, TOUCH_RST, max(TOUCH_MAP_X1, TOUCH_MAP_X2), max(TOUCH_MAP_Y1, TOUCH_MAP_Y2));

int ColorArray[] = {BLACK, BLUE, GREEN, WHITE, RED, ORANGE, NAVY, DARKGREEN, DARKCYAN, MAROON, PURPLE, OLIVE, LIGHTGREY, DARKCYAN, DARKGREY, MAGENTA, YELLOW, GREENYELLOW, PINK};
//int ColorArray[]={BLACK,BLUE,GREEN,WHITE,RED,ORANGE};

void touch_init(void)
{
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  ts.begin();
  ts.setRotation(TOUCH_ROTATION);
}

bool touch_touched(void)
{
  ts.read();
  if (ts.isTouched) {
    for (int i = 0; i < ts.touches; i++) {
    touch_last_x = map(ts.points[0].x, TOUCH_MAP_X1, TOUCH_MAP_X2, 0, 480 - 1);
    touch_last_y = map(ts.points[0].y, TOUCH_MAP_Y1, TOUCH_MAP_Y2, 0, 480 - 1);

      Serial.print("Touch "); Serial.print(i + 1); Serial.print(": ");;
      Serial.print("  x: "); Serial.print(ts.points[i].x);
      Serial.print("  y: "); Serial.print(ts.points[i].y);
      Serial.print("  size: "); Serial.println(ts.points[i].size);
      Serial.println(' ');
      break;
    }
    ts.isTouched = false;
    return true;
  }
  else
  {
    return false;
  }
}

// pixel drawing callback
static int jpegDrawCallback(JPEGDRAW *pDraw)
{
  // Serial.printf("Draw pos = %d,%d. size = %d x %d\n", pDraw->x, pDraw->y, pDraw->iWidth, pDraw->iHeight);
  gfx->draw16bitBeRGBBitmap(pDraw->x, pDraw->y, pDraw->pPixels, pDraw->iWidth, pDraw->iHeight);

  return 1;
}



TaskHandle_t xHandleTaskAudio; //vTaskDelete(xHandleTaskAudio); 

// define two tasks for Music & uart
void TaskAudio( void *pvParameters );
void TaskDisplay( void *pvParameters );

// optional
void audio_info(const char *info){
    Serial.print("info        "); Serial.println(info);
}
void audio_id3data(const char *info){  //id3 metadata
    Serial.print("id3data     ");Serial.println(info);
}
void audio_eof_mp3(const char *info){  //end of file
    vTaskDelete(xHandleTaskAudio);
    Serial.print("eof_mp3     ");Serial.println(info);
}
void audio_showstation(const char *info){
    Serial.print("station     ");Serial.println(info);
}
void audio_showstreamtitle(const char *info){
    Serial.print("streamtitle ");Serial.println(info);
}
void audio_bitrate(const char *info){
    Serial.print("bitrate     ");Serial.println(info);
}
void audio_commercial(const char *info){  //duration in sec
    Serial.print("commercial  ");Serial.println(info);
}
void audio_icyurl(const char *info){  //homepage
    Serial.print("icyurl      ");Serial.println(info);
}
void audio_lasthost(const char *info){  //stream URL played
    Serial.print("lasthost    ");Serial.println(info);
}

// the setup function runs once when you press reset or power the board
void setup() {
  
  // initialize serial communication at 115200 bits per second:
  Serial.begin(115200);

  pinMode(GFX_BL, OUTPUT);
  digitalWrite(GFX_BL, LOW);
  ledcSetup(channel, freq, resolution); // 设置通道
  ledcAttachPin(GFX_BL, channel);  // 将通道与对应的引脚连接

  SD_MMC.setPins(PIN_SD_CLK, PIN_SD_CMD, PIN_SD_D0);
  if (!SD_MMC.begin("/sdcard", true, true))
  {
      Serial.println("Card Mount Failed");
      return;
  }

  audio.setPinout(I2S_BCLK, I2S_LRCK, I2S_DOUT);
  audio.setVolume(21); // 0...21
  audio.connecttoFS(SD_MMC, AUDIO_FILENAME_01);

  #if 1
  // some of time is may crash
  // Now set up two tasks to run independently.
  xTaskCreatePinnedToCore(
    TaskAudio
    ,  "TaskAudio"   // A name just for humans
    ,  12288  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  3  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL 
    ,  1//,  ARDUINO_RUNNING_CORE  // Core you want to run the task on (0 or 1)
    );
#endif

  pinMode(TOUCH_RST, OUTPUT);
  delay(100);
  digitalWrite(TOUCH_RST, LOW);
  delay(1000);
  digitalWrite(TOUCH_RST, HIGH);
  delay(1000);
  
  //while (!Serial);
  Serial.println("ESP32S3 5.0inch");

  digitalWrite(TOUCH_RST, LOW);
  delay(1000);
  digitalWrite(TOUCH_RST, HIGH);
  delay(1000);
  
  touch_init();
  delay(300);
    
  //Init Display
  gfx->begin();
  gfx->fillScreen(WHITE);
  
  gfx->setTextSize(4);
  gfx->setTextColor(BLACK);
  gfx->setCursor(180, 50);
  gfx->println(F("Makerfabs"));
  
  gfx->setTextSize(3);
  gfx->setCursor(30, 100);
  gfx->println(F("5inch TFT with Touch ")); 
  for(int i=250;i>0;i--)//背光0-250--->亮--暗
  {
      ledcWrite(channel, i);
      delay(10);
  }
  //delay(1000);

  gfx->setCursor(0, 20);
  //gfx->println(F("RED"));
  gfx->fillScreen(RED);
  Serial.println("--RED--");
  delay(1000);
  //gfx->println(F("GREEN"));
  gfx->fillScreen(GREEN);
  Serial.println("--GREEN--");
  delay(1000);
  //gfx->println(F("BLUE"));
  gfx->fillScreen(BLUE);
  Serial.println("--BLUE--");
  delay(1000);
  //gfx->println(F("WHITE"));
  gfx->fillScreen(WHITE);
  Serial.println("--WHITE--");
  delay(1000);
  gfx->fillScreen(BLACK);
  Serial.println("--BLACK--");
  delay(1000);

//    audio.connecttohost("http://www.wdr.de/wdrlive/media/einslive.m3u");
//    audio.connecttohost("http://macslons-irish-pub-radio.com/media.asx");
//    audio.connecttohost("http://mp3.ffh.de/radioffh/hqlivestream.aac"); //  128k aac
//    audio.connecttohost("http://mp3.ffh.de/radioffh/hqlivestream.mp3"); //  128k mp3

  delay(1000);
  Serial.println("ESP32S3 5.0inch Start");

#if 1

  gfx->fillScreen(WHITE);
  gfx->fillRect(200, 160, 260, 160, BLACK);

  gfx->setTextSize(4);
  gfx->setTextColor(WHITE);
  gfx->setCursor(200 + 20, 180);
  gfx->println("3 * POINTS ");
  gfx->setCursor(200 + 20, 220);
  gfx->println("TOUCH TO");
  gfx->setCursor(200 + 20, 260);
  gfx->println("CONTINUE");
  delay(500);
  jpegDraw(JPEG_FILENAME_COVER_01, jpegDrawCallback, true /* useBigEndian */,
           0 /* x */, 0 /* y */, gfx->width() /* widthLimit */, gfx->height() /* heightLimit */);
  delay(500);

  while (1)
  {
    ts.read();
    if (ts.isTouched) {
      for (int i = 0; i < ts.touches; i++) {
        //gfx->fillScreen(WHITE);
        touch_last_x = map(ts.points[0].x, TOUCH_MAP_X1, TOUCH_MAP_X2, 0, 480 - 1);
        touch_last_y = map(ts.points[0].y, TOUCH_MAP_Y1, TOUCH_MAP_Y2, 0, 480 - 1);

        int temp = random(0, 6);
        gfx->fillScreen(ColorArray[temp]);

        gfx->setTextSize(4);
        //if(num) gfx->setTextColor(BLACK);
        //else
        if (temp == 4)
          gfx->setTextColor(WHITE);
        else
          gfx->setTextColor(RED);
        gfx->setCursor(320, 400);
        gfx->print("X: ");
        gfx->println(String(ts.points[i].x));
        gfx->setCursor(320, 440);
        gfx->print("Y: ");
        gfx->println(String(ts.points[i].y));
        
        Serial.print("Touch "); Serial.print(i + 1); Serial.print(": ");;
        Serial.print("  x: "); Serial.print(ts.points[i].x);
        Serial.print("  y: "); Serial.print(ts.points[i].y);
        Serial.print("  size: "); Serial.println(ts.points[i].size);
        Serial.println(' ');

      }

      ts.isTouched = false;
      if (ts.touches > 2) break;
    }
    delay(100);
  }
#endif

#if 1
  xTaskCreatePinnedToCore(
    TaskDisplay
    ,  "TaskDisplay"
    ,  20480  // Stack size
    ,  NULL
    ,  1  // Priority
    ,  NULL 
    ,  ARDUINO_RUNNING_CORE);

#endif
  // Now the task scheduler, which takes over control of scheduling individual tasks, is automatically started.
}

void loop()
{
  
  delay(2);
}

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/

void TaskAudio(void *pvParameters)  // This is a task.
{
  (void) pvParameters;

  Serial.println("Task UART Begin!");
  for (;;) // A Task shall never return or exit.
  {
    audio.loop();//play musics 
    //vTaskDelete(NULL); // Delete this task
    //vTaskDelay(2000);
  }
}

void TaskDisplay(void *pvParameters)  // This is a task.
{
  (void) pvParameters;
  
  Serial.println("ESP32S3 Display Begin!");
  for (;;)
  {
    vTaskDelay(1000);
#if 0
    gfx->fillScreen(GREEN);
    //Serial.println("GREEN");
    vTaskDelay(1000);
    gfx->fillScreen(BLUE);
    //Serial.println("BLUE");
    vTaskDelay(1000);
    gfx->fillScreen(WHITE);
    //Serial.println("WHITE");
    vTaskDelay(1000);
    gfx->fillScreen(YELLOW);
    //Serial.println("YELLOW");
    vTaskDelay(1000); 
#endif

#if 1
    unsigned long start = millis();//gfx->width() //gfx->height()
    jpegDraw(JPEG_FILENAME_01, jpegDrawCallback, true /* useBigEndian */,
                 0 /* x */, 0 /* y */, gfx->width() /* widthLimit */, gfx->height() /* heightLimit */);
    Serial.printf("Time used: %lums\n", millis() - start);
    vTaskDelay(1000);
    start = millis();
    jpegDraw(JPEG_FILENAME_02, jpegDrawCallback, true /* useBigEndian */,
                 0 /* x */, 0 /* y */, gfx->width() /* widthLimit */, gfx->height() /* heightLimit */);
    Serial.printf("Time used: %lums\n", millis() - start);
    vTaskDelay(1000);
    start = millis();
    jpegDraw(JPEG_FILENAME_03, jpegDrawCallback, true /* useBigEndian */,
                 0 /* x */, 0 /* y */, gfx->width() /* widthLimit */, gfx->height() /* heightLimit */);
    Serial.printf("Time used: %lums\n", millis() - start);
    vTaskDelay(1000);
    start = millis();
    jpegDraw(JPEG_FILENAME_04, jpegDrawCallback, true /* useBigEndian */,
                 0 /* x */, 0 /* y */, gfx->width() /* widthLimit */, gfx->height() /* heightLimit */);
    Serial.printf("Time used: %lums\n", millis() - start);
    vTaskDelay(1000);  
    start = millis();
    jpegDraw(JPEG_FILENAME_05, jpegDrawCallback, true /* useBigEndian */,
                 0 /* x */, 0 /* y */, gfx->width() /* widthLimit */, gfx->height() /* heightLimit */);
    Serial.printf("Time used: %lums\n", millis() - start);
    vTaskDelay(1000);
#endif
  }
}
