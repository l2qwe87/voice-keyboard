#include "i2s_config.h"
#include "config.h"
#include "esp_log.h"

static const char *TAG = "I2S_CONFIG";

// I2S handle
static i2s_chan_handle_t rx_handle;

esp_err_t i2s_init(void)
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

i2s_chan_handle_t get_i2s_rx_handle(void)
{
    return rx_handle;
}

esp_err_t i2s_enable(void)
{
    esp_err_t ret = i2s_channel_enable(rx_handle);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "I2S channel enabled");
    } else {
        ESP_LOGE(TAG, "Failed to enable I2S channel: %s", esp_err_to_name(ret));
    }
    return ret;
}

esp_err_t i2s_disable(void)
{
    esp_err_t ret = i2s_channel_disable(rx_handle);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "I2S channel disabled");
    } else {
        ESP_LOGE(TAG, "Failed to disable I2S channel: %s", esp_err_to_name(ret));
    }
    return ret;
}