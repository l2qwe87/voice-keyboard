#include "gpio_task.h"
#include "config/config.h"
#include "config/gpio_config.h"
#include "config/i2s_config.h"
#include "esp_log.h"

static const char *TAG = "GPIO_TASK";

// Внешний флаг состояния I2S / External I2S state flag
extern bool is_i2s_enabled;

// Задача GPIO для обработки событий кнопки / GPIO task to handle button events
static void gpio_task_impl(void* arg)
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

void create_gpio_task(void)
{
    xTaskCreate(gpio_task_impl, "gpio_task", GPIO_TASK_STACK_SIZE, NULL, GPIO_TASK_PRIORITY, NULL);
}