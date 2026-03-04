#include "global.h"

volatile bool human_detected = false;

QueueHandle_t xQueueSensorDataNeoPixel = xQueueCreate(5, sizeof(SensorData));
QueueHandle_t xQueueSensorDataCoreIOT = xQueueCreate(5, sizeof(SensorData));

String WIFI_SSID;
String WIFI_PASS;
String CORE_IOT_TOKEN;
String CORE_IOT_SERVER;
String CORE_IOT_PORT;

SemaphoreHandle_t xBinarySemaphoreInternet = xSemaphoreCreateBinary();
SemaphoreHandle_t xBinarySemaphoreNeoPixel = xSemaphoreCreateBinary();
SemaphoreHandle_t xBinarySemaphoreNormalMode = xSemaphoreCreateBinary();
SemaphoreHandle_t xBinarySemaphoreSavePower = xSemaphoreCreateBinary();


QueueHandle_t xQueueNeoPixelConfig = xQueueCreate(5, sizeof(NeoPixelConfigStruct));

void sendRelayStatusToServer(bool state, String name, int gpio) {
    StaticJsonDocument<128> doc;
    doc["type"] = "device";
    doc["value"]["name"] = name;
    doc["value"]["status"] = state ? "ON" : "OFF";
    doc["value"]["gpio"] = gpio;
    
    String json;
    serializeJson(doc, json);

    Webserver_sendata(json); // Gửi tới tất cả client
}
