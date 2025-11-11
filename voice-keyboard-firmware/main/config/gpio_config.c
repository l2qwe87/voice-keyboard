#include "gpio_config.h"
#include "config.h"
#include "esp_log.h"

static const char *TAG = "GPIO_CONFIG";

// GPIO event queue
static QueueHandle_t gpio_evt_queue = NULL;

// GPIO interrupt handler
static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

esp_err_t gpio_init(void)
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
    if (gpio_evt_queue == NULL) {
        ESP_LOGE(TAG, "Failed to create GPIO event queue");
        return ESP_FAIL;
    }
    
    // Install GPIO interrupt service
    ESP_ERROR_CHECK(gpio_install_isr_service(0));
    ESP_ERROR_CHECK(gpio_isr_handler_add(BUTTON_PIN, gpio_isr_handler, (void*) BUTTON_PIN));
    
    ESP_LOGI(TAG, "GPIO initialized successfully");
    return ESP_OK;
}

QueueHandle_t get_gpio_evt_queue(void)
{
    return gpio_evt_queue;
}

void set_led_state(bool state)
{
    gpio_set_level(LED_PIN, state ? 1 : 0);
}

int get_button_state(void)
{
    return gpio_get_level(BUTTON_PIN);
}