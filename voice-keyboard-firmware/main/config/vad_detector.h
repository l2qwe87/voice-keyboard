/**
 * @file vad_detector.h
 * @brief Voice Activity Detection component header
 * @author Voice Keyboard Team
 * @date 2025
 * 
 * Заголовочный файл компонента обнаружения голосовой активности
 * Header file for Voice Activity Detection component
 */

#ifndef VAD_DETECTOR_H
#define VAD_DETECTOR_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

// Конфигурация VAD детектора / VAD detector configuration
typedef struct {
    float threshold;                // Порог энергии / Energy threshold
    int min_voice_frames;          // Минимальное количество речевых кадров / Min voice frames
    int silence_frames_threshold;  // Порог тишины для завершения / Silence frames threshold
    int sample_rate;               // Частота дискретизации / Sample rate
    int frame_size;                // Размер кадра / Frame size
} vad_config_t;

// Дескриптор VAD детектора / VAD detector handle
typedef struct vad_detector* vad_detector_handle_t;

/**
 * @brief Callback для событий VAD
 * VAD event callback
 */
typedef void (*vad_event_callback_t)(bool is_speaking, void* user_data);

/**
 * @brief Инициализация VAD детектора
 * Initialize VAD detector
 */
esp_err_t vad_detector_init(vad_detector_handle_t* handle, const vad_config_t* config);

/**
 * @brief Деинициализация VAD детектора
 * Deinitialize VAD detector
 */
esp_err_t vad_detector_deinit(vad_detector_handle_t handle);

/**
 * @brief Обработать аудио данные
 * Process audio data
 */
esp_err_t vad_detector_process_audio(vad_detector_handle_t handle, const int16_t* audio_data, size_t sample_count);

/**
 * @brief Установить callback для событий
 * Set event callback
 */
esp_err_t vad_detector_set_callback(vad_detector_handle_t handle, vad_event_callback_t callback, void* user_data);

/**
 * @brief Получить текущее состояние
 * Get current state
 */
bool vad_detector_is_speaking(vad_detector_handle_t handle);

/**
 * @brief Получить статистику
 * Get statistics
 */
typedef struct {
    uint32_t total_frames;        // Всего кадров обработано / Total frames processed
    uint32_t voice_frames;        // Речевых кадров обнаружено / Voice frames detected
    uint32_t silence_frames;      // Кадров тишины / Silence frames
    float current_energy;         // Текущая энергия / Current energy
    float average_energy;         // Средняя энергия / Average energy
} vad_stats_t;

esp_err_t vad_detector_get_stats(vad_detector_handle_t handle, vad_stats_t* stats);

/**
 * @brief Сбросить статистику
 * Reset statistics
 */
esp_err_t vad_detector_reset_stats(vad_detector_handle_t handle);

#endif // VAD_DETECTOR_H