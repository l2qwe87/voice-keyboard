#ifndef I2S_CONFIG_H
#define I2S_CONFIG_H

#include "esp_err.h"
#include "driver/i2s_std.h"

// Initialize I2S for INMP441 microphone
esp_err_t i2s_init(void);

// Get I2S RX channel handle
i2s_chan_handle_t get_i2s_rx_handle(void);

// Enable I2S channel
esp_err_t i2s_enable(void);

// Disable I2S channel
esp_err_t i2s_disable(void);

#endif // I2S_CONFIG_H