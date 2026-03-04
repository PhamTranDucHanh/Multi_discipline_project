#include "collect_data.h"
#include <WiFi.h>
#include <time.h> // Thư viện lấy thời gian NTP

#define BOOT_BUTTON_PIN 0 // GPIO0 
#define MQ135_PIN 1       // Chân analog đọc MQ-135 (A0)

// Máy trạng thái
enum CollectState {
  NORMAL = 0,
  LEAK = 1,
  FIRE = 2,
  RAIN = 3,
  IDLE0 = 4,
  IDLE1 = 5,
  IDLE2 = 6,
  IDLE3 = 7
};

volatile CollectState currentState = IDLE0;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50; // ms
DHT20 dht20x;
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2x(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);


// Hàm lấy timestamp dạng yyyy-mm-dd HH:MM:SS
String getTimestamp() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "1970-01-01 00:00:00";
  }
  char buf[20];
  strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(buf);
}

void setupTime() {
  // Cấu hình NTP với server Việt Nam và múi giờ GMT+7
  configTime(7 * 3600, 0, "1.vn.pool.ntp.org", "0.vn.pool.ntp.org", "time1.google.com");
  // Chờ đồng bộ thời gian
  struct tm timeinfo;
  int retry = 0;
  while (!getLocalTime(&timeinfo) && retry < 10) {
    delay(500);
    retry++;
  }
}

void drawx(float temperature, float humidity, int smoke){
    u8g2x.setFont(u8g2_font_ncenB08_tr);       // choose a suitable font
    char temp_str[20];
    sprintf(temp_str, "Temp: %.2f C", temperature);
    u8g2x.drawStr(0,10,temp_str);
    char humi_str[20];
    sprintf(humi_str, "Humi: %.2f %%", humidity);
    u8g2x.drawStr(0,30,humi_str);
    char light_str[20];
    sprintf(light_str, "Smoke: %d", smoke);
    u8g2x.drawStr(0,50,light_str);
}

void toMonitor(CollectState state, String timestamp, float temperature, float humidity, int smoke) {
    Serial.print(timestamp); Serial.print(",");
    Serial.print(temperature, 2); Serial.print(",");
    Serial.print(humidity, 2); Serial.print(",");
    Serial.print(smoke); Serial.print(",");
    switch (state) {
        case NORMAL:
            Serial.println("0");
            break;
        case LEAK:
            Serial.println("1");
            break;
        case FIRE:
            Serial.println("2");
            break;
        case RAIN:
            Serial.println("3");
            break;
        default:
            break;
    }
}

void collect_data_task(void *pvParameters)
{
    pinMode(BOOT_BUTTON_PIN, INPUT_PULLUP);
    Wire.begin(11, 12);
    dht20x.begin();
    Adafruit_NeoPixel strip(LED_COUNT, NEO_PIN, NEO_GRB + NEO_KHZ800);
    strip.begin();
    strip.clear();
    strip.show();

    strip.setPixelColor(0, strip.Color(203, 0, 230));  //Hiện màu hồng khi chưa kết nối WiFi
    strip.show();
    u8g2x.begin();

    while(1){   //Wait for WiFi connection
      if (xSemaphoreTake(xBinarySemaphoreInternet, portMAX_DELAY)) {
        break;
      }
      vTaskDelay(500);
      Serial.print(".");
    }

    setupTime(); // Lấy thời gian từ NTP sau khi đã kết nối WiFi

    while (1) {
      dht20x.read();
      float temperature = dht20x.getTemperature();
      float humidity = dht20x.getHumidity();
      int smoke = analogRead(MQ135_PIN);
      String timestamp = getTimestamp();

      // OLED display
      u8g2x.clearBuffer();					// clear the internal memory
      drawx(temperature, humidity, smoke);	
      u8g2x.sendBuffer();					// transfer internal memory to the display

      switch (currentState) {
        case IDLE0:
          if (digitalRead(BOOT_BUTTON_PIN) == LOW) {
            currentState = NORMAL;
            lastDebounceTime = millis();
          }
          strip.setPixelColor(0, strip.Color(255, 255, 255));  //Hiện màu trắng khi IDLE
          strip.show();
          break;
        case NORMAL:
          if (digitalRead(BOOT_BUTTON_PIN) == LOW && (millis() - lastDebounceTime) > debounceDelay) {
            currentState = IDLE1;
            lastDebounceTime = millis();
          }
          strip.setPixelColor(0, strip.Color(255, 255, 0));  //Hiện màu vàng khi NORMAL
          strip.show();
          toMonitor(currentState, timestamp, temperature, humidity, smoke);
          break;
        case IDLE1:
          if (digitalRead(BOOT_BUTTON_PIN) == LOW) {
            currentState = LEAK;
            lastDebounceTime = millis();
          }
          strip.setPixelColor(0, strip.Color(255, 255, 255));  //Hiện màu trắng khi IDLE
          strip.show();
          break;
        case LEAK:
          if (digitalRead(BOOT_BUTTON_PIN) == LOW && (millis() - lastDebounceTime) > debounceDelay) {
            currentState = IDLE2;
            lastDebounceTime = millis();
          }
          strip.setPixelColor(0, strip.Color(0, 255, 0));  //Hiện màu xanh lá khi LEAK
          strip.show();
          toMonitor(currentState, timestamp, temperature, humidity, smoke);
          break;
        case IDLE2:
          if (digitalRead(BOOT_BUTTON_PIN) == LOW) {
            currentState = FIRE;
            lastDebounceTime = millis();
          }
          strip.setPixelColor(0, strip.Color(255, 255, 255));  //Hiện màu trắng khi IDLE
          strip.show();
          break;
        case FIRE:
          if (digitalRead(BOOT_BUTTON_PIN) == LOW && (millis() - lastDebounceTime) > debounceDelay) {
            currentState = IDLE3;
            lastDebounceTime = millis();
          }
          strip.setPixelColor(0, strip.Color(255, 0, 0));  //Hiện màu đỏ khi FIRE
          strip.show();
          toMonitor(currentState, timestamp, temperature, humidity, smoke);
          break;
        case IDLE3:
          if (digitalRead(BOOT_BUTTON_PIN) == LOW) {
            currentState = RAIN;
            lastDebounceTime = millis();
          }
          strip.setPixelColor(0, strip.Color(255, 255, 255));  //Hiện màu trắng khi IDLE
          strip.show();
          break;
        case RAIN:
          if (digitalRead(BOOT_BUTTON_PIN) == LOW && (millis() - lastDebounceTime) > debounceDelay) {
            currentState = IDLE0;
            lastDebounceTime = millis();
          }
          strip.setPixelColor(0, strip.Color(0, 0, 255));  //Hiện màu xanh dương khi RAIN
          strip.show();
          toMonitor(currentState, timestamp, temperature, humidity, smoke);
          break;
        default:
          break;
      }
      vTaskDelay(500 / portTICK_PERIOD_MS); // Delay 1 giây
    }
}
