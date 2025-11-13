#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "config/config.h"
#include "config/i2s_config.h"
#include "config/gpio_config.h"
#include "config/hid_config.h"
#include "config/voice_commands.h"
#include "tasks/gpio_task.h"
#include "tasks/audio_task.h"
#include "tasks/hid_task.h"

static const char *TAG = "VOICE_KEYBOARD";

// Флаг состояния I2S / I2S state flag
bool is_i2s_enabled = false;

// Дескриптор задачи HID / HID task handle
static hid_task_handle_t hid_task = NULL;

// Дескриптор процессора команд / Command processor handle
static voice_command_processor_handle_t command_processor = NULL;

/**
 * @brief Callback для выполнения команд
 * Command execution callback
 */
static void command_execution_callback(const voice_command_t* command, void* user_data) {
    ESP_LOGI(TAG, "🎯 Executing command: '%s' -> %s (type: %d, action: %d)", 
             command->text, command->command, command->type, command->action);
    
    // Отправка команды в HID задачу / Send command to HID task
    if (hid_task) {
        hid_task_send_command(hid_task, command);
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "Voice Keyboard starting... / Голосовая клавиатура запускается...");
    
    // Сначала инициализируем GPIO / Initialize GPIO first
    ESP_ERROR_CHECK(gpio_init());
    
    // Инициализируем I2S / Initialize I2S
    ESP_ERROR_CHECK(i2s_init());
    
    // Инициализация HID / Initialize HID
    ESP_ERROR_CHECK(hid_task_init(&hid_task));
    
    // Инициализация процессора команд / Initialize command processor
    ESP_ERROR_CHECK(voice_command_processor_init(&command_processor));
    ESP_ERROR_CHECK(voice_command_processor_set_callback(command_processor, command_execution_callback, NULL));
    
    // Создаем задачу GPIO / Create GPIO task
    create_gpio_task();
    
    // Создаем задачу обработки аудио / Create audio processing task
    create_audio_task();
    
    // Запускаем HID задачу / Start HID task
    ESP_ERROR_CHECK(hid_task_start(hid_task));
    
    ESP_LOGI(TAG, "Voice Keyboard initialized successfully / Голосовая клавиатура успешно инициализирована");
    ESP_LOGI(TAG, "🎤 HID functionality ready - Voice commands will be converted to keyboard/mouse actions!");
    ESP_LOGI(TAG, "Press and hold button to record audio / Нажмите и удерживайте кнопку для записи аудио");
    
    // Основной цикл / Main loop
    while(1) {
        vTaskDelay(pdMS_TO_TICKS(5000));
        
        // Проверка статуса HID / Check HID status
        if (hid_task) {
            bool connected = hid_task_is_connected(hid_task);
            ESP_LOGD(TAG, "HID connected: %s", connected ? "yes" : "no");
            
            // Получение статистики / Get statistics
            hid_stats_t stats;
            if (hid_task_get_stats(hid_task, &stats) == ESP_OK) {
                ESP_LOGD(TAG, "HID stats: processed=%d, keyboard=%d, mouse=%d, media=%d, system=%d", 
                         stats.commands_processed, stats.keyboard_commands, stats.mouse_commands,
                         stats.media_commands, stats.system_commands);
            }
        }
        
        ESP_LOGD(TAG, "System running... / Система работает...");
    }
}