#ifndef COLLECT_DATA_H
#define COLLECT_DATA_H
#include <Arduino.h>
#include "LiquidCrystal_I2C.h"
#include "DHT20.h"
#include "global.h"
#include <Wire.h>
#include <neo_blinky.h>
#include <temp_humi_monitor.h>

void collect_data_task(void *pvParameters);


#endif  // COLLECT_DATA_H