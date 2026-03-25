#include "task_telegram.h"
#include <WiFi.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>  
#include <ArduinoHttpClient.h> 

/**
 * @brief Gửi tin nhắn lên Telegram 
 */
void send_telegram_msg(String message) {
    // Kiểm tra WiFi
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[TELEGRAM] WiFi not connected!");
        return;
    }

    // Kiểm tra token và chat ID
    if (TELEGRAM_BOT_TOKEN == "" || TELEGRAM_CHAT_ID == "") {
        Serial.println("[TELEGRAM] Token or Chat ID not configured!");
        return;
    }

    // Khởi tạo WiFi client secure (HTTPS)
    WiFiClientSecure wifiClient;
    wifiClient.setInsecure();  // Bỏ qua SSL verification (demo)
    
    // Tạo HTTP client (Telegram API dùng HTTPS port 443)
    HttpClient client = HttpClient(wifiClient, "api.telegram.org", 443);

    // Tạo JSON payload
    DynamicJsonDocument doc(256);
    doc["chat_id"] = TELEGRAM_CHAT_ID;
    doc["text"] = message;

    String payload;
    serializeJson(doc, payload);

    // Tạo endpoint
    String path = "/bot" + TELEGRAM_BOT_TOKEN + "/sendMessage";

    Serial.println("[TELEGRAM] Sending request...");
    Serial.print("[TELEGRAM] Path: ");
    Serial.println(path);
    Serial.print("[TELEGRAM] Payload: ");
    Serial.println(payload);

    // Gửi POST request
    int statusCode = client.post(path, "application/json", payload);

    // Đọc response
    String response = client.responseBody();

    Serial.print("[TELEGRAM] Response Code: ");
    Serial.println(statusCode);
    Serial.print("[TELEGRAM] Response Body: ");
    Serial.println(response);

    if (statusCode == 200) {
        Serial.println("[TELEGRAM] Message sent successfully!");
    } else {
        Serial.println("[TELEGRAM] Failed to send message!");
    }

    client.stop();
}

/**
 * @brief Task Telegram - gửi khi có yêu cầu
 */
void telegram_task(void *parameter) {
    send_telegram_msg("Connected sucessfully. ESP32 is online.");
    bool gas = false, fire = false;
    while(1){
        gas  = (xSemaphoreTake(xBinarySemaphoreMsgGas, 0) == pdTRUE);
        fire = (xSemaphoreTake(xBinarySemaphoreMsgBurn, 0) == pdTRUE);
        if (gas)  send_telegram_msg("WARNING: Gas leak detected!");
        if (fire) send_telegram_msg("WARNING: Fire detected!");
        vTaskDelay(10000 / portTICK_PERIOD_MS); // Chạy task mỗi 10 giây
    }

}