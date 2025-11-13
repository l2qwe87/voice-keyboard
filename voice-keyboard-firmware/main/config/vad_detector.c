/**
 * @file vad_detector.c
 * @brief Voice Activity Detection component implementation
 * @author Voice Keyboard Team
 * @date 2025
 * 
 * Реализация компонента обнаружения голосовой активности
 * Implementation of Voice Activity Detection component
 */

#include "vad_detector.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "esp_log.h"

static const char* TAG = "VAD_DETECTOR";

// Внутренняя структура VAD детектора / Internal VAD detector structure
struct vad_detector {
    vad_config_t config;
    
    // Состояние / State
    bool is_speaking;
    int voice_frame_count;
    int silence_frame_count;
    
    // Callback / Callback
    vad_event_callback_t event_callback;
    void* user_data;
    
    // Статистика / Statistics
    vad_stats_t stats;
    float energy_sum;
    uint32_t energy_count;
    
    // Адаптивный порог / Adaptive threshold
    float adaptive_threshold;
    float noise_floor;
    bool noise_estimated;
};

/**
 * @brief Рассчитать энергию сигнала
 * Calculate signal energy
 */
static float calculate_energy(const int16_t* audio, size_t length) {
    float sum = 0.0f;
    
    for (size_t i = 0; i < length; i++) {
        float sample = (float)audio[i] / 32768.0f;
        sum += sample * sample;
    }
    
    return sqrtf(sum / length);
}

/**
 * @brief Обновить адаптивный порог
 * Update adaptive threshold
 */
static void update_adaptive_threshold(struct vad_detector* detector, float energy) {
    const float ALPHA = 0.95f;  // Коэффициент сглаживания / Smoothing coefficient
    const float THRESHOLD_MULTIPLIER = 3.0f;  // Множитель порога / Threshold multiplier
    
    if (!detector->noise_estimated) {
        // Оценка уровня шума в начале / Initial noise estimation
        detector->energy_sum += energy;
        detector->energy_count++;
        
        if (detector->energy_count >= 100) {  // 100 кадров для оценки / 100 frames for estimation
            detector->noise_floor = detector->energy_sum / detector->energy_count;
            detector->adaptive_threshold = detector->noise_floor * THRESHOLD_MULTIPLIER;
            detector->noise_estimated = true;
            
            ESP_LOGI(TAG, "Noise floor estimated: %.6f, threshold: %.6f", 
                     detector->noise_floor, detector->adaptive_threshold);
        }
    } else {
        // Адаптивное обновление порога / Adaptive threshold update
        if (energy < detector->noise_floor * 2.0f) {
            // Обновление уровня шума / Update noise floor
            detector->noise_floor = ALPHA * detector->noise_floor + (1.0f - ALPHA) * energy;
            detector->adaptive_threshold = detector->noise_floor * THRESHOLD_MULTIPLIER;
        }
    }
}

/**
 * @brief Сгенерировать событие
 * Generate event
 */
static void generate_event(struct vad_detector* detector, bool is_speaking) {
    if (detector->event_callback) {
        detector->event_callback(is_speaking, detector->user_data);
    }
}

esp_err_t vad_detector_init(vad_detector_handle_t* handle, const vad_config_t* config) {
    if (!handle || !config) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // Выделение памяти / Allocate memory
    *handle = malloc(sizeof(struct vad_detector));
    if (!*handle) {
        ESP_LOGE(TAG, "Failed to allocate memory for VAD detector");
        return ESP_ERR_NO_MEM;
    }
    
    // Инициализация структуры / Initialize structure
    memset(*handle, 0, sizeof(struct vad_detector));
    (*handle)->config = *config;
    
    // Установка параметров по умолчанию / Set default parameters
    if ((*handle)->config.frame_size == 0) {
        (*handle)->config.frame_size = 160;  // 10ms при 16kHz / 10ms at 16kHz
    }
    
    // Инициализация состояния / Initialize state
    (*handle)->is_speaking = false;
    (*handle)->voice_frame_count = 0;
    (*handle)->silence_frame_count = 0;
    (*handle)->adaptive_threshold = (*handle)->config.threshold;
    (*handle)->noise_estimated = false;
    
    ESP_LOGI(TAG, "VAD detector initialized: threshold=%.6f, min_voice_frames=%d, silence_frames=%d",
             config->threshold, config->min_voice_frames, config->silence_frames_threshold);
    
    return ESP_OK;
}

esp_err_t vad_detector_deinit(vad_detector_handle_t handle) {
    if (!handle) {
        return ESP_ERR_INVALID_ARG;
    }
    
    free(handle);
    ESP_LOGI(TAG, "VAD detector deinitialized");
    
    return ESP_OK;
}

esp_err_t vad_detector_process_audio(vad_detector_handle_t handle, const int16_t* audio_data, size_t sample_count) {
    if (!handle || !audio_data || sample_count == 0) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // Расчет энергии / Calculate energy
    float energy = calculate_energy(audio_data, sample_count);
    handle->stats.current_energy = energy;
    
    // Обновление адаптивного порога / Update adaptive threshold
    update_adaptive_threshold(handle, energy);
    
    // Обновление статистики / Update statistics
    handle->stats.total_frames++;
    handle->energy_sum += energy;
    handle->energy_count++;
    handle->stats.average_energy = handle->energy_sum / handle->energy_count;
    
    // Определение голосовой активности / Determine voice activity
    float threshold = handle->noise_estimated ? handle->adaptive_threshold : handle->config.threshold;
    bool voice_detected = energy > threshold;
    
    if (voice_detected) {
        handle->voice_frame_count++;
        handle->silence_frame_count = 0;
        handle->stats.voice_frames++;
        
        if (!handle->is_speaking && handle->voice_frame_count >= handle->config.min_voice_frames) {
            handle->is_speaking = true;
            ESP_LOGD(TAG, "Speech started (energy: %.6f > %.6f)", energy, threshold);
            generate_event(handle, true);
        }
    } else {
        handle->silence_frame_count++;
        handle->voice_frame_count = 0;
        handle->stats.silence_frames++;
        
        if (handle->is_speaking && handle->silence_frame_count >= handle->config.silence_frames_threshold) {
            handle->is_speaking = false;
            ESP_LOGD(TAG, "Speech ended (energy: %.6f < %.6f)", energy, threshold);
            generate_event(handle, false);
        }
    }
    
    return ESP_OK;
}

esp_err_t vad_detector_set_callback(vad_detector_handle_t handle, vad_event_callback_t callback, void* user_data) {
    if (!handle) {
        return ESP_ERR_INVALID_ARG;
    }
    
    handle->event_callback = callback;
    handle->user_data = user_data;
    
    return ESP_OK;
}

bool vad_detector_is_speaking(vad_detector_handle_t handle) {
    if (!handle) {
        return false;
    }
    
    return handle->is_speaking;
}

esp_err_t vad_detector_get_stats(vad_detector_handle_t handle, vad_stats_t* stats) {
    if (!handle || !stats) {
        return ESP_ERR_INVALID_ARG;
    }
    
    *stats = handle->stats;
    return ESP_OK;
}

esp_err_t vad_detector_reset_stats(vad_detector_handle_t handle) {
    if (!handle) {
        return ESP_ERR_INVALID_ARG;
    }
    
    memset(&handle->stats, 0, sizeof(vad_stats_t));
    handle->energy_sum = 0.0f;
    handle->energy_count = 0;
    
    return ESP_OK;
}