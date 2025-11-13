/**
 * @file speech_recognition.c
 * @brief Speech recognition framework implementation
 * @author Voice Keyboard Team
 * @date 2025
 * 
 * Реализация фреймворка распознавания речи
 * Implementation of speech recognition framework
 */

#include "speech_recognition.h"
#include "audio_processor.h"
#include "vad_detector.h"
#include <stdlib.h>
#include <math.h>

static const char* TAG = "SPEECH_RECOGNITION";

// Внутренняя структура распознавателя / Internal recognizer structure
struct speech_recognizer {
    speech_config_t config;
    speech_state_t state;
    speech_result_callback_t result_callback;
    void* user_data;
    
    // Компоненты обработки / Processing components
    audio_processor_handle_t audio_processor;
    vad_detector_handle_t vad_detector;
    
    // Буферы / Buffers
    int16_t* audio_buffer;
    size_t buffer_size;
    
    // Очередь результатов / Result queue
    QueueHandle_t result_queue;
    
    // Статистика / Statistics
    uint32_t total_frames_processed;
    uint32_t voice_frames_detected;
};

/**
 * @brief Обработчик обнаружения голосовой активности
 * Voice activity detection handler
 */
static void vad_event_handler(bool is_speaking, void* user_data) {
    speech_recognizer_handle_t handle = (speech_recognizer_handle_t)user_data;
    
    if (is_speaking) {
        ESP_LOGI(TAG, "Voice activity detected");
        handle->state = SPEECH_STATE_PROCESSING;
    } else {
        ESP_LOGI(TAG, "Voice activity ended");
        if (handle->state == SPEECH_STATE_PROCESSING) {
            // Генерация результата / Generate result
            speech_result_t result = {0};
            strncpy(result.text, "voice command detected", sizeof(result.text) - 1);
            result.confidence = 0.8f;
            result.is_final = true;
            
            if (handle->result_callback) {
                handle->result_callback(&result, handle->user_data);
            }
            
            // Отправка в очередь / Send to queue
            xQueueSend(handle->result_queue, &result, pdMS_TO_TICKS(100));
            
            handle->state = SPEECH_STATE_LISTENING;
        }
    }
}

esp_err_t speech_recognizer_init(speech_recognizer_handle_t* handle, const speech_config_t* config) {
    if (!handle || !config) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // Выделение памяти / Allocate memory
    *handle = malloc(sizeof(struct speech_recognizer));
    if (!*handle) {
        ESP_LOGE(TAG, "Failed to allocate memory for recognizer");
        return ESP_ERR_NO_MEM;
    }
    
    // Инициализация структуры / Initialize structure
    memset(*handle, 0, sizeof(struct speech_recognizer));
    (*handle)->config = *config;
    (*handle)->state = SPEECH_STATE_IDLE;
    
    // Выделение аудио буфера / Allocate audio buffer
    (*handle)->buffer_size = SPEECH_BUFFER_SIZE;
    (*handle)->audio_buffer = malloc((*handle)->buffer_size);
    if (!(*handle)->audio_buffer) {
        ESP_LOGE(TAG, "Failed to allocate audio buffer");
        free(*handle);
        return ESP_ERR_NO_MEM;
    }
    
    // Создание очереди результатов / Create result queue
    (*handle)->result_queue = xQueueCreate(5, sizeof(speech_result_t));
    if (!(*handle)->result_queue) {
        ESP_LOGE(TAG, "Failed to create result queue");
        free((*handle)->audio_buffer);
        free(*handle);
        return ESP_ERR_NO_MEM;
    }
    
    // Инициализация аудио процессора / Initialize audio processor
    audio_processor_config_t audio_config = {
        .sample_rate = SPEECH_SAMPLE_RATE,
        .enable_noise_reduction = config->enable_noise_reduction,
        .enable_agc = config->enable_agc,
        .target_rms = 0.1f
    };
    
    esp_err_t ret = audio_processor_init(&(*handle)->audio_processor, &audio_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize audio processor");
        vQueueDelete((*handle)->result_queue);
        free((*handle)->audio_buffer);
        free(*handle);
        return ret;
    }
    
    // Инициализация VAD детектора / Initialize VAD detector
    vad_config_t vad_config = {
        .threshold = SPEECH_VAD_THRESHOLD,
        .min_voice_frames = SPEECH_MIN_VOICE_FRAMES,
        .silence_frames_threshold = SPEECH_SILENCE_FRAMES,
        .sample_rate = SPEECH_SAMPLE_RATE
    };
    
    ret = vad_detector_init(&(*handle)->vad_detector, &vad_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize VAD detector");
        audio_processor_deinit((*handle)->audio_processor);
        vQueueDelete((*handle)->result_queue);
        free((*handle)->audio_buffer);
        free(*handle);
        return ret;
    }
    
    // Установка callback для VAD / Set VAD callback
    vad_detector_set_callback((*handle)->vad_detector, vad_event_handler, *handle);
    
    ESP_LOGI(TAG, "Speech recognizer initialized successfully");
    (*handle)->state = SPEECH_STATE_IDLE;
    
    return ESP_OK;
}

