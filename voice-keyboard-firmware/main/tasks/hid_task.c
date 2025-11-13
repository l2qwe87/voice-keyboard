/**
 * @file hid_task.c
 * @brief HID task implementation
 * @author Voice Keyboard Team
 * @date 2025
 * 
 * Реализация задачи HID
 * Implementation of HID task
 */

#include "hid_task.h"
#include "hid_config.h"
#include <stdlib.h>
#include <string.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

static const char* TAG = "HID_TASK";

// Внутренняя структура задачи HID / Internal HID task structure
struct hid_task {
    hid_device_handle_t hid_device;
    
    // Задача / Task
    TaskHandle_t task_handle;
    bool is_running;
    
    // Очередь команд / Command queue
    QueueHandle_t command_queue;
    
    // Статистика / Statistics
    hid_stats_t stats;
};

/**
 * @brief Выполнить клавиатурную команду
 * Execute keyboard command
 */
static void execute_keyboard_command(struct hid_task* hid_task, const voice_command_t* command) {
    ESP_LOGI(TAG, "⌨️  Executing keyboard command: %s", command->command);
    
    if (strcmp(command->command, "space") == 0) {
        hid_keyboard_click_key(hid_task->hid_device, HID_KEY_SPACE, 0);
    } else if (strcmp(command->command, "enter") == 0) {
        hid_keyboard_click_key(hid_task->hid_device, HID_KEY_ENTER, 0);
    } else if (strcmp(command->command, "tab") == 0) {
        hid_keyboard_click_key(hid_task->hid_device, HID_KEY_TAB, 0);
    } else if (strcmp(command->command, "escape") == 0) {
        hid_keyboard_click_key(hid_task->hid_device, HID_KEY_ESCAPE, 0);
    } else if (strcmp(command->command, "backspace") == 0) {
        hid_keyboard_click_key(hid_task->hid_device, HID_KEY_BACKSPACE, 0);
    } else if (strcmp(command->command, "hello") == 0) {
        // Напечатать "Hello" / Type "Hello"
        const char* hello_text = "Hello";
        for (int i = 0; hello_text[i]; i++) {
            char c = tolower(hello_text[i]);
            hid_keyboard_key_t key;
            
            if (c >= 'a' && c <= 'z') {
                key = (hid_keyboard_key_t)(HID_KEY_A + (c - 'a'));
                hid_keyboard_click_key(hid_task->hid_device, key, 0);
                vTaskDelay(pdMS_TO_TICKS(50)); // Задержка между символами / Delay between characters
            } else if (c >= '0' && c <= '9') {
                key = (hid_keyboard_key_t)(HID_KEY_0 + (c - '0'));
                hid_keyboard_click_key(hid_task->hid_device, key, 0);
                vTaskDelay(pdMS_TO_TICKS(50));
            }
        }
    }
    
    hid_task->stats.keyboard_commands++;
}

/**
 * @brief Выполнить команду мыши
 * Execute mouse command
 */
static void execute_mouse_command(struct hid_task* hid_task, const voice_command_t* command) {
    ESP_LOGI(TAG, "🖱️  Executing mouse command: %s", command->command);
    
    if (strcmp(command->command, "left") == 0) {
        hid_mouse_click(hid_task->hid_device, HID_MOUSE_BUTTON_LEFT);
    } else if (strcmp(command->command, "right") == 0) {
        hid_mouse_click(hid_task->hid_device, HID_MOUSE_BUTTON_RIGHT);
    } else if (strcmp(command->command, "double_left") == 0) {
        hid_mouse_click(hid_task->hid_device, HID_MOUSE_BUTTON_LEFT);
        vTaskDelay(pdMS_TO_TICKS(100));
        hid_mouse_click(hid_task->hid_device, HID_MOUSE_BUTTON_LEFT);
    } else if (strcmp(command->command, "move_up") == 0) {
        hid_mouse_move(hid_task->hid_device, 0, -10);
    } else if (strcmp(command->command, "move_down") == 0) {
        hid_mouse_move(hid_task->hid_device, 0, 10);
    } else if (strcmp(command->command, "move_left") == 0) {
        hid_mouse_move(hid_task->hid_device, -10, 0);
    } else if (strcmp(command->command, "move_right") == 0) {
        hid_mouse_move(hid_task->hid_device, 10, 0);
    }
    
    hid_task->stats.mouse_commands++;
}

/**
 * @brief Выполнить медиа команду
 * Execute media command
 */
