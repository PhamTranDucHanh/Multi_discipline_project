#ifndef __TEMP_HUMI_MONITOR__
#define __TEMP_HUMI_MONITOR__
#include <Arduino.h>
#include "LiquidCrystal_I2C.h"
#include "DHT20.h"
#include "global.h"
#include <ArduinoJson.h>
#include "task_webserver.h"
#include <Wire.h>
#include <U8g2lib.h>

#include "random_forest_model.h"

void temp_humi_monitor(void *pvParameters);
void draw();
#endif