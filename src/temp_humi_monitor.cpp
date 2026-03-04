#include "temp_humi_monitor.h"
DHT20 dht20;
// Set the LCD I2C address to 0x27 for a 16 column and 2 row.
LiquidCrystal_I2C lcd(0x27,16,2);

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
#define LIGHT_ANALOG_PIN 1 //A0

// Instance to hold sensor data
SensorData sensorData;
// Determine light level
String lightLevel;

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
        int lightValue = analogRead(LIGHT_ANALOG_PIN);

        // Check if any reads failed and exit early
        if (isnan(temperature) || isnan(humidity)) {
            Serial.println("Failed to read from DHT sensor!");
            temperature = humidity =  -1;
            //return;
        }

        if(lightValue <= 1200) lightLevel = "Bright";
        else if(lightValue <= 3200) lightLevel = "Medium";
        else lightLevel = "Dark";

        // Đóng gói dữ liệu vào struct và gửi vào queue
        sensorData.temperature = temperature;
        sensorData.humidity = humidity;
        sensorData.light_value = lightValue;
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
        Serial.print("Light analog: ");
        Serial.print(lightValue);
        Serial.print(" -> ");
        Serial.println(lightLevel);

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
        vTaskDelay(1000);
        
        // ==================================================
        StaticJsonDocument<128> doc;
        doc["type"] = "sensor_data";
        doc["temperature"] = temperature;
        doc["humidity"] = humidity;
        doc["light_value"] = lightValue;  // Gửi giá trị analog (0-4095)

        String jsonString;
        serializeJson(doc, jsonString);
        
        Webserver_sendata(jsonString);
        // ==================================================
        vTaskDelay(3000 / portTICK_PERIOD_MS); // Đọc mỗi 3 giây
        
    }
}
void draw(){
    u8g2.setFont(u8g2_font_ncenB08_tr);       // choose a suitable font
    char temp_str[20];
    sprintf(temp_str, "Temp: %.2f C", sensorData.temperature);
    u8g2.drawStr(0,10,temp_str);
    char humi_str[20];
    sprintf(humi_str, "Humi: %.2f %%", sensorData.humidity);
    u8g2.drawStr(0,30,humi_str);
    char light_str[20];
    sprintf(light_str, "Light: %s", lightLevel.c_str());
    u8g2.drawStr(0,50,light_str);
}