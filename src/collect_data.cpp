#include "collect_data.h"
#include <WiFi.h>
#include <time.h> // Thư viện lấy thời gian NTP

#define BOOT_BUTTON_PIN 0 // GPIO0 
#define MQ135_PIN 1       // Chân analog đọc MQ-135 (A0)

// Máy trạng thái
enum CollectState {
  IDLE,
  NORMAL,
  ANOMOLY
};

volatile CollectState currentState = IDLE;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50; // ms
DHT20 dht20x;

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

      switch (currentState) {
        case IDLE:
          if (digitalRead(BOOT_BUTTON_PIN) == LOW) {
            currentState = NORMAL;
            lastDebounceTime = millis();
          }
          strip.setPixelColor(0, strip.Color(0, 255, 0));  //Hiện màu xanh lá
          strip.show();
          //hiển thị lên LCD... để sẵn sàng
          break;
        case NORMAL:
          if (digitalRead(BOOT_BUTTON_PIN) == LOW && (millis() - lastDebounceTime) > debounceDelay) {
            currentState = ANOMOLY;
            lastDebounceTime = millis();
          }
          strip.setPixelColor(0, strip.Color(0, 0, 255));  //Hiện màu xanh dương
          strip.show();

          Serial.print(timestamp); Serial.print(",");
          Serial.print(temperature, 2); Serial.print(",");
          Serial.print(humidity, 2); Serial.print(",");
          Serial.print(smoke); Serial.print(",");
          Serial.println("NORMAL");
          break;
        case ANOMOLY:
          if (digitalRead(BOOT_BUTTON_PIN) == LOW && (millis() - lastDebounceTime) > debounceDelay) {
            currentState = IDLE;
            lastDebounceTime = millis();
          }
          strip.setPixelColor(0, strip.Color(255, 0, 0));  //Hiện màu đỏ
          strip.show();
          Serial.print(timestamp); Serial.print(",");
          Serial.print(temperature, 2); Serial.print(",");
          Serial.print(humidity, 2); Serial.print(",");
          Serial.print(smoke); Serial.print(",");
          Serial.println("BURNING");
          break;
        default:
          break;
      }
      vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay 1 giây
    }
}