/*
Arduino IDE V2.3.4
LovyanGFX V1.2.0

Tools:
PSRAM:"OPI PSRAM"
*/
#include <Wire.h>
#include "LGFX_Matouch5.h"

#define GFX_BL 10

// 準備したクラスのインスタンスを作成します。
LGFX display;

void setup(void)
{
    Serial.begin(115200);
    pinMode(GFX_BL, OUTPUT);
    digitalWrite(GFX_BL, LOW);
    // SPIバスとパネルの初期化を実行すると使用可能になります。
    display.init();
    //display.fillScreen(TFT_BLUE);

    xTaskCreatePinnedToCore(Task_TFT, "Task_TFT", 20480, NULL, 2, NULL, ARDUINO_RUNNING_CORE);
    xTaskCreatePinnedToCore(Task_Touch, "Task_Touch", 2048, NULL, 2, NULL, ARDUINO_RUNNING_CORE);
}

void loop(void)
{
}

void Task_TFT(void *pvParameters) // This is a task.
{
    while (1) // A Task shall never return or exit.
    {
         display.fillRect(0, 0, 400, 480, TFT_YELLOW);
         display.fillRect(400, 0, 400, 480, TFT_RED);
         vTaskDelay(2000);
         display.fillRect(0, 0, 400, 480, TFT_RED);
         display.fillRect(400, 0, 400, 480, TFT_WHITE);
         vTaskDelay(2000);
         display.fillRect(0, 0, 400, 480, TFT_WHITE);
         display.fillRect(400, 0, 400, 480, TFT_BLUE);
         vTaskDelay(2000);

    }
}

void Task_Touch(void *pvParameters) // This is a task.
{
    while (1)
    {
        int32_t x, y;
        if (display.getTouch(&x, &y))
        {
            Serial.print(x);
            Serial.print(",");
            Serial.println(y);
        }
        vTaskDelay(100);
    }
}