#ifndef GPIO_CONFIG_H
#define GPIO_CONFIG_H

#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

// Initialize GPIO for button and LED
esp_err_t gpio_init(void);

// Get GPIO event queue handle
QueueHandle_t get_gpio_evt_queue(void);

// Set LED state
void set_led_state(bool state);

// Get button state
int get_button_state(void);

#endif // GPIO_CONFIG_H