static void execute_media_command(struct hid_task* hid_task, const voice_command_t* command) {
    ESP_LOGI(TAG, "🎵 Executing media command: %s", command->command);
    
    if (strcmp(command->command, "play") == 0 || strcmp(command->command, "pause") == 0) {
        // Используем клавишу пробела для play/pause / Use space key for play/pause
        hid_keyboard_click_key(hid_task->hid_device, HID_KEY_SPACE, HID_MODIFIER_LEFT_CTRL);
    } else if (strcmp(command->command, "next") == 0) {
        // Используем Ctrl+Right для следующего трека / Use Ctrl+Right for next track
        hid_keyboard_press_key(hid_task->hid_device, HID_KEY_RIGHT_ARROW, HID_MODIFIER_LEFT_CTRL);
        vTaskDelay(pdMS_TO_TICKS(50));
        hid_keyboard_release_key(hid_task->hid_device);
    } else if (strcmp(command->command, "previous") == 0) {
        // Используем Ctrl+Left для предыдущего трека / Use Ctrl+Left for previous track
        hid_keyboard_press_key(hid_task->hid_device, HID_KEY_LEFT_ARROW, HID_MODIFIER_LEFT_CTRL);
        vTaskDelay(pdMS_TO_TICKS(50));
        hid_keyboard_release_key(hid_task->hid_device);
    }
    
    hid_task->stats.media_commands++;
}

/**
 * @brief Выполнить системную команду
 * Execute system command
 */
static void execute_system_command(struct hid_task* hid_task, const voice_command_t* command) {
    ESP_LOGI(TAG, "⚙️  Executing system command: %s", command->command);
    
    if (strcmp(command->command, "sleep") == 0) {
        // Используем Alt+F4 для закрытия окна / Use Alt+F4 to close window
        hid_keyboard_press_key(hid_task->hid_device, HID_KEY_F4, HID_MODIFIER_LEFT_ALT);
        vTaskDelay(pdMS_TO_TICKS(50));
        hid_keyboard_release_key(hid_task->hid_device);
    } else if (strcmp(command->command, "lock") == 0) {
        // Используем Win+L для блокировки / Use Win+L for lock
        hid_keyboard_press_key(hid_task->hid_device, HID_KEY_L, HID_MODIFIER_LEFT_GUI);
        vTaskDelay(pdMS_TO_TICKS(50));
        hid_keyboard_release_key(hid_task->hid_device);
    }
    
    hid_task->stats.system_commands++;
}

/**
 * @brief Выполнить команду громкости
 * Execute volume command
 */
static void execute_volume_command(struct hid_task* hid_task, const voice_command_t* command) {
    ESP_LOGI(TAG, "🔊 Executing volume command: %s", command->command);
    
    if (strcmp(command->command, "up") == 0) {
        // Используем Volume Up (мультимедийная клавиша) / Use Volume Up (multimedia key)
        hid_keyboard_click_key(hid_task->hid_device, HID_KEY_F12, 0);
    } else if (strcmp(command->command, "down") == 0) {
        // Используем Volume Down (мультимедийная клавиша) / Use Volume Down (multimedia key)
        hid_keyboard_click_key(hid_task->hid_device, HID_KEY_F11, 0);
    } else if (strcmp(command->command, "mute") == 0) {
        // Используем Volume Mute (мультимедийная клавиша) / Use Volume Mute (multimedia key)
        hid_keyboard_click_key(hid_task->hid_device, HID_KEY_F10, 0);
    }
    
    hid_task->stats.media_commands++;
}

/**
 * @brief Основная задача HID
 * Main HID task
 */
