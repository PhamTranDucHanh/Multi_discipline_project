#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include <ArduinoJson.h>
#include "task_webserver.h"
#include <Wire.h>

#define PRINT_QUEUE_STATUS
#define PRINT_SENSOR_DATA
#define PRINT_COREIOT_PUBLISH
// #define PRINT_HUMAN_DETECTION

extern volatile bool human_detected;
#define RELAY1_PIN 10
#define RELAY2_PIN 9
#define RELAY3_PIN 8

// Queues for Sensor Datas
typedef struct {
	float temperature;
	float humidity;
	float light_value;
} SensorData;
extern QueueHandle_t xQueueSensorDataNeoPixel;
extern QueueHandle_t xQueueSensorDataCoreIOT;

// WiFi and CoreIOT Info
extern String WIFI_SSID;
extern String WIFI_PASS;
extern String CORE_IOT_TOKEN;
extern String CORE_IOT_SERVER;
extern String CORE_IOT_PORT;
extern SemaphoreHandle_t xBinarySemaphoreInternet;
extern SemaphoreHandle_t xBinarySemaphoreNeoPixel;
extern SemaphoreHandle_t xBinarySemaphoreNormalMode;
extern SemaphoreHandle_t xBinarySemaphoreSavePower;

// Queues for NeoPixel Config
typedef struct {
	String mode;
	String effect;
	// Temp
	int temp_low_min, temp_low_max;
	int temp_mid_min, temp_mid_max;
	int temp_high_min, temp_high_max;
	int temp_low_r, temp_low_g, temp_low_b;
	int temp_mid_r, temp_mid_g, temp_mid_b;
	int temp_high_r, temp_high_g, temp_high_b;
	// Humi
	int humi_low_min, humi_low_max;
	int humi_mid_min, humi_mid_max;
	int humi_high_min, humi_high_max;
	int humi_low_r, humi_low_g, humi_low_b;
	int humi_mid_r, humi_mid_g, humi_mid_b;
	int humi_high_r, humi_high_g, humi_high_b;
	// Light
	int light_low_min, light_low_max;
	int light_mid_min, light_mid_max;
	int light_high_min, light_high_max;
	int light_low_r, light_low_g, light_low_b;
	int light_mid_r, light_mid_g, light_mid_b;
	int light_high_r, light_high_g, light_high_b;
	// Custom color
	int neopixel_r, neopixel_g, neopixel_b;
} NeoPixelConfigStruct;
extern QueueHandle_t xQueueNeoPixelConfig;
void sendRelayStatusToServer(bool state, String name, int gpio);
#endif