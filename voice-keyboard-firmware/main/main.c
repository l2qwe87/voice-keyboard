#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "config/config.h"
#include "config/i2s_config.h"
#include "config/gpio_config.h"

static const char *TAG = "VOICE_KEYBOARD";

// Флаг состояния I2S / I2S state flag
static bool is_i2s_enabled = false;

// Аудиобуфер / Audio buffer
static int16_t audio_buffer[I2S_BUFFER_SIZE];

// Задача GPIO для обработки событий кнопки / GPIO task to handle button events
static void gpio_task(void* arg)
{
    uint32_t io_num;
    bool is_recording = false;
    QueueHandle_t gpio_evt_queue = get_gpio_evt_queue();
    
    for(;;) {
        if(xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
            // Нажатие кнопки (активный LOW) / Button pressed (active LOW)
            if(io_num == BUTTON_PIN) {
                int level = get_button_state();
                
                if(level == 0 && !is_recording) {
                    // Начать запись / Start recording
                    is_recording = true;
                    is_i2s_enabled = true;
                    set_led_state(true);  // Включить LED / Turn LED on
                    ESP_LOGI(TAG, "Recording started / Запись начата");
                    
                    // Включить I2S / Enable I2S
                    i2s_enable();
                    
                } else if(level == 1 && is_recording) {
                    // Остановить запись / Stop recording
                    is_recording = false;
                    is_i2s_enabled = false;
                    set_led_state(false);  // Выключить LED / Turn LED off
                    ESP_LOGI(TAG, "Recording stopped / Запись остановлена");
                    
                    // Выключить I2S / Disable I2S
                    i2s_disable();
                }
            }
        }
    }
}

// Задача обработки аудио / Audio processing task
static void audio_task(void* arg)
{
    size_t bytes_read;
    esp_err_t ret;
    i2s_chan_handle_t rx_handle = get_i2s_rx_handle();
    
    ESP_LOGI(TAG, "Audio processing task started / Задача обработки аудио запущена");
    
    for(;;) {
        // Проверить включен ли I2S (запись) / Check if I2S is enabled (recording)
        if(is_i2s_enabled) {
            // Чтение аудиоданных из I2S / Read audio data from I2S
            ret = i2s_channel_read(rx_handle, audio_buffer, sizeof(audio_buffer), &bytes_read, portMAX_DELAY);
            
            if(ret == ESP_OK && bytes_read > 0) {
                // Обработка аудио сэмплов / Process audio samples
                int samples_read = bytes_read / sizeof(int16_t);
                
                // Простое определение уровня аудио для отладки / Simple audio level detection for debugging
                int32_t sum = 0;
                for(int i = 0; i < samples_read; i++) {
                    sum += abs(audio_buffer[i]);
                }
                int avg_level = sum / samples_read;
                
                // Логирование уровня аудио каждые N буферов / Log audio level every N buffers
                static int buffer_count = 0;
                if(++buffer_count >= AUDIO_LEVEL_LOG_INTERVAL) {
                    ESP_LOGI(TAG, "Audio level: %d (samples: %d) / Уровень аудио: %d (сэмплов: %d)", avg_level, samples_read);
                    buffer_count = 0;
                }
                
                // TODO: Сохранить аудиоданные для распознавания речи / TODO: Store audio data for speech recognition
                // Сейчас просто обрабатываем и отбрасываем / For now, we just process and discard
            }
        } else {
            // Не записываем, немного ждем / Not recording, wait a bit
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "Voice Keyboard starting... / Голосовая клавиатура запускается...");
    
    // Сначала инициализируем GPIO / Initialize GPIO first
    ESP_ERROR_CHECK(gpio_init());
    
    // Инициализируем I2S / Initialize I2S
    ESP_ERROR_CHECK(i2s_init());
    
    // Создаем задачу GPIO / Create GPIO task
    xTaskCreate(gpio_task, "gpio_task", GPIO_TASK_STACK_SIZE, NULL, GPIO_TASK_PRIORITY, NULL);
    
    // Создаем задачу обработки аудио / Create audio processing task
    xTaskCreate(audio_task, "audio_task", AUDIO_TASK_STACK_SIZE, NULL, AUDIO_TASK_PRIORITY, NULL);
    
    ESP_LOGI(TAG, "Voice Keyboard initialized successfully / Голосовая клавиатура успешно инициализирована");
    ESP_LOGI(TAG, "Press and hold button to record audio / Нажмите и удерживайте кнопку для записи аудио");
    
    // Основной цикл / Main loop
    while(1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        ESP_LOGD(TAG, "System running... / Система работает...");
    }
}