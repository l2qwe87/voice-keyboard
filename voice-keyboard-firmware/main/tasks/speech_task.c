/**
 * @file speech_task.c
 * @brief Speech recognition task implementation
 * @author Voice Keyboard Team
 * @date 2025
 * 
 * Реализация задачи распознавания речи
 * Implementation of speech recognition task
 */

#include "speech_task.h"
#include "../voice_commands.h"
#include <stdlib.h>
#include <string.h>
#include "esp_log.h"

static const char* TAG = "SPEECH_TASK";

// Глобальные переменные для простой реализации / Global variables for simple implementation
static speech_recognizer_handle_t g_recognizer = NULL;
static QueueHandle_t g_audio_queue = NULL;
static TaskHandle_t g_speech_task_handle = NULL;

// Структура для аудио данных в очереди / Audio data structure for queue
typedef struct {
    int16_t* audio_data;
    size_t audio_size;
    uint32_t timestamp;
} audio_frame_t;

/**
 * @brief Callback для результатов распознавания
 * Speech recognition result callback
 */
static void speech_result_callback(const speech_result_t* result, void* user_data) {
    if (!result) {
        return;
    }
    
    ESP_LOGI(TAG, "Speech result: '%s' (confidence: %.2f, final: %s)", 
             result->text, result->confidence, result->is_final ? "yes" : "no");
    
    // Отправить результат в обработчик голосовых команд / Send result to voice command handler
    if (result->is_final) {
        voice_commands_process_result(result);
    }
}

/**
 * @brief Основная задача распознавания речи
 * Main speech recognition task
 */
static void speech_task_function(void* arg) {
    ESP_LOGI(TAG, "Speech recognition task started");
    
    audio_frame_t frame;
    
    while (1) {
        // Получить аудио кадр из очереди / Get audio frame from queue
        if (xQueueReceive(g_audio_queue, &frame, pdMS_TO_TICKS(100)) == pdTRUE) {
            // Обработать аудио кадр / Process audio frame
            esp_err_t ret = speech_recognizer_process_audio(g_recognizer, 
                                                           frame.audio_data, 
                                                           frame.audio_size);
            if (ret != ESP_OK) {
                ESP_LOGE(TAG, "Failed to process audio frame: %s", esp_err_to_name(ret));
            }
            
            // Освободить память / Free memory
            free(frame.audio_data);
        }
    }
}

void create_speech_task(void) {
    ESP_LOGI(TAG, "Creating speech recognition task");
    
    // Инициализация системы голосовых команд / Initialize voice command system
    voice_commands_init();
    
    // Конфигурация распознавателя речи / Speech recognizer configuration
    speech_config_t config = {
        .sensitivity = 0.5f,
        .max_recording_time = 5000,
        .language = "ru",
        .enable_noise_reduction = true,
        .enable_agc = true,
        .confidence_threshold = 0.7f
    };
    
    // Создание распознавателя речи / Create speech recognizer
    esp_err_t ret = speech_recognizer_init(&g_recognizer, &config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize speech recognizer: %s", esp_err_to_name(ret));
        return;
    }
    
    // Установка callback для результатов / Set result callback
    ret = speech_recognizer_set_callback(g_recognizer, speech_result_callback, NULL);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set speech callback: %s", esp_err_to_name(ret));
        speech_recognizer_deinit(g_recognizer);
        g_recognizer = NULL;
        return;
    }
    
    // Создание очереди аудио данных / Create audio queue
    g_audio_queue = xQueueCreate(SPEECH_AUDIO_QUEUE_SIZE, sizeof(audio_frame_t));
    if (!g_audio_queue) {
        ESP_LOGE(TAG, "Failed to create audio queue");
        speech_recognizer_deinit(g_recognizer);
        g_recognizer = NULL;
        return;
    }
    
    // Запуск распознавателя / Start recognizer
    ret = speech_recognizer_start(g_recognizer);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start speech recognizer: %s", esp_err_to_name(ret));
        vQueueDelete(g_audio_queue);
        g_audio_queue = NULL;
        speech_recognizer_deinit(g_recognizer);
        g_recognizer = NULL;
        return;
    }
    
    // Создание задачи / Create task
    BaseType_t task_ret = xTaskCreate(speech_task_function, "speech_task",
                                      SPEECH_TASK_STACK_SIZE, NULL,
                                      SPEECH_TASK_PRIORITY, &g_speech_task_handle);
    if (task_ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create speech task");
        speech_recognizer_stop(g_recognizer);
        vQueueDelete(g_audio_queue);
        g_audio_queue = NULL;
        speech_recognizer_deinit(g_recognizer);
        g_recognizer = NULL;
        return;
    }
    
    ESP_LOGI(TAG, "Speech recognition task created successfully");
}

esp_err_t speech_send_audio_frame(const int16_t* audio_data, size_t audio_size) {
    if (!g_audio_queue || !audio_data || audio_size == 0) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // Выделить память для аудио данных / Allocate memory for audio data
    int16_t* audio_copy = malloc(audio_size * sizeof(int16_t));
    if (!audio_copy) {
        ESP_LOGE(TAG, "Failed to allocate memory for audio frame");
        return ESP_ERR_NO_MEM;
    }
    
    // Копировать аудио данные / Copy audio data
    memcpy(audio_copy, audio_data, audio_size * sizeof(int16_t));
    
    // Создать кадр для очереди / Create frame for queue
    audio_frame_t frame = {
        .audio_data = audio_copy,
        .audio_size = audio_size,
        .timestamp = xTaskGetTickCount()
    };
    
    // Отправить в очередь / Send to queue
    if (xQueueSend(g_audio_queue, &frame, 0) != pdTRUE) {
        ESP_LOGW(TAG, "Audio queue full, dropping frame");
        free(audio_copy);
        return ESP_ERR_NO_MEM;
    }
    
    return ESP_OK;
}