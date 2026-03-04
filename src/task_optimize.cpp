#include "task_optimize.h"

enum PowerState {
    NORMAL,
    POWER_OPTIMIZE,
    LIGHT_SLEEP
};

#define BOOT 0 // Chân nút boot

void task_power_optimize(void *pvParameters){
    PowerState state = NORMAL;
    unsigned long buttonPressStartTime = 0;
    static bool bootHandled = false;

    pinMode(BOOT, INPUT_PULLUP);
    // pinMode(7, INPUT); // GPIO7 wakeup nếu cần

    while (1)
    {
        switch (state) {
            case NORMAL:
                // Chuyển sang POWER_OPTIMIZE nếu nhấn BOOT > 1s, chỉ chuyển 1 lần cho mỗi lần nhấn giữ
                if (digitalRead(BOOT) == LOW) {
                    if (buttonPressStartTime == 0) buttonPressStartTime = millis();
                    else if ((millis() - buttonPressStartTime > 1000) && !bootHandled) {
                        xSemaphoreGive(xBinarySemaphoreSavePower);
                        state = POWER_OPTIMIZE;
                        buttonPressStartTime = 0;
                        bootHandled = true;
                        Serial.println("Switched to POWER_OPTIMIZE mode");
                    }
                } else {
                    buttonPressStartTime = 0;
                    bootHandled = false;
                }
                break;

            case POWER_OPTIMIZE:
                // Chuyển về NORMAL nếu nhấn BOOT > 1s, chỉ chuyển 1 lần cho mỗi lần nhấn giữ
                if (digitalRead(BOOT) == LOW) {
                    if (buttonPressStartTime == 0) buttonPressStartTime = millis();
                    else if ((millis() - buttonPressStartTime > 1000) && !bootHandled) {
                        xSemaphoreGive(xBinarySemaphoreNormalMode);
                        state = NORMAL;
                        buttonPressStartTime = 0;
                        bootHandled = true;
                        Serial.println("Switched to NORMAL mode");
                    }
                } else {
                    buttonPressStartTime = 0;
                    bootHandled = false;
                }

                // Nếu không có người thì chuyển sang LIGHT_SLEEP
                if (!human_detected) {
                    state = LIGHT_SLEEP;
                    Serial.println("Switched to LIGHT_SLEEP mode");
                }
                break;

            case LIGHT_SLEEP:
                Serial.println("No target detected. Entering light sleep mode...");
                Serial.print("GPIO7 state before sleep: ");
                Serial.println(digitalRead(7));
                // Cấu hình wakeup bằng GPIO7 (HIGH)
                esp_sleep_enable_ext0_wakeup(GPIO_NUM_7, 1); // Wakeup khi có người
                esp_light_sleep_start();
                Serial.println("Woke up! Checking wakeup source...");
                state = POWER_OPTIMIZE;
                break;

            default:
                break;
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}