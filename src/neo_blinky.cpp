#include "neo_blinky.h"

enum NeoPixelState {
    STATE_DEFAULT,
    STATE_CUSTOM,
    STATE_TEMP,
    STATE_HUMI,
    STATE_LIGHT
};

NeoPixelState getNeoPixelState(const String& mode) {
    if (mode == "custom") return STATE_CUSTOM;
    if (mode == "temp") return STATE_TEMP;
    if (mode == "humi") return STATE_HUMI;
    if (mode == "light") return STATE_LIGHT;
    return STATE_DEFAULT;
}

NeoPixelState state = STATE_DEFAULT;

void neo_blinky(void *pvParameters) {
    Adafruit_NeoPixel strip(LED_COUNT, NEO_PIN, NEO_GRB + NEO_KHZ800);
    strip.begin();
    strip.clear();
    strip.show();

    SensorData sensorData = {0};
    NeoPixelConfigStruct config = {};
    while (1) {
        // Ưu tiên kiểm tra semaphore
        if (xSemaphoreTake(xBinarySemaphoreNeoPixel, 0) == pdTRUE) {
            // Đèn trắng báo hiệu kết nối WiFi thành công
            strip.setPixelColor(0, strip.Color(255, 255, 255));
            strip.show();
            vTaskDelay(2000 / portTICK_PERIOD_MS);
            strip.setPixelColor(0, strip.Color(0, 0, 0));
            strip.show();
            continue;
        }
        if (xSemaphoreTake(xBinarySemaphoreNormalMode, 0) == pdTRUE) {
            // Đèn đỏ báo hiệu đã vào mode NORMAL
            strip.setPixelColor(0, strip.Color(255, 0, 0));
            strip.show();
            vTaskDelay(2000 / portTICK_PERIOD_MS);
            // Quay về nhấp nháy đỏ mặc định
            strip.setPixelColor(0, strip.Color(0, 0, 0));
            strip.show();
            continue;
        }
        if (xSemaphoreTake(xBinarySemaphoreSavePower, 0) == pdTRUE) {
            // Đèn xanh dương báo hiệu đã vào mode SAVE POWER
            strip.setPixelColor(0, strip.Color(0, 0, 255));
            strip.show();
            vTaskDelay(2000 / portTICK_PERIOD_MS);
            // Quay về nhấp nháy đỏ mặc định
            strip.setPixelColor(0, strip.Color(0, 0, 0));
            strip.show();
            continue;
        }

        // Nhận dữ liệu cấu hình từ queue (nếu có)
        if (xQueueReceive(xQueueNeoPixelConfig, &config, 0) != pdPASS) {
#ifdef PRINT_QUEUE_STATUS
            Serial.println("xQueueNeoPixelConfig is empty");
#endif
        }
        // Nhận dữ liệu cảm biến từ queue
        if (xQueueReceive(xQueueSensorDataNeoPixel, &sensorData, 0) != pdPASS) {
#ifdef PRINT_QUEUE_STATUS
            Serial.println("xQueueSensorDataNeoPixel is empty");
#endif
        }

        // Sử dụng config thay cho các biến cấu hình cũ
        switch (state) {
            case STATE_DEFAULT:
                //Default nhấp nháy đỏ
                strip.setPixelColor(0, strip.Color(255, 0, 0));
                strip.show();
                vTaskDelay(500 / portTICK_PERIOD_MS);
                strip.setPixelColor(0, strip.Color(0, 0, 0));
                strip.show();
                vTaskDelay(500 / portTICK_PERIOD_MS);
                //Transitions
                if (config.mode == "custom") state = STATE_CUSTOM;
                else if (config.mode == "temp") state = STATE_TEMP;
                else if (config.mode == "humi") state = STATE_HUMI;
                else if (config.mode == "light") state = STATE_LIGHT;
                break;

            case STATE_CUSTOM: {
                if (config.effect == "static") {
                    strip.setPixelColor(0, strip.Color(config.neopixel_r, config.neopixel_g, config.neopixel_b));
                    strip.show();
                    vTaskDelay(100 / portTICK_PERIOD_MS);
                } else if (config.effect == "blink") {
                    strip.setPixelColor(0, strip.Color(config.neopixel_r, config.neopixel_g, config.neopixel_b));
                    strip.show();
                    vTaskDelay(300 / portTICK_PERIOD_MS);
                    strip.setPixelColor(0, strip.Color(0, 0, 0));
                    strip.show();
                    vTaskDelay(300 / portTICK_PERIOD_MS);
                } else if (config.effect == "fade") {
                    static int fadeValue = 0;
                    static int fadeDir = 1;
                    fadeValue += fadeDir * 15;
                    if (fadeValue >= 255) { fadeValue = 255; fadeDir = -1; }
                    if (fadeValue <= 0) { fadeValue = 0; fadeDir = 1; }
                    int r = (config.neopixel_r * fadeValue) / 255;
                    int g = (config.neopixel_g * fadeValue) / 255;
                    int b = (config.neopixel_b * fadeValue) / 255;
                    strip.setPixelColor(0, strip.Color(r, g, b));
                    strip.show();
                    vTaskDelay(80 / portTICK_PERIOD_MS);
                } else {
                    strip.setPixelColor(0, strip.Color(config.neopixel_r, config.neopixel_g, config.neopixel_b));
                    strip.show();
                    vTaskDelay(100 / portTICK_PERIOD_MS);
                }
                //Transitions
                if (config.mode == "temp") state = STATE_TEMP;
                else if (config.mode == "humi") state = STATE_HUMI;
                else if (config.mode == "light") state = STATE_LIGHT;
                break;
            }
            case STATE_TEMP: {
                int r = 0, g = 0, b = 0;
                if (sensorData.temperature <= config.temp_low_max) {
                    r = config.temp_low_r; g = config.temp_low_g; b = config.temp_low_b;
                } else if (sensorData.temperature <= config.temp_mid_max) {
                    r = config.temp_mid_r; g = config.temp_mid_g; b = config.temp_mid_b;
                } else {
                    r = config.temp_high_r; g = config.temp_high_g; b = config.temp_high_b;
                }
                strip.setPixelColor(0, strip.Color(r, g, b));
                strip.show();
                vTaskDelay(100 / portTICK_PERIOD_MS);
                //Transitions
                if (config.mode == "humi") state = STATE_HUMI;
                else if (config.mode == "light") state = STATE_LIGHT;
                else if (config.mode == "custom") state = STATE_CUSTOM;
                break;
            }
            case STATE_HUMI: {
                int r = 0, g = 0, b = 0;
                if (sensorData.humidity <= config.humi_low_max) {
                    r = config.humi_low_r; g = config.humi_low_g; b = config.humi_low_b;
                } else if (sensorData.humidity <= config.humi_mid_max) {
                    r = config.humi_mid_r; g = config.humi_mid_g; b = config.humi_mid_b;
                } else {
                    r = config.humi_high_r; g = config.humi_high_g; b = config.humi_high_b;
                }
                strip.setPixelColor(0, strip.Color(r, g, b));
                strip.show();
                vTaskDelay(100 / portTICK_PERIOD_MS);
                //Transitions
                if (config.mode == "light") state = STATE_LIGHT;
                else if (config.mode == "custom") state = STATE_CUSTOM;
                else if (config.mode == "temp") state = STATE_TEMP;
                break;
            }
            case STATE_LIGHT: {
                int r = 0, g = 0, b = 0;
                float light_percent = 100 - (sensorData.light_value / 4095.0 * 100.0);
                if (light_percent <= config.light_low_max) {
                    r = config.light_low_r; g = config.light_low_g; b = config.light_low_b;
                } else if (light_percent <= config.light_mid_max) {
                    r = config.light_mid_r; g = config.light_mid_g; b = config.light_mid_b;
                } else {
                    r = config.light_high_r; g = config.light_high_g; b = config.light_high_b;
                }
                strip.setPixelColor(0, strip.Color(r, g, b));
                strip.show();
                vTaskDelay(100 / portTICK_PERIOD_MS);
                //Transitions
                if (config.mode == "custom") state = STATE_CUSTOM;
                else if (config.mode == "temp") state = STATE_TEMP;
                else if (config.mode == "humi") state = STATE_HUMI;
                break;
            }
            default:
                break;
        }
    }
}