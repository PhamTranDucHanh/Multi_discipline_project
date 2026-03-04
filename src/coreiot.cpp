#include "coreiot.h"

// ----------- CONFIGURE THESE! -----------
// const char* coreIOT_Server = "app.coreiot.io";  
// const char* coreIOT_Token = "g7drm1amhd3dchr379xu";   // Device Access Token
const int   mqttPort = 1883;
// ----------------------------------------

WiFiClient espClient;
PubSubClient client(espClient);


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect (username=token, password=empty)
    if (client.connect("ESP32Client", CORE_IOT_TOKEN.c_str(), NULL)) {    //fix hard code
      Serial.println("connected to CoreIOT Server!");
      client.subscribe("v1/devices/me/rpc/request/+");
      Serial.println("Subscribed to v1/devices/me/rpc/request/+");

    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}


void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.println("] ");

  // Allocate a temporary buffer for the message
  char message[length + 1];
  memcpy(message, payload, length);
  message[length] = '\0';
  Serial.print("Payload: ");
  Serial.println(message);

  // Parse JSON
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, message);

  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }

    const char* method = doc["method"]; 
    if (strcmp(method, "setRelay1") == 0) {
      Serial.println("RPC: setRelay1 received");
      if (doc["params"].is<bool>()) {
        bool state = doc["params"];
        Serial.print("Relay1 state: ");
        Serial.println(state ? "ON" : "OFF");
        //TODO: Xử lý relay 1
        if(state){
          digitalWrite(RELAY1_PIN, HIGH);
        } else {
          digitalWrite(RELAY1_PIN, LOW);
        }
        // Gửi trạng thái relay1 lên web server
        sendRelayStatusToServer(state, "Relay 1", RELAY1_PIN);

        // Trả lời lại coreIOT
        coreiot_publishRelay( "Relay",1, state);

      } else {
        Serial.println("Invalid params type for Relay1!");
      }
    } else if (strcmp(method, "setRelay2") == 0) {
      Serial.println("RPC: setRelay2 received");
      if (doc["params"].is<bool>()) {
        bool state = doc["params"];
        Serial.print("Relay2 state: ");
        Serial.println(state ? "ON" : "OFF");
        //TODO: Xử lý relay 2
        if(state){
          digitalWrite(RELAY2_PIN, HIGH);
        } else {
          digitalWrite(RELAY2_PIN, LOW);
        }
        // Gửi trạng thái relay2 lên web server
        sendRelayStatusToServer(state, "Relay 2", RELAY2_PIN);

        // Trả lời lại coreIOT
        coreiot_publishRelay("Relay",2,state);


      } else {
        Serial.println("Invalid params type for Relay2!");
      }
    } else if (strcmp(method, "setRelay3") == 0) {
      Serial.println("RPC: setRelay3 received");
      if (doc["params"].is<bool>()) {
        bool state = doc["params"];
        Serial.print("Relay3 state: ");
        Serial.println(state ? "ON" : "OFF");
        //TODO: Xử lý relay 3
        if (state){
          digitalWrite(RELAY3_PIN, HIGH);
        } else {
          digitalWrite(RELAY3_PIN, LOW);
        }

        // Gửi trạng thái relay3 lên web server
        sendRelayStatusToServer(state, "Relay 3", RELAY3_PIN);
        
        // Trả lời lại coreIOT
        coreiot_publishRelay( "Relay", 3, state);

      } else {
        Serial.println("Invalid params type for Relay3!");
      }
    } else {
      Serial.print("Unknown method: ");
      Serial.println(method);
    }
}


void setup_coreiot(){

  while(1){
    if (xSemaphoreTake(xBinarySemaphoreInternet, portMAX_DELAY)) {
      break;
    }
    delay(500);
    Serial.print(".");
  }


  Serial.println(" Connected!");

  client.setServer(CORE_IOT_SERVER.c_str(), mqttPort);    //fix hard code
  client.setCallback(callback);

}

void coreiot_task(void *pvParameters){

  setup_coreiot();
  SensorData sensorData = {0};
  while(1){
    if (!client.connected()) {
      reconnect();
    }
    client.loop();

    // Lấy dữ liệu từ queue
    if (xQueueReceive(xQueueSensorDataCoreIOT, &sensorData, 0) != pdPASS) {
#ifdef PRINT_QUEUE_STATUS
      Serial.println("Queue Sensor Data CoreIOT empty");
#endif
    }
    float lightPercent = 100 - (sensorData.light_value / 4095 * 100);
    int roomStatus = human_detected ? 1 : 0;
    String payload = "{\"temperature\":" + String(sensorData.temperature) +
                     ",\"humidity\":" + String(sensorData.humidity) +
                     ",\"illuminance\":" + String(lightPercent) +
                     ",\"room_status\":" + String(roomStatus) + "}";

    client.publish("v1/devices/me/telemetry", payload.c_str());
#ifdef PRINT_COREIOT_PUBLISH
    Serial.println("Published payload: " + payload);
#endif
    vTaskDelay(500/portTICK_PERIOD_MS);

  }
}
void coreiot_publishRelay(const char* group, uint8_t id, bool state) {
  if (!client.connected()) {
    // reconnect(); // tùy chọn, nếu muốn chủ động
    if (!client.connected()) return;
  }
  String key = String(group) + String(id);          // ví dụ "Relay1" hoặc "LED2"
  String payload = String("{\"") + key + "\":" + (state ? "true" : "false") + "}";

  // Lưu bền trạng thái trên dashboard:
  client.publish("v1/devices/me/attributes", payload.c_str());
}