#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "config/config.h"
#include "config/i2s_config.h"
#include "config/gpio_config.h"
#include "tasks/gpio_task.h"
#include "tasks/audio_task.h"

static const char *TAG = "VOICE_KEYBOARD";

// Флаг состояния I2S / I2S state flag
bool is_i2s_enabled = false;

void app_main(void)
{
    ESP_LOGI(TAG, "Voice Keyboard starting... / Голосовая клавиатура запускается...");
    
    // Сначала инициализируем GPIO / Initialize GPIO first
    ESP_ERROR_CHECK(gpio_init());
    
    // Инициализируем I2S / Initialize I2S
    ESP_ERROR_CHECK(i2s_init());
    
    // Создаем задачу GPIO / Create GPIO task
    create_gpio_task();
    
    // Создаем задачу обработки аудио / Create audio processing task
    create_audio_task();
    
    ESP_LOGI(TAG, "Voice Keyboard initialized successfully / Голосовая клавиатура успешно инициализирована");
    ESP_LOGI(TAG, "Press and hold button to record audio / Нажмите и удерживайте кнопку для записи аудио");
    
    // Основной цикл / Main loop
    while(1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        ESP_LOGD(TAG, "System running... / Система работает...");
    }
}