#include "global.h"

#include "led_blinky.h"
#include "neo_blinky.h"
#include "temp_humi_monitor.h"
#include "coreiot.h"

// include task
#include "task_check_info.h"
#include "task_wifi.h"
#include "task_webserver.h"
#include "task1_human_detection.h"
#include "task_optimize.h"
#include "collect_data.h"
//#define COLLECT_DATA_ONLY

void setup()
{
#ifndef COLLECT_DATA_ONLY
  Serial.begin(115200);
  check_info_File(0);

  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);
  pinMode(RELAY3_PIN, OUTPUT);

  xTaskCreate(led_blinky, "Task LED Blink", 2048, NULL, 2, NULL);
  xTaskCreate(neo_blinky, "Task NEO Blink", 2048, NULL, 2, NULL);
  xTaskCreate(temp_humi_monitor, "Task TEMP HUMI Monitor", 4096, NULL, 2, NULL);
  xTaskCreate(coreiot_task, "CoreIOT Task" ,4096  ,NULL  ,2 , NULL);
  // xTaskCreate(Task_Toogle_BOOT, "Task_Toogle_BOOT", 4096, NULL, 2, NULL);
  xTaskCreate(human_detection_task1, "Task Human Detection", 4096, NULL, 2, NULL);
  xTaskCreate(task_power_optimize, "Task Power Optimize", 2048, NULL, 2, NULL);
#endif
#ifdef COLLECT_DATA_ONLY
check_info_File(0);
  Serial.begin(115200);
  xTaskCreate(collect_data_task, "Collect Data Task", 4096, NULL, 2, NULL);
  xTaskCreate(led_blinky, "Task LED Blink", 2048, NULL, 2, NULL);   //Heartbeat
#endif
}

void loop()
{
//#ifndef COLLECT_DATA_ONLY
  if (check_info_File(1))
  {
    if (!Wifi_reconnect())
    {
      Webserver_stop();
    }
    else
    {
      // CORE_IOT_reconnect();
      // Serial.println("WiFi connected");
      // Serial.print("IP address: ");
      // Serial.println(WiFi.localIP());
    }
  }
  Webserver_reconnect();
//#endif
}