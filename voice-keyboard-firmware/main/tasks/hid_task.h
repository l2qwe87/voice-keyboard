/**
 * @file hid_task.h
 * @brief HID task header
 * @author Voice Keyboard Team
 * @date 2025
 * 
 * Заголовочный файл задачи HID
 * Header file for HID task
 */

#ifndef HID_TASK_H
#define HID_TASK_H

#include "hid_config.h"
#include "voice_commands.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"

// Приоритет задачи / Task priority
#define HID_TASK_PRIORITY    4

// Размер стека задачи / Task stack size
#define HID_TASK_STACK_SIZE  4096

// Размер очереди команд / Command queue size
#define HID_COMMAND_QUEUE_SIZE 20

// Дескриптор задачи HID / HID task handle
typedef struct hid_task* hid_task_handle_t;

/**
 * @brief Инициализация задачи HID
 * Initialize HID task
 */
esp_err_t hid_task_init(hid_task_handle_t* handle);

/**
 * @brief Деинициализация задачи HID
 * Deinitialize HID task
 */
esp_err_t hid_task_deinit(hid_task_handle_t handle);

/**
 * @brief Запустить задачу HID
 * Start HID task
 */
esp_err_t hid_task_start(hid_task_handle_t handle);

/**
 * @brief Остановить задачу HID
 * Stop HID task
 */
esp_err_t hid_task_stop(hid_task_handle_t handle);

/**
 * @brief Отправить голосовую команду в HID задачу
 * Send voice command to HID task
 */
esp_err_t hid_task_send_command(hid_task_handle_t handle, const voice_command_t* command);

/**
 * @brief Получить статус HID
 * Get HID status
 */
bool hid_task_is_connected(hid_task_handle_t handle);

/**
 * @brief Получить статистику HID
 * Get HID statistics
 */
typedef struct {
    uint32_t commands_processed;    // Команд обработано / Commands processed
    uint32_t keyboard_commands;     // Клавиатурных команд / Keyboard commands
    uint32_t mouse_commands;        // Команд мыши / Mouse commands
    uint32_t media_commands;        // Медиа команд / Media commands
    uint32_t system_commands;       // Системных команд / System commands
    uint32_t unknown_commands;      // Неизвестных команд / Unknown commands
} hid_stats_t;

esp_err_t hid_task_get_stats(hid_task_handle_t handle, hid_stats_t* stats);

/**
 * @brief Сбросить статистику
 * Reset statistics
 */
esp_err_t hid_task_reset_stats(hid_task_handle_t handle);

#endif // HID_TASK_H