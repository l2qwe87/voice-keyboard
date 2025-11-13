/**
 * @file audio_processor.h
 * @brief Audio processing component header
 * @author Voice Keyboard Team
 * @date 2025
 * 
 * Заголовочный файл компонента обработки аудио
 * Header file for audio processing component
 */

#ifndef AUDIO_PROCESSOR_H
#define AUDIO_PROCESSOR_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

// Конфигурация аудио процессора / Audio processor configuration
typedef struct {
    int sample_rate;              // Частота дискретизации / Sample rate
    bool enable_noise_reduction;  // Шумоподавление / Noise reduction
    bool enable_agc;             // Автоматическая регулировка усиления / AGC
    float target_rms;            // Целевой RMS уровень / Target RMS level
    int filter_order;            // Порядок фильтра / Filter order
    float high_pass_cutoff;      // Частота среза ВЧ фильтра / High-pass cutoff
} audio_processor_config_t;

// Дескриптор аудио процессора / Audio processor handle
typedef struct audio_processor* audio_processor_handle_t;

/**
 * @brief Инициализация аудио процессора
 * Initialize audio processor
 */
esp_err_t audio_processor_init(audio_processor_handle_t* handle, const audio_processor_config_t* config);

/**
 * @brief Деинициализация аудио процессора
 * Deinitialize audio processor
 */
esp_err_t audio_processor_deinit(audio_processor_handle_t handle);

/**
 * @brief Обработать аудио данные
 * Process audio data
 */
esp_err_t audio_processor_process(audio_processor_handle_t handle, const int16_t* input_data, size_t input_size);

/**
 * @brief Получить статистику обработки
 * Get processing statistics
 */
typedef struct {
    float peak_level;      // Пиковый уровень / Peak level
    float rms_level;       // RMS уровень / RMS level
    int clipped_samples;   // Количество клиппированных сэмплов / Clipped samples count
    float dc_offset;       // DC смещение / DC offset
} audio_stats_t;

esp_err_t audio_processor_get_stats(audio_processor_handle_t handle, audio_stats_t* stats);

/**
 * @brief Сбросить статистику
 * Reset statistics
 */
esp_err_t audio_processor_reset_stats(audio_processor_handle_t handle);

#endif // AUDIO_PROCESSOR_H