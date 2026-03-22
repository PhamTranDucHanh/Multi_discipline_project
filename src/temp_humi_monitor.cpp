#include "temp_humi_monitor.h"
DHT20 dht20;
// Set the LCD I2C address to 0x27 for a 16 column and 2 row.
LiquidCrystal_I2C lcd(0x27,16,2);

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
#define SMOKE_ANALOG_PIN 1 //A0

// Instance to hold sensor data
SensorData sensorData;

Eloquent::ML::Port::RandomForest model;

void temp_humi_monitor(void *pvParameters){

    Wire.begin(11, 12);
    // Serial.begin(115200);
    dht20.begin();
    u8g2.begin();
    lcd.begin();
    lcd.backlight();
    
    while (1){
        /* code */
        
        dht20.read();
        // Reading temperature in Celsius
        float temperature = dht20.getTemperature();
        // Reading humidity
        float humidity = dht20.getHumidity();
        // Read light sensor value
        float smokeValue = analogRead(SMOKE_ANALOG_PIN);

        // Check if any reads failed and exit early
        if (isnan(temperature) || isnan(humidity)) {
            Serial.println("Failed to read from DHT sensor!");
            temperature = humidity =  -1;
            //return;
        }

        static float temp_prev = 0, gas_prev = 0;

        float temp_diff = temperature - temp_prev;
        float gas_diff = smokeValue - gas_prev;

        float features[5] = {temperature, humidity, smokeValue, temp_diff, gas_diff};
        int result = model.predict(features);

        // Cập nhật giá trị trước cho lần sau
        temp_prev = temperature;
        gas_prev = smokeValue;

        // Xử lý kết quả
        if (result == 0) { 
            // Normal
            Serial.println("Normal");
         }
        else if (result == 1) { 
            /* Gas Leak */ 
            Serial.println("Gas Leak");
        }
        else if (result == 2) { 
            /* Burning */ 
            Serial.println("Burning");
        }

        // Đóng gói dữ liệu vào struct và gửi vào queue
        sensorData.temperature = temperature;
        sensorData.humidity = humidity;
        sensorData.smoke = smokeValue;
        sensorData.status = result; // 0: normal, 1: leak, 2: fire
        if (xQueueSend(xQueueSensorDataNeoPixel, &sensorData, 0) != pdPASS) {
#ifdef PRINT_QUEUE_STATUS
            Serial.println("Queue Sensor Data NeoPixel full");
#endif
        }
        if (xQueueSend(xQueueSensorDataCoreIOT, &sensorData, 0) != pdPASS) {
#ifdef PRINT_QUEUE_STATUS
            Serial.println("Queue Sensor Data CoreIOT full");
#endif
        }
        
        // Serial output
#ifdef PRINT_SENSOR_DATA
        Serial.print("Smoke analog: ");
        Serial.print(smokeValue);

        Serial.print("Humidity: ");
        Serial.print(humidity);
        Serial.print("%  Temperature: ");
        Serial.print(temperature);
        Serial.println("°C");
#endif

        lcd.setCursor(0,0);
        lcd.print("Temp: ");
        lcd.setCursor(6,0);
        lcd.print(temperature);

        lcd.setCursor(0,1);
        lcd.print("Humi: ");
        lcd.setCursor(6,1);
        lcd.print(humidity);

        // OLED display
        u8g2.clearBuffer();					// clear the internal memory
        draw();
        u8g2.sendBuffer();					// transfer internal memory to the display
        // vTaskDelay(1000);
        
        // ==================================================
        StaticJsonDocument<128> doc;
        doc["type"] = "sensor_data";
        doc["temperature"] = temperature;
        doc["humidity"] = humidity;
        doc["smokeValue"] = smokeValue;  // Gửi giá trị analog (0-4095)
        doc["status"] = result; // 0: normal, 1: leak, 2: fire

        String jsonString;
        serializeJson(doc, jsonString);
        
        Webserver_sendata(jsonString);
        // ==================================================
        vTaskDelay(1000 / portTICK_PERIOD_MS); // Đọc mỗi 1 giây
        
    }
}
void draw() {
    u8g2.setFont(u8g2_font_ncenB08_tr); // chọn font phù hợp
    char line1[32], line2[32], status_str[20];
    // Dòng 1: Temp | Humi
    sprintf(line1, "T:%.1fC  H:%.1f%%", sensorData.temperature, sensorData.humidity);
    u8g2.drawStr(0, 14, line1);
    // Dòng 2: Gas
    sprintf(line2, "Gas: %d", (int)sensorData.smoke);
    u8g2.drawStr(0, 32, line2);
    // Dòng 3: Status
    if (sensorData.status == 0)
        strcpy(status_str, "NORMAL");
    else if (sensorData.status == 1)
        strcpy(status_str, "GAS LEAK");
    else if (sensorData.status == 2)
        strcpy(status_str, "BURN");
    else
        strcpy(status_str, "UNKNOWN");
    u8g2.drawStr(0, 50, status_str);
}