static void hid_task_function(void* arg) {
    struct hid_task* hid_task = (struct hid_task*)arg;
    voice_command_t command;
    
    ESP_LOGI(TAG, "HID task started");
    
    while (hid_task->is_running) {
        // Ожидание команды / Wait for command
        if (xQueueReceive(hid_task->command_queue, &command, pdMS_TO_TICKS(100)) == pdTRUE) {
            ESP_LOGI(TAG, "Processing voice command: '%s' -> %s (type: %d, action: %d)", 
                     command.text, command.command, command.type, command.action);
            
            hid_task->stats.commands_processed++;
            
            // Выполнение команды в зависимости от типа / Execute command based on type
            switch (command.type) {
                case CMD_TYPE_KEYBOARD:
                    execute_keyboard_command(hid_task, &command);
                    break;
                    
                case CMD_TYPE_MOUSE:
                    execute_mouse_command(hid_task, &command);
                    break;
                    
                case CMD_TYPE_MEDIA:
                    execute_media_command(hid_task, &command);
                    break;
                    
                case CMD_TYPE_SYSTEM:
                    execute_system_command(hid_task, &command);
                    break;
                    
                case CMD_TYPE_VOLUME:
                    execute_volume_command(hid_task, &command);
                    break;
                    
                case CMD_TYPE_GREETING:
                case CMD_TYPE_GOODBYE:
                    // Просто логируем приветствия / Just log greetings
                    ESP_LOGI(TAG, "👋 %s command: '%s'", 
                             command.type == CMD_TYPE_GREETING ? "Greeting" : "Goodbye", command.text);
                    break;
                    
                case CMD_TYPE_UNKNOWN:
                default:
                    ESP_LOGW(TAG, "❓ Unknown command type: %d", command.type);
                    hid_task->stats.unknown_commands++;
                    break;
            }
            
            // Небольшая задержка между командами / Small delay between commands
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        
        // Проверка статуса HID устройства / Check HID device status
        if (!hid_is_connected(hid_task->hid_device)) {
            static int disconnect_count = 0;
            if (++disconnect_count >= 50) { // Каждые 5 секунд / Every 5 seconds
                ESP_LOGW(TAG, "HID device not connected");
                disconnect_count = 0;
            }
        } else {
            // Устройство подключено / Device connected
        }
    }
    
    ESP_LOGI(TAG, "HID task stopped");
    vTaskDelete(NULL);
}

esp_err_t hid_task_init(hid_task_handle_t* handle) {
    if (!handle) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // Выделение памяти / Allocate memory
    *handle = malloc(sizeof(struct hid_task));
    if (!*handle) {
        ESP_LOGE(TAG, "Failed to allocate memory for HID task");
        return ESP_ERR_NO_MEM;
    }
    
    // Инициализация структуры / Initialize structure
    memset(*handle, 0, sizeof(struct hid_task));
    
    // Создание очереди команд / Create command queue
    (*handle)->command_queue = xQueueCreate(HID_COMMAND_QUEUE_SIZE, sizeof(voice_command_t));
    if (!(*handle)->command_queue) {
        ESP_LOGE(TAG, "Failed to create command queue");
        free(*handle);
        return ESP_ERR_NO_MEM;
    }
    
    // Инициализация HID устройства / Initialize HID device
    esp_err_t ret = hid_init(&(*handle)->hid_device);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize HID device: %s", esp_err_to_name(ret));
        vQueueDelete((*handle)->command_queue);
        free(*handle);
        return ret;
    }
    
    ESP_LOGI(TAG, "HID task initialized successfully");
    return ESP_OK;
}

esp_err_t hid_task_deinit(hid_task_handle_t handle) {
    if (!handle) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // Остановка задачи / Stop task
    hid_task_stop(handle);
    
    // Деинициализация HID устройства / Deinitialize HID device
    if (handle->hid_device) {
        hid_deinit(handle->hid_device);
    }
    
    // Очистка очереди / Cleanup queue
    if (handle->command_queue) {
        vQueueDelete(handle->command_queue);
    }
    
    free(handle);
    ESP_LOGI(TAG, "HID task deinitialized");
    
    return ESP_OK;
}

esp_err_t hid_task_start(hid_task_handle_t handle) {
    if (!handle) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (handle->is_running) {
        ESP_LOGW(TAG, "HID task already running");
        return ESP_ERR_INVALID_STATE;
    }
    
    // Создание задачи / Create task
    handle->is_running = true;
    BaseType_t task_ret = xTaskCreate(hid_task_function, "hid_task",
                                      HID_TASK_STACK_SIZE, handle,
                                      HID_TASK_PRIORITY, &handle->task_handle);
    
    if (task_ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create HID task");
        handle->is_running = false;
        return ESP_ERR_NO_MEM;
    }
    
    ESP_LOGI(TAG, "HID task started successfully");
    return ESP_OK;
}

esp_err_t hid_task_stop(hid_task_handle_t handle) {
    if (!handle) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (!handle->is_running) {
        ESP_LOGW(TAG, "HID task already stopped");
        return ESP_ERR_INVALID_STATE;
    }
    
    // Остановка задачи / Stop task
    handle->is_running = false;
    if (handle->task_handle) {
        vTaskDelete(handle->task_handle);
        handle->task_handle = NULL;
    }
    
    ESP_LOGI(TAG, "HID task stopped");
    return ESP_OK;
}

esp_err_t hid_task_send_command(hid_task_handle_t handle, const voice_command_t* command) {
    if (!handle || !command) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (!handle->is_running) {
        return ESP_ERR_INVALID_STATE;
    }
    
    // Отправка команды в очередь / Send command to queue
    if (xQueueSend(handle->command_queue, command, pdMS_TO_TICKS(10)) != pdTRUE) {
        ESP_LOGW(TAG, "Command queue full, dropping command");
        return ESP_ERR_TIMEOUT;
    }
    
    return ESP_OK;
}

bool hid_task_is_connected(hid_task_handle_t handle) {
    if (!handle) {
        return false;
    }
    
    return hid_is_connected(handle->hid_device);
}

esp_err_t hid_task_get_stats(hid_task_handle_t handle, hid_stats_t* stats) {
    if (!handle || !stats) {
        return ESP_ERR_INVALID_ARG;
    }
    
    *stats = handle->stats;
    return ESP_OK;
}

esp_err_t hid_task_reset_stats(hid_task_handle_t handle) {
    if (!handle) {
        return ESP_ERR_INVALID_ARG;
    }
    
    memset(&handle->stats, 0, sizeof(hid_stats_t));
    return ESP_OK;
}