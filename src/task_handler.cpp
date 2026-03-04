#include <task_handler.h>

void handleWebSocketMessage(String message)
{
    Serial.println(message);
    // Tăng kích thước bộ nhớ cho JSON lớn hơn (mode light/temp/humi gửi xuống dài hơn 256 bytes)
    StaticJsonDocument<1024> doc;

    DeserializationError error = deserializeJson(doc, message);
    if (error)
    {
        Serial.print("Lỗi parse JSON! ");
        Serial.println(error.c_str());
        return;
    }
    JsonObject value = doc["value"];
    if (doc["page"] == "device")
    {
        if (!value.containsKey("gpio") || !value.containsKey("status"))
        {
            Serial.println("JSON thiếu thông tin gpio hoặc status");
            return;
        }

        int gpio = value["gpio"];
        const char* name = value["name"];
        String status = value["status"].as<String>();

        Serial.printf("Điều khiển GPIO %d → %s\n", gpio, status.c_str());
        pinMode(gpio, OUTPUT);

        // Xác định id theo tên
        uint8_t id = 0;
        if (strcmp(name, "Relay 1") == 0) id = 1;
        else if (strcmp(name, "Relay 2") == 0) id = 2;
        else if (strcmp(name, "Relay 3") == 0) id = 3;

        if (status.equalsIgnoreCase("ON"))
        {
            digitalWrite(gpio, HIGH);
            Serial.printf("GPIO %d ON\n", gpio);

            // Gửi trạng thái lên coreIOT
            coreiot_publishRelay("Relay", id, true);

        }
        else if (status.equalsIgnoreCase("OFF"))
        {
            digitalWrite(gpio, LOW);
            Serial.printf("GPIO %d OFF\n", gpio);

            // Gửi trạng thái lên coreIOT
            coreiot_publishRelay("Relay", id, false);
        }

    }
    else if (doc["page"] == "setting")
    {
        String WIFI_SSID = doc["value"]["ssid"].as<String>();
        String WIFI_PASS = doc["value"]["password"].as<String>();
        String CORE_IOT_TOKEN = doc["value"]["token"].as<String>();
        String CORE_IOT_SERVER = doc["value"]["server"].as<String>();
        String CORE_IOT_PORT = doc["value"]["port"].as<String>();

        Serial.println("Nhận cấu hình từ WebSocket:");
        Serial.println("SSID: " + WIFI_SSID);
        Serial.println("PASS: " + WIFI_PASS);
        Serial.println("TOKEN: " + CORE_IOT_TOKEN);
        Serial.println("SERVER: " + CORE_IOT_SERVER);
        Serial.println("PORT: " + CORE_IOT_PORT);

        // Gọi hàm lưu cấu hình
        Save_info_File(WIFI_SSID, WIFI_PASS, CORE_IOT_TOKEN, CORE_IOT_SERVER, CORE_IOT_PORT);

        // Phản hồi lại client (tùy chọn)
        String msg = "{\"status\":\"ok\",\"page\":\"setting_saved\"}";
        ws.textAll(msg);
    }
    else if (doc["page"] == "neopixel")
    {
        NeoPixelConfigStruct config;
        config.mode = doc["mode"].as<String>();
        config.effect = doc["effect"].as<String>();
        // Custom mode
        if (config.mode == "custom") {
            JsonArray colorArr = doc["color"].as<JsonArray>();
            config.neopixel_r = colorArr[0];
            config.neopixel_g = colorArr[1];
            config.neopixel_b = colorArr[2];
        }
        // Temp
        if (config.mode == "temp") {
            JsonObject temp = doc["temp"];
            config.temp_low_min = temp["low"]["min"].as<int>();
            config.temp_low_max = temp["low"]["max"].as<int>();
            JsonArray lowColor = temp["low"]["color"].as<JsonArray>();
            config.temp_low_r = lowColor[0];
            config.temp_low_g = lowColor[1];
            config.temp_low_b = lowColor[2];
            config.temp_mid_min = temp["mid"]["min"].as<int>();
            config.temp_mid_max = temp["mid"]["max"].as<int>();
            JsonArray midColor = temp["mid"]["color"].as<JsonArray>();
            config.temp_mid_r = midColor[0];
            config.temp_mid_g = midColor[1];
            config.temp_mid_b = midColor[2];
            config.temp_high_min = temp["high"]["min"].as<int>();
            config.temp_high_max = temp["high"]["max"].as<int>();
            JsonArray highColor = temp["high"]["color"].as<JsonArray>();
            config.temp_high_r = highColor[0];
            config.temp_high_g = highColor[1];
            config.temp_high_b = highColor[2];
        }
        // Humi
        if (config.mode == "humi") {
            JsonObject humi = doc["humi"];
            config.humi_low_min = humi["low"]["min"].as<int>();
            config.humi_low_max = humi["low"]["max"].as<int>();
            JsonArray lowColor = humi["low"]["color"].as<JsonArray>();
            config.humi_low_r = lowColor[0];
            config.humi_low_g = lowColor[1];
            config.humi_low_b = lowColor[2];
            config.humi_mid_min = humi["mid"]["min"].as<int>();
            config.humi_mid_max = humi["mid"]["max"].as<int>();
            JsonArray midColor = humi["mid"]["color"].as<JsonArray>();
            config.humi_mid_r = midColor[0];
            config.humi_mid_g = midColor[1];
            config.humi_mid_b = midColor[2];
            config.humi_high_min = humi["high"]["min"].as<int>();
            config.humi_high_max = humi["high"]["max"].as<int>();
            JsonArray highColor = humi["high"]["color"].as<JsonArray>();
            config.humi_high_r = highColor[0];
            config.humi_high_g = highColor[1];
            config.humi_high_b = highColor[2];
        }
        // Light
        if (config.mode == "light") {
            JsonObject light = doc["light"];
            config.light_low_min = light["low"]["min"].as<int>();
            config.light_low_max = light["low"]["max"].as<int>();
            JsonArray lowColor = light["low"]["color"].as<JsonArray>();
            config.light_low_r = lowColor[0];
            config.light_low_g = lowColor[1];
            config.light_low_b = lowColor[2];
            config.light_mid_min = light["mid"]["min"].as<int>();
            config.light_mid_max = light["mid"]["max"].as<int>();
            JsonArray midColor = light["mid"]["color"].as<JsonArray>();
            config.light_mid_r = midColor[0];
            config.light_mid_g = midColor[1];
            config.light_mid_b = midColor[2];
            config.light_high_min = light["high"]["min"].as<int>();
            config.light_high_max = light["high"]["max"].as<int>();
            JsonArray highColor = light["high"]["color"].as<JsonArray>();
            config.light_high_r = highColor[0];
            config.light_high_g = highColor[1];
            config.light_high_b = highColor[2];
        }
        //In ra Serial để kiểm tra
        Serial.println("===== NeoPixel Config Received =====");
        Serial.print("Mode: "); Serial.println(config.mode);
        Serial.print("Effect: "); Serial.println(config.effect);

        if (config.mode == "custom") {
            Serial.print("Custom Color - R: "); Serial.print(config.neopixel_r);
            Serial.print(", G: "); Serial.print(config.neopixel_g);
            Serial.print(", B: "); Serial.println(config.neopixel_b);
        }
        if (config.mode == "temp") {
            Serial.println("Temp Ranges:");
            Serial.printf("  Low [%d-%d] Color(RGB): %d,%d,%d\n", config.temp_low_min, config.temp_low_max, config.temp_low_r, config.temp_low_g, config.temp_low_b);
            Serial.printf("  Mid [%d-%d] Color(RGB): %d,%d,%d\n", config.temp_mid_min, config.temp_mid_max, config.temp_mid_r, config.temp_mid_g, config.temp_mid_b);
            Serial.printf("  High [%d-%d] Color(RGB): %d,%d,%d\n", config.temp_high_min, config.temp_high_max, config.temp_high_r, config.temp_high_g, config.temp_high_b);
        }
        if (config.mode == "humi") {
            Serial.println("Humi Ranges:");
            Serial.printf("  Low [%d-%d] Color(RGB): %d,%d,%d\n", config.humi_low_min, config.humi_low_max, config.humi_low_r, config.humi_low_g, config.humi_low_b);
            Serial.printf("  Mid [%d-%d] Color(RGB): %d,%d,%d\n", config.humi_mid_min, config.humi_mid_max, config.humi_mid_r, config.humi_mid_g, config.humi_mid_b);
            Serial.printf("  High [%d-%d] Color(RGB): %d,%d,%d\n", config.humi_high_min, config.humi_high_max, config.humi_high_r, config.humi_high_g, config.humi_high_b);
        }
        if (config.mode == "light") {
            Serial.println("Light Ranges:");
            Serial.printf("  Low [%d-%d] Color(RGB): %d,%d,%d\n", config.light_low_min, config.light_low_max, config.light_low_r, config.light_low_g, config.light_low_b);
            Serial.printf("  Mid [%d-%d] Color(RGB): %d,%d,%d\n", config.light_mid_min, config.light_mid_max, config.light_mid_r, config.light_mid_g, config.light_mid_b);
            Serial.printf("  High [%d-%d] Color(RGB): %d,%d,%d\n", config.light_high_min, config.light_high_max, config.light_high_r, config.light_high_g, config.light_high_b);
        }
        Serial.println("====================================");
        // Gửi vào queue
        if (xQueueSend(xQueueNeoPixelConfig, &config, 0) != pdPASS) {
#ifdef PRINT_QUEUE_STATUS
            Serial.println("xQueueNeoPixelConfig is full");
#endif
        }
    }
}
