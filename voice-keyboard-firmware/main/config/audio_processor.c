/**
 * @file audio_processor.c
 * @brief Audio processing component implementation
 * @author Voice Keyboard Team
 * @date 2025
 * 
 * Реализация компонента обработки аудио
 * Implementation of audio processing component
 */

#include "audio_processor.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "esp_log.h"

static const char* TAG = "AUDIO_PROCESSOR";

// Внутренняя структура аудио процессора / Internal audio processor structure
struct audio_processor {
    audio_processor_config_t config;
    
    // Фильтры / Filters
    float* hp_coeffs;           // Коэффициенты ВЧ фильтра / High-pass filter coefficients
    float* hp_states;           // Состояния ВЧ фильтра / High-pass filter states
    
    // AGC параметры / AGC parameters
    float agc_gain;             // Текущий коэффициент усиления / Current gain
    float agc_envelope;         // Огибающая сигнала / Signal envelope
    float agc_attack_time;      // Время атаки / Attack time
    float agc_release_time;     // Время релиза / Release time
    
    // Статистика / Statistics
    audio_stats_t stats;
    uint32_t samples_processed;
    
    // Временные буферы / Temporary buffers
    float* float_buffer;
    size_t buffer_size;
};

/**
 * @brief Рассчитать коэффициенты фильтра Баттерворта
 * Calculate Butterworth filter coefficients
 */
static void calculate_butterworth_coeffs(float* coeffs, float cutoff_rad, int order) {
    // Упрощенный расчет для 4-го порядка / Simplified calculation for 4th order
    if (order == 4) {
        float w = cutoff_rad;
        float w2 = w * w;
        float sqrt2 = sqrtf(2.0f);
        
        // Нормализованные коэффициенты / Normalized coefficients
        coeffs[0] = 1.0f;
        coeffs[1] = -4.0f;
        coeffs[2] = 6.0f - 2.0f * w2;
        coeffs[3] = -4.0f + 2.0f * w2;
        coeffs[4] = 1.0f - sqrt2 * w + w2;
        
        // Нормализация / Normalization
        float sum = 0.0f;
        for (int i = 0; i <= order; i++) {
            sum += fabsf(coeffs[i]);
        }
        if (sum > 0.0f) {
            for (int i = 0; i <= order; i++) {
                coeffs[i] /= sum;
            }
        }
    }
}

/**
 * @brief Применить ВЧ фильтр
 * Apply high-pass filter
 */
static void apply_high_pass_filter(struct audio_processor* proc, float* audio, size_t length) {
    for (size_t i = 0; i < length; i++) {
        float input = audio[i];
        float output = proc->hp_coeffs[0] * input;
        
        // Применение фильтра / Apply filter
        for (int j = 1; j <= proc->config.filter_order; j++) {
            output += proc->hp_coeffs[j] * proc->hp_states[j - 1];
            if (j < proc->config.filter_order) {
                proc->hp_states[j - 1] = proc->hp_states[j];
            }
        }
        
        proc->hp_states[proc->config.filter_order - 1] = input;
        audio[i] = output;
    }
}

/**
 * @brief Применить AGC
 * Apply AGC
 */
static void apply_agc(struct audio_processor* proc, float* audio, size_t length) {
    const float SAMPLE_RATE = (float)proc->config.sample_rate;
    
    for (size_t i = 0; i < length; i++) {
        float sample = audio[i];
        float abs_sample = fabsf(sample);
        
        // Обновление огибающей / Update envelope
        float time_constant = (abs_sample > proc->agc_envelope) ? 
                              proc->agc_attack_time : proc->agc_release_time;
        float alpha = expf(-1.0f / (time_constant * SAMPLE_RATE));
        proc->agc_envelope = alpha * proc->agc_envelope + (1.0f - alpha) * abs_sample;
        
        // Расчет усиления / Calculate gain
        if (proc->agc_envelope > 0.001f) {
            float target_gain = proc->config.target_rms / proc->agc_envelope;
            float gain_alpha = 0.001f;
            proc->agc_gain = gain_alpha * target_gain + (1.0f - gain_alpha) * proc->agc_gain;
            proc->agc_gain = fmaxf(0.1f, fminf(10.0f, proc->agc_gain));
        }
        
        audio[i] = sample * proc->agc_gain;
    }
}

/**
 * @brief Обновить статистику
 * Update statistics
 */
static void update_stats(struct audio_processor* proc, const int16_t* audio, size_t length) {
    float sum = 0.0f;
    float sum_sq = 0.0f;
    float peak = 0.0f;
    int clipped = 0;
    
    for (size_t i = 0; i < length; i++) {
        float sample = (float)audio[i] / 32768.0f;
        sum += sample;
        sum_sq += sample * sample;
        peak = fmaxf(peak, fabsf(sample));
        
        if (audio[i] == 32767 || audio[i] == -32768) {
            clipped++;
        }
    }
    
    proc->stats.dc_offset = sum / length;
    proc->stats.rms_level = sqrtf(sum_sq / length);
    proc->stats.peak_level = peak;
    proc->stats.clipped_samples += clipped;
    
    proc->samples_processed += length;
}

