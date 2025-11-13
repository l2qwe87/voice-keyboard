/**
 * @file voice_commands.h
 * @brief Voice command processing header
 * @author Voice Keyboard Team
 * @date 2025
 * 
 * Заголовочный файл обработки голосовых команд
 * Header file for voice command processing
 */

#ifndef VOICE_COMMANDS_H
#define VOICE_COMMANDS_H

#include "speech_recognition.h"
#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

// Типы команд / Command types
typedef enum {
    CMD_TYPE_UNKNOWN,        // Неизвестная команда / Unknown command
    CMD_TYPE_GREETING,       // Приветствие / Greeting
    CMD_TYPE_GOODBYE,        // Прощание / Goodbye
    CMD_TYPE_KEYBOARD,       // Команды клавиатуры / Keyboard commands
    CMD_TYPE_MOUSE,          // Команды мыши / Mouse commands
    CMD_TYPE_SYSTEM,         // Системные команды / System commands
    CMD_TYPE_VOLUME,         // Команды громкости / Volume commands
    CMD_TYPE_MEDIA           // Медиа команды / Media commands
} command_type_t;

// Действия команд / Command actions
typedef enum {
    CMD_ACTION_NONE,          // Нет действия / No action
    CMD_ACTION_KEY_PRESS,     // Нажатие клавиши / Key press
    CMD_ACTION_KEY_HOLD,      // Удержание клавиши / Key hold
    CMD_ACTION_KEY_RELEASE,   // Отпускание клавиши / Key release
    CMD_ACTION_MOUSE_CLICK,   // Клик мыши / Mouse click
    CMD_ACTION_MOUSE_MOVE,    // Движение мыши / Mouse move
    CMD_ACTION_VOLUME_UP,     // Громкость вверх / Volume up
    CMD_ACTION_VOLUME_DOWN,   // Громкость вниз / Volume down
    CMD_ACTION_VOLUME_MUTE,   // Выключить звук / Mute
    CMD_ACTION_PLAY_PAUSE,    // Воспроизведение/пауза / Play/pause
    CMD_ACTION_NEXT_TRACK,    // Следующий трек / Next track
    CMD_ACTION_PREV_TRACK,    // Предыдущий трек / Previous track
    CMD_ACTION_SYSTEM_SLEEP,  // Сон / Sleep
    CMD_ACTION_SYSTEM_LOCK,   // Блокировка / Lock
    CMD_ACTION_SYSTEM_WAKE    // Пробуждение / Wake
} command_action_t;

// Голосовая команда / Voice command
typedef struct {
    command_type_t type;      // Тип команды / Command type
    command_action_t action;  // Действие / Action
    char text[64];            // Распознанный текст / Recognized text
    char command[32];         // Команда / Command
    char param[32];           // Параметр / Parameter
    float confidence;         // Уверенность / Confidence
} voice_command_t;

// Дескриптор процессора команд / Command processor handle
typedef struct voice_command_processor* voice_command_processor_handle_t;

/**
 * @brief Callback для выполнения команд
 * Command execution callback
 */
typedef void (*command_execution_callback_t)(const voice_command_t* command, void* user_data);

/**
 * @brief Инициализация процессора голосовых команд
 * Initialize voice command processor
 */
esp_err_t voice_command_processor_init(voice_command_processor_handle_t* handle);

/**
 * @brief Деинициализация процессора голосовых команд
 * Deinitialize voice command processor
 */
esp_err_t voice_command_processor_deinit(voice_command_processor_handle_t handle);

/**
 * @brief Обработать результат распознавания речи
 * Process speech recognition result
 */
esp_err_t voice_command_processor_process_result(voice_command_processor_handle_t handle, 
                                                 const speech_result_t* speech_result);

/**
 * @brief Установить callback для выполнения команд
 * Set command execution callback
 */
esp_err_t voice_command_processor_set_callback(voice_command_processor_handle_t handle,
                                               command_execution_callback_t callback, 
                                               void* user_data);

/**
 * @brief Получить статистику обработки команд
 * Get command processing statistics
 */
typedef struct {
    uint32_t total_commands;      // Всего команд обработано / Total commands processed
    uint32_t recognized_commands; // Распознано команд / Commands recognized
    uint32_t unknown_commands;    // Неизвестных команд / Unknown commands
    float average_confidence;     // Средняя уверенность / Average confidence
} command_stats_t;

esp_err_t voice_command_processor_get_stats(voice_command_processor_handle_t handle, 
                                            command_stats_t* stats);

/**
 * @brief Сбросить статистику
 * Reset statistics
 */
esp_err_t voice_command_processor_reset_stats(voice_command_processor_handle_t handle);

#endif // VOICE_COMMANDS_H