#include "audio_task.h"
#include "config/config.h"
#include "config/i2s_config.h"
#include "esp_log.h"

static const char *TAG = "AUDIO_TASK";

// Внешний флаг состояния I2S / External I2S state flag
extern bool is_i2s_enabled;

// Аудиобуфер / Audio buffer
static int16_t audio_buffer[I2S_BUFFER_SIZE];

// Задача обработки аудио / Audio processing task
static void audio_task_impl(void* arg)
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

void create_audio_task(void)
{
    xTaskCreate(audio_task_impl, "audio_task", AUDIO_TASK_STACK_SIZE, NULL, AUDIO_TASK_PRIORITY, NULL);
}