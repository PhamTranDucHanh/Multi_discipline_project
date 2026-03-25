#ifndef __TASK_TELEGRAM_H__
#define __TASK_TELEGRAM_H__

#include <Arduino.h>
#include "global.h"

// Telegram Configuration
#define TELEGRAM_API_URL "https://api.telegram.org/bot"

// Function declarations
void telegram_task(void *parameter);
void send_telegram_msg(String message);

#endif