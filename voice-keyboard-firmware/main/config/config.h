#ifndef CONFIG_H
#define CONFIG_H

#include "driver/gpio.h"

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

// Task Configuration
#define GPIO_TASK_STACK_SIZE    2048
#define GPIO_TASK_PRIORITY      10
#define AUDIO_TASK_STACK_SIZE   4096
#define AUDIO_TASK_PRIORITY     5
#define SPEECH_TASK_STACK_SIZE  4096
#define SPEECH_TASK_PRIORITY    5

// Audio Processing
#define AUDIO_LEVEL_LOG_INTERVAL 100  // Log every N buffers

#endif // CONFIG_H