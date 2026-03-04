#ifndef __COREIOT_H__
#define __COREIOT_H__

#include <Arduino.h>
#include <WiFi.h>
#include "global.h"
#include <PubSubClient.h>
#include <ArduinoJson.h>


void coreiot_task(void *pvParameters);
void coreiot_publishRelay(const char* group, uint8_t id, bool state);
#endif  // __COREIOT_H__