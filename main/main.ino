#include <Arduino.h>
#include <U8g2lib.h>
#include <Bounce2.h>
#include "face.h"
#include "esp_sleep.h"


/*touch config*/

#define TP_PIN 4
#define LONG_PRESS_MS 5000

Bounce BTN;

/*oled config*/

 U8G2_SSD1306_128X64_NONAME_F_HW_I2C oled(U8G2_R0, SCL, SDA, U8X8_PIN_NONE);

/*函数声明*/
void sleepClose();    // close deep sleep
void oledInit();      // init OLED
void tpInit();        // init touch sensor
void faceMove();      // face move
void sleepEnable();  // enable deep sleep

enum ClickStatus {
    NO_CLICK,
    SINGLE_CLICK,
    DOUBLE_CLICK,   
}clickStatus;

ClickStatus tpCheck();

void setup()
{
    Serial.begin(115200);
    tpInit();
    sleepClose();
    oledInit();

}

void loop()
{
    BTN.update();

    faceMove();
    sleepEnable();
}

void oledInit() {

    oled.begin();

    oled.enableUTF8Print(); 

    oled.setFontMode(1);
    oled.setFont(u8g2_font_wqy12_t_gb2312);
    oled.setDrawColor(1);
    oled.setPowerSave(0); 

    oled.clearDisplay();
}

void tpInit(){

    BTN.attach(TP_PIN, INPUT);
    BTN.interval(50);
}

void sleepClose(){

    esp_sleep_enable_gpio_wakeup();    // keep GPIO wakeup enabled
    esp_sleep_wakeup_cause_t wake = esp_sleep_get_wakeup_cause();

    if (wake == ESP_SLEEP_WAKEUP_GPIO) {
        Serial.println("touch wakeup detected, verifying if long press 2 seconds...");
        
        uint32_t checkStart = millis();
        bool verifySuccess = true;

        // check if long press 2 seconds
        while (millis() - checkStart < 2000) {

            // check if touch is released
            if (digitalRead(TP_PIN) == LOW) {
                verifySuccess = false;
                break;
            }
            delay(20); 
        }

        if (!verifySuccess) {
            Serial.println("touch released before 2 seconds, re-enter deep sleep!");
            
            while (digitalRead(TP_PIN) == HIGH) { delay(20); }
            delay(100);

            esp_deep_sleep_enable_gpio_wakeup(1ULL << TP_PIN, ESP_GPIO_WAKEUP_GPIO_HIGH);
            esp_deep_sleep_start();
        } else {
            Serial.println("touch pressed for 2 seconds, booting up...");
        }
    }
    
}

void sleepEnable(){

    if (BTN.read() == HIGH && BTN.currentDuration() >= LONG_PRESS_MS) {

        Serial.println("touch pressed for 50 seconds, entering deep sleep...");

        oled.clearDisplay();

        while (digitalRead(TP_PIN) == HIGH) {
            delay(20); 
        }

            Serial.println("touch released, 50 seconds deep sleep enabled!");
            delay(100); 


            esp_deep_sleep_enable_gpio_wakeup(1ULL << TP_PIN, ESP_GPIO_WAKEUP_GPIO_HIGH);
            esp_deep_sleep_start();

              
    } 

    delay(50);
}

void faceMove(){

    static short x_tag = 0;
    static short vx = 6;
    static short Select = 0;
    if(x_tag <= -12)
    {
        vx = -vx;
    }
    else if (x_tag >= 12)
    {
        vx = -vx;
    }

    x_tag += vx;

 
    static unsigned long lastTime = millis();

    if (millis() - lastTime >= 1000)
    {

        lastTime = millis();

        oled.clearBuffer();

        oled.drawXBMP(x_tag, 0, 128, 64, face_arry[0]);
    

        oled.sendBuffer();
    }
}