esp_err_t audio_processor_init(audio_processor_handle_t* handle, const audio_processor_config_t* config) {
    if (!handle || !config) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // Выделение памяти / Allocate memory
    *handle = malloc(sizeof(struct audio_processor));
    if (!*handle) {
        ESP_LOGE(TAG, "Failed to allocate memory for audio processor");
        return ESP_ERR_NO_MEM;
    }
    
    // Инициализация структуры / Initialize structure
    memset(*handle, 0, sizeof(struct audio_processor));
    (*handle)->config = *config;
    
    // Установка параметров по умолчанию / Set default parameters
    if ((*handle)->config.filter_order == 0) {
        (*handle)->config.filter_order = 4;
    }
    if ((*handle)->config.high_pass_cutoff == 0.0f) {
        (*handle)->config.high_pass_cutoff = 80.0f;
    }
    if ((*handle)->agc_attack_time == 0.0f) {
        (*handle)->agc_attack_time = 0.001f;
    }
    if ((*handle)->agc_release_time == 0.0f) {
        (*handle)->agc_release_time = 0.1f;
    }
    
    // Выделение памяти для фильтра / Allocate filter memory
    (*handle)->hp_coeffs = malloc(((*handle)->config.filter_order + 1) * sizeof(float));
    (*handle)->hp_states = malloc((*handle)->config.filter_order * sizeof(float));
    
    if (!(*handle)->hp_coeffs || !(*handle)->hp_states) {
        ESP_LOGE(TAG, "Failed to allocate filter memory");
        if ((*handle)->hp_coeffs) free((*handle)->hp_coeffs);
        if ((*handle)->hp_states) free((*handle)->hp_states);
        free(*handle);
        return ESP_ERR_NO_MEM;
    }
    
    // Расчет коэффициентов фильтра / Calculate filter coefficients
    float cutoff_rad = 2.0f * M_PI * (*handle)->config.high_pass_cutoff / (*handle)->config.sample_rate;
    calculate_butterworth_coeffs((*handle)->hp_coeffs, cutoff_rad, (*handle)->config.filter_order);
    memset((*handle)->hp_states, 0, (*handle)->config.filter_order * sizeof(float));
    
    // Инициализация AGC / Initialize AGC
    (*handle)->agc_gain = 1.0f;
    (*handle)->agc_envelope = 0.0f;
    
    // Выделение временного буфера / Allocate temporary buffer
    (*handle)->buffer_size = 1024; // Базовый размер / Base size
    (*handle)->float_buffer = malloc((*handle)->buffer_size * sizeof(float));
    if (!(*handle)->float_buffer) {
        ESP_LOGE(TAG, "Failed to allocate float buffer");
        free((*handle)->hp_coeffs);
        free((*handle)->hp_states);
        free(*handle);
        return ESP_ERR_NO_MEM;
    }
    
    // Сброс статистики / Reset statistics
    memset(&(*handle)->stats, 0, sizeof(audio_stats_t));
    (*handle)->samples_processed = 0;
    
    ESP_LOGI(TAG, "Audio processor initialized: sample_rate=%d, noise_reduction=%s, agc=%s",
             config->sample_rate,
             config->enable_noise_reduction ? "enabled" : "disabled",
             config->enable_agc ? "enabled" : "disabled");
    
    return ESP_OK;
}

esp_err_t audio_processor_deinit(audio_processor_handle_t handle) {
    if (!handle) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // Освобождение памяти / Free memory
    if (handle->hp_coeffs) free(handle->hp_coeffs);
    if (handle->hp_states) free(handle->hp_states);
    if (handle->float_buffer) free(handle->float_buffer);
    
    free(handle);
    ESP_LOGI(TAG, "Audio processor deinitialized");
    
    return ESP_OK;
}

esp_err_t audio_processor_process(audio_processor_handle_t handle, const int16_t* input_data, size_t input_size) {
    if (!handle || !input_data || input_size == 0) {
        return ESP_ERR_INVALID_ARG;
    }
    
    size_t sample_count = input_size / sizeof(int16_t);
    
    // Проверка размера буфера / Check buffer size
    if (sample_count > handle->buffer_size) {
        // Перевыделение буфера / Reallocate buffer
        float* new_buffer = realloc(handle->float_buffer, sample_count * sizeof(float));
        if (!new_buffer) {
            ESP_LOGE(TAG, "Failed to reallocate float buffer");
            return ESP_ERR_NO_MEM;
        }
        handle->float_buffer = new_buffer;
        handle->buffer_size = sample_count;
    }
    
    // Конвертация в float / Convert to float
    for (size_t i = 0; i < sample_count; i++) {
        handle->float_buffer[i] = (float)input_data[i] / 32768.0f;
    }
    
    // Обработка / Processing
    if (handle->config.enable_noise_reduction) {
        apply_high_pass_filter(handle, handle->float_buffer, sample_count);
    }
    
    if (handle->config.enable_agc) {
        apply_agc(handle, handle->float_buffer, sample_count);
    }
    
    // Конвертация обратно в int16_t / Convert back to int16_t
    for (size_t i = 0; i < sample_count; i++) {
        float sample = handle->float_buffer[i] * 32768.0f;
        sample = fmaxf(-32768.0f, fminf(32767.0f, sample));
        ((int16_t*)input_data)[i] = (int16_t)sample;
    }
    
    // Обновление статистики / Update statistics
    update_stats(handle, input_data, sample_count);
    
    return ESP_OK;
}

esp_err_t audio_processor_get_stats(audio_processor_handle_t handle, audio_stats_t* stats) {
    if (!handle || !stats) {
        return ESP_ERR_INVALID_ARG;
    }
    
    *stats = handle->stats;
    return ESP_OK;
}

esp_err_t audio_processor_reset_stats(audio_processor_handle_t handle) {
    if (!handle) {
        return ESP_ERR_INVALID_ARG;
    }
    
    memset(&handle->stats, 0, sizeof(audio_stats_t));
    handle->samples_processed = 0;
    
    return ESP_OK;
}