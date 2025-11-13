/**
 * @file speech_task.h
 * @brief Speech recognition task header
 * @author Voice Keyboard Team
 * @date 2025
 * 
 * Заголовочный файл задачи распознавания речи
 * Header file for speech recognition task
 */

#ifndef SPEECH_TASK_H
#define SPEECH_TASK_H

#include "../config/speech_recognition.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"

// Приоритет задачи / Task priority
#define SPEECH_TASK_PRIORITY    5

// Размер стека задачи / Task stack size
#define SPEECH_TASK_STACK_SIZE  4096

// Размер очереди аудио данных / Audio data queue size
#define SPEECH_AUDIO_QUEUE_SIZE 10

/**
 * @brief Создать задачу распознавания речи
 * Create speech recognition task
 */
void create_speech_task(void);

/**
 * @brief Отправить аудио кадр в задачу распознавания речи
 * Send audio frame to speech recognition task
 */
esp_err_t speech_send_audio_frame(const int16_t* audio_data, size_t audio_size);

#endif // SPEECH_TASK_H