#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/i2s_std.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_system.h"

// I2S Configuration
#define I2S_SAMPLE_RATE     16000
#define I2S_BITS_PER_SAMPLE 16
#define I2S_CHANNEL_FORMAT  I2S_STD_MSB_SLOT_RIGHT
#define I2S_CHANNEL_NUM     1
#define I2S_BUFFER_SIZE     1024
#define I2S_DMA_FRAME_COUNT 256
#define I2S_DMA_BUFFER_SIZE (I2S_BUFFER_SIZE * I2S_DMA_FRAME_COUNT)

// GPIO Configuration (from CIRCUIT.md)
#define I2S_WS_PIN          GPIO_NUM_2  // Word Select
#define I2S_SD_PIN          GPIO_NUM_3  // Serial Data
#define I2S_SCK_PIN         GPIO_NUM_4  // Serial Clock
#define BUTTON_PIN          GPIO_NUM_0  // External button
#define LED_PIN             GPIO_NUM_1  // Status LED

// Audio buffer
static int16_t audio_buffer[I2S_BUFFER_SIZE];
static QueueHandle_t gpio_evt_queue = NULL;

static const char *TAG = "VOICE_KEYBOARD";

// I2S handle
i2s_chan_handle_t rx_handle;

// I2S state flag
static bool is_i2s_enabled = false;

// GPIO interrupt handler
static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

// GPIO task to handle button events
static void gpio_task(void* arg)
{
    uint32_t io_num;
    bool is_recording = false;
    
    for(;;) {
        if(xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
            // Button pressed (active LOW)
            if(io_num == BUTTON_PIN) {
                int level = gpio_get_level(BUTTON_PIN);
                
                if(level == 0 && !is_recording) {
                    // Start recording
                    is_recording = true;
                    is_i2s_enabled = true;
                    gpio_set_level(LED_PIN, 1);  // Turn LED on
                    ESP_LOGI(TAG, "Recording started");
                    
                    // Enable I2S
                    i2s_channel_enable(rx_handle);
                    
                } else if(level == 1 && is_recording) {
                    // Stop recording
                    is_recording = false;
                    is_i2s_enabled = false;
                    gpio_set_level(LED_PIN, 0);  // Turn LED off
                    ESP_LOGI(TAG, "Recording stopped");
                    
                    // Disable I2S
                    i2s_channel_disable(rx_handle);
                }
            }
        }
    }
}

// Initialize I2S for INMP441 microphone
static esp_err_t i2s_init(void)
{
    ESP_LOGI(TAG, "Initializing I2S for INMP441 microphone");
    
    // I2S configuration for INMP441
    i2s_std_config_t i2s_config = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(I2S_SAMPLE_RATE),
        .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,        // INMP441 doesn't use MCLK
            .bclk = I2S_SCK_PIN,           // Serial Clock
            .ws   = I2S_WS_PIN,            // Word Select
            .dout  = I2S_GPIO_UNUSED,       // Not used for input
            .din   = I2S_SD_PIN,           // Serial Data (microphone output)
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv   = false,
            },
        },
    };
    
    // Create I2S RX channel
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_ROLE_MASTER, I2S_DIR_RX);
    ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, NULL, &rx_handle));
    
    // Enable I2S channel
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(rx_handle, &i2s_config));
    
    ESP_LOGI(TAG, "I2S initialized successfully");
    return ESP_OK;
}

// Initialize GPIO for button and LED
static esp_err_t gpio_init(void)
{
    ESP_LOGI(TAG, "Initializing GPIO");
    
    // Button configuration (GPIO 0 - input with pull-up)
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << BUTTON_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_ANYEDGE,  // Interrupt on both edges
    };
    ESP_ERROR_CHECK(gpio_config(&io_conf));
    
    // LED configuration (GPIO 1 - output)
    io_conf.pin_bit_mask = (1ULL << LED_PIN);
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    ESP_ERROR_CHECK(gpio_config(&io_conf));
    
    // Turn LED off initially
    gpio_set_level(LED_PIN, 0);
    
    // Create queue for GPIO events
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    
    // Install GPIO interrupt service
    ESP_ERROR_CHECK(gpio_install_isr_service(0));
    ESP_ERROR_CHECK(gpio_isr_handler_add(BUTTON_PIN, gpio_isr_handler, (void*) BUTTON_PIN));
    
    ESP_LOGI(TAG, "GPIO initialized successfully");
    return ESP_OK;
}

// Audio processing task
static void audio_task(void* arg)
{
    size_t bytes_read;
    esp_err_t ret;
    
    ESP_LOGI(TAG, "Audio processing task started");
    
    for(;;) {
        // Check if I2S is enabled (recording)
        if(is_i2s_enabled) {
            // Read audio data from I2S
            ret = i2s_channel_read(rx_handle, audio_buffer, sizeof(audio_buffer), &bytes_read, portMAX_DELAY);
            
            if(ret == ESP_OK && bytes_read > 0) {
                // Process audio samples
                int samples_read = bytes_read / sizeof(int16_t);
                
                // Simple audio level detection for debugging
                int32_t sum = 0;
                for(int i = 0; i < samples_read; i++) {
                    sum += abs(audio_buffer[i]);
                }
                int avg_level = sum / samples_read;
                
                // Log audio level every 100 buffers
                static int buffer_count = 0;
                if(++buffer_count >= 100) {
                    ESP_LOGI(TAG, "Audio level: %d (samples: %d)", avg_level, samples_read);
                    buffer_count = 0;
                }
                
                // TODO: Store audio data for speech recognition
                // For now, we just process and discard
            }
        } else {
            // Not recording, wait a bit
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "Voice Keyboard starting...");
    
    // Initialize GPIO first
    ESP_ERROR_CHECK(gpio_init());
    
    // Initialize I2S
    ESP_ERROR_CHECK(i2s_init());
    
    // Create GPIO task
    xTaskCreate(gpio_task, "gpio_task", 2048, NULL, 10, NULL);
    
    // Create audio processing task
    xTaskCreate(audio_task, "audio_task", 4096, NULL, 5, NULL);
    
    ESP_LOGI(TAG, "Voice Keyboard initialized successfully");
    ESP_LOGI(TAG, "Press and hold button to record audio");
    
    // Main loop
    while(1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        ESP_LOGD(TAG, "System running...");
    }
}
