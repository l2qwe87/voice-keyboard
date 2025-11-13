/**
 * @file speech_recognition.h
 * @brief Speech recognition framework header
 * @author Voice Keyboard Team
 * @date 2025
 * 
 * Заголовочный файл фреймворка распознавания речи
 * Header file for speech recognition framework
 */

#ifndef SPEECH_RECOGNITION_H
#define SPEECH_RECOGNITION_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "esp_log.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

// Конфигурация распознавания речи / Speech recognition configuration
#define SPEECH_SAMPLE_RATE        16000    // Частота дискретизации / Sample rate
#define SPEECH_CHANNELS           1        // Количество каналов / Number of channels
#define SPEECH_BITS_PER_SAMPLE    16       // Бит на выборку / Bits per sample
#define SPEECH_BUFFER_SIZE        1024     // Размер буфера аудио / Audio buffer size
#define SPEECH_VAD_THRESHOLD      0.01f    // Порог VAD / VAD threshold
#define SPEECH_MIN_VOICE_FRAMES   10       // Минимальное количество речевых кадров / Min voice frames
#define SPEECH_SILENCE_FRAMES     20       // Порог тишины для завершения / Silence frames threshold

// Состояния распознавания / Recognition states
typedef enum {
    SPEECH_STATE_IDLE,        // Ожидание / Idle
    SPEECH_STATE_LISTENING,   // Прослушивание / Listening
    SPEECH_STATE_PROCESSING, // Обработка / Processing
    SPEECH_STATE_ERROR        // Ошибка / Error
} speech_state_t;

// Результат распознавания / Recognition result
typedef struct {
    char text[256];           // Распознанный текст / Recognized text
    float confidence;         // Уверенность / Confidence
    bool is_final;           // Финальный результат / Final result
} speech_result_t;

// Конфигурация распознавания / Speech configuration
typedef struct {
    float sensitivity;        // Чувствительность / Sensitivity
    int max_recording_time;   // Максимальное время записи / Max recording time
    char language[4];         // Язык / Language
    bool enable_noise_reduction; // Шумоподавление / Noise reduction
    bool enable_agc;          // AGC / AGC
    float confidence_threshold; // Порог уверенности / Confidence threshold
} speech_config_t;

// Дескриптор распознавания / Speech recognizer handle
typedef struct speech_recognizer* speech_recognizer_handle_t;

/**
 * @brief Инициализация распознавателя речи
 * Initialize speech recognizer
 */
esp_err_t speech_recognizer_init(speech_recognizer_handle_t* handle, const speech_config_t* config);

/**
 * @brief Деинициализация распознавателя речи
 * Deinitialize speech recognizer
 */
esp_err_t speech_recognizer_deinit(speech_recognizer_handle_t handle);

/**
 * @brief Начать распознавание
 * Start recognition
 */
esp_err_t speech_recognizer_start(speech_recognizer_handle_t handle);

/**
 * @brief Остановить распознавание
 * Stop recognition
 */
esp_err_t speech_recognizer_stop(speech_recognizer_handle_t handle);

/**
 * @brief Обработать аудио данные
 * Process audio data
 */
esp_err_t speech_recognizer_process_audio(speech_recognizer_handle_t handle, 
                                         const int16_t* audio_data, size_t audio_size);

/**
 * @brief Получить результат распознавания
 * Get recognition result
 */
esp_err_t speech_recognizer_get_result(speech_recognizer_handle_t handle, speech_result_t* result);

/**
 * @brief Получить текущее состояние
 * Get current state
 */
speech_state_t speech_recognizer_get_state(speech_recognizer_handle_t handle);

/**
 * @brief Установить callback для результатов
 * Set result callback
 */
typedef void (*speech_result_callback_t)(const speech_result_t* result, void* user_data);
esp_err_t speech_recognizer_set_callback(speech_recognizer_handle_t handle, 
                                        speech_result_callback_t callback, void* user_data);

#endif // SPEECH_RECOGNITION_H