esp_err_t speech_recognizer_deinit(speech_recognizer_handle_t handle) {
    if (!handle) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // Остановка распознавания / Stop recognition
    speech_recognizer_stop(handle);
    
    // Деинициализация компонентов / Deinitialize components
    if (handle->audio_processor) {
        audio_processor_deinit(handle->audio_processor);
    }
    
    if (handle->vad_detector) {
        vad_detector_deinit(handle->vad_detector);
    }
    
    // Очистка ресурсов / Cleanup resources
    if (handle->result_queue) {
        vQueueDelete(handle->result_queue);
    }
    
    if (handle->audio_buffer) {
        free(handle->audio_buffer);
    }
    
    free(handle);
    ESP_LOGI(TAG, "Speech recognizer deinitialized");
    
    return ESP_OK;
}

esp_err_t speech_recognizer_start(speech_recognizer_handle_t handle) {
    if (!handle) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (handle->state != SPEECH_STATE_IDLE) {
        ESP_LOGW(TAG, "Recognizer already started");
        return ESP_ERR_INVALID_STATE;
    }
    
    handle->state = SPEECH_STATE_LISTENING;
    handle->total_frames_processed = 0;
    handle->voice_frames_detected = 0;
    
    ESP_LOGI(TAG, "Speech recognition started");
    return ESP_OK;
}

esp_err_t speech_recognizer_stop(speech_recognizer_handle_t handle) {
    if (!handle) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (handle->state == SPEECH_STATE_IDLE) {
        ESP_LOGW(TAG, "Recognizer already stopped");
        return ESP_ERR_INVALID_STATE;
    }
    
    handle->state = SPEECH_STATE_IDLE;
    
    // Очистка очереди результатов / Clear result queue
    xQueueReset(handle->result_queue);
    
    ESP_LOGI(TAG, "Speech recognition stopped");
    return ESP_OK;
}

esp_err_t speech_recognizer_process_audio(speech_recognizer_handle_t handle, 
                                         const int16_t* audio_data, size_t audio_size) {
    if (!handle || !audio_data || audio_size == 0) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (handle->state == SPEECH_STATE_IDLE || handle->state == SPEECH_STATE_ERROR) {
        return ESP_ERR_INVALID_STATE;
    }
    
    // Предобработка аудио / Audio preprocessing
    esp_err_t ret = audio_processor_process(handle->audio_processor, audio_data, audio_size);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Audio preprocessing failed");
        return ret;
    }
    
    // Обнаружение голосовой активности / Voice activity detection
    vad_detector_process_audio(handle->vad_detector, audio_data, audio_size / sizeof(int16_t));
    
    handle->total_frames_processed++;
    
    return ESP_OK;
}

esp_err_t speech_recognizer_get_result(speech_recognizer_handle_t handle, speech_result_t* result) {
    if (!handle || !result) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // Получение результата из очереди / Get result from queue
    if (xQueueReceive(handle->result_queue, result, pdMS_TO_TICKS(100)) == pdTRUE) {
        return ESP_OK;
    }
    
    return ESP_ERR_TIMEOUT;
}

speech_state_t speech_recognizer_get_state(speech_recognizer_handle_t handle) {
    if (!handle) {
        return SPEECH_STATE_ERROR;
    }
    
    return handle->state;
}

esp_err_t speech_recognizer_set_callback(speech_recognizer_handle_t handle, 
                                        speech_result_callback_t callback, void* user_data) {
    if (!handle) {
        return ESP_ERR_INVALID_ARG;
    }
    
    handle->result_callback = callback;
    handle->user_data = user_data;
    
    return ESP_OK;
}