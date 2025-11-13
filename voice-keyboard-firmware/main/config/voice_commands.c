/**
 * @file voice_commands.c
 * @brief Voice command processing implementation
 * @author Voice Keyboard Team
 * @date 2025
 * 
 * Реализация обработки голосовых команд
 * Implementation of voice command processing
 */

#include "voice_commands.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "esp_log.h"

static const char* TAG = "VOICE_COMMANDS";

// Внутренняя структура процессора команд / Internal command processor structure
struct voice_command_processor {
    command_execution_callback_t execution_callback;
    void* user_data;
    
    // Статистика / Statistics
    command_stats_t stats;
    float confidence_sum;
    uint32_t confidence_count;
};

// Шаблоны команд / Command patterns
typedef struct {
    const char* pattern;         // Шаблон / Pattern
    command_type_t type;         // Тип / Type
    command_action_t action;     // Действие / Action
    const char* command;         // Команда / Command
} command_pattern_t;

// Словарь команд / Command dictionary
static const command_pattern_t command_patterns[] = {
    // Приветствия / Greetings
    {"привет", CMD_TYPE_GREETING, CMD_ACTION_NONE, "hello"},
    {"здравствуй", CMD_TYPE_GREETING, CMD_ACTION_NONE, "hello"},
    {"hello", CMD_TYPE_GREETING, CMD_ACTION_NONE, "hello"},
    {"hi", CMD_TYPE_GREETING, CMD_ACTION_NONE, "hello"},
    
    // Прощания / Goodbyes
    {"пока", CMD_TYPE_GOODBYE, CMD_ACTION_NONE, "goodbye"},
    {"до свидания", CMD_TYPE_GOODBYE, CMD_ACTION_NONE, "goodbye"},
    {"goodbye", CMD_TYPE_GOODBYE, CMD_ACTION_NONE, "goodbye"},
    {"bye", CMD_TYPE_GOODBYE, CMD_ACTION_NONE, "goodbye"},
    
    // Команды клавиатуры / Keyboard commands
    {"нажми пробел", CMD_TYPE_KEYBOARD, CMD_ACTION_KEY_PRESS, "space"},
    {"нажми ввод", CMD_TYPE_KEYBOARD, CMD_ACTION_KEY_PRESS, "enter"},
    {"нажми таб", CMD_TYPE_KEYBOARD, CMD_ACTION_KEY_PRESS, "tab"},
    {"нажми эскейп", CMD_TYPE_KEYBOARD, CMD_ACTION_KEY_PRESS, "escape"},
    {"нажми бэкспейс", CMD_TYPE_KEYBOARD, CMD_ACTION_KEY_PRESS, "backspace"},
    {"press space", CMD_TYPE_KEYBOARD, CMD_ACTION_KEY_PRESS, "space"},
    {"press enter", CMD_TYPE_KEYBOARD, CMD_ACTION_KEY_PRESS, "enter"},
    {"press tab", CMD_TYPE_KEYBOARD, CMD_ACTION_KEY_PRESS, "tab"},
    {"press escape", CMD_TYPE_KEYBOARD, CMD_ACTION_KEY_PRESS, "escape"},
    {"press backspace", CMD_TYPE_KEYBOARD, CMD_ACTION_KEY_PRESS, "backspace"},
    
    // Команды мыши / Mouse commands
    {"кликни", CMD_TYPE_MOUSE, CMD_ACTION_MOUSE_CLICK, "left"},
    {"кликни правой", CMD_TYPE_MOUSE, CMD_ACTION_MOUSE_CLICK, "right"},
    {"кликни левой", CMD_TYPE_MOUSE, CMD_ACTION_MOUSE_CLICK, "left"},
    {"двойной клик", CMD_TYPE_MOUSE, CMD_ACTION_MOUSE_CLICK, "double_left"},
    {"click", CMD_TYPE_MOUSE, CMD_ACTION_MOUSE_CLICK, "left"},
    {"right click", CMD_TYPE_MOUSE, CMD_ACTION_MOUSE_CLICK, "right"},
    {"left click", CMD_TYPE_MOUSE, CMD_ACTION_MOUSE_CLICK, "left"},
    {"double click", CMD_TYPE_MOUSE, CMD_ACTION_MOUSE_CLICK, "double_left"},
    {"двигай вверх", CMD_TYPE_MOUSE, CMD_ACTION_MOUSE_MOVE, "move_up"},
    {"двигай вниз", CMD_TYPE_MOUSE, CMD_ACTION_MOUSE_MOVE, "move_down"},
    {"двигай влево", CMD_TYPE_MOUSE, CMD_ACTION_MOUSE_MOVE, "move_left"},
    {"двигай вправо", CMD_TYPE_MOUSE, CMD_ACTION_MOUSE_MOVE, "move_right"},
    {"move up", CMD_TYPE_MOUSE, CMD_ACTION_MOUSE_MOVE, "move_up"},
    {"move down", CMD_TYPE_MOUSE, CMD_ACTION_MOUSE_MOVE, "move_down"},
    {"move left", CMD_TYPE_MOUSE, CMD_ACTION_MOUSE_MOVE, "move_left"},
    {"move right", CMD_TYPE_MOUSE, CMD_ACTION_MOUSE_MOVE, "move_right"},
    
    // Команды громкости / Volume commands
    {"громче", CMD_TYPE_VOLUME, CMD_ACTION_VOLUME_UP, "up"},
    {"тише", CMD_TYPE_VOLUME, CMD_ACTION_VOLUME_DOWN, "down"},
    {"выклюши звук", CMD_TYPE_VOLUME, CMD_ACTION_VOLUME_MUTE, "mute"},
    {"увеличь громкость", CMD_TYPE_VOLUME, CMD_ACTION_VOLUME_UP, "up"},
    {"уменьши громкость", CMD_TYPE_VOLUME, CMD_ACTION_VOLUME_DOWN, "down"},
    {"volume up", CMD_TYPE_VOLUME, CMD_ACTION_VOLUME_UP, "up"},
    {"volume down", CMD_TYPE_VOLUME, CMD_ACTION_VOLUME_DOWN, "down"},
    {"mute", CMD_TYPE_VOLUME, CMD_ACTION_VOLUME_MUTE, "mute"},
    {"louder", CMD_TYPE_VOLUME, CMD_ACTION_VOLUME_UP, "up"},
    {"quieter", CMD_TYPE_VOLUME, CMD_ACTION_VOLUME_DOWN, "down"},
    
    // Медиа команды / Media commands
    {"играй", CMD_TYPE_MEDIA, CMD_ACTION_PLAY_PAUSE, "play"},
    {"пауза", CMD_TYPE_MEDIA, CMD_ACTION_PLAY_PAUSE, "pause"},
    {"следующий трек", CMD_TYPE_MEDIA, CMD_ACTION_NEXT_TRACK, "next"},
    {"предыдущий трек", CMD_TYPE_MEDIA, CMD_ACTION_PREV_TRACK, "previous"},
    {"play", CMD_TYPE_MEDIA, CMD_ACTION_PLAY_PAUSE, "play"},
    {"pause", CMD_TYPE_MEDIA, CMD_ACTION_PLAY_PAUSE, "pause"},
    {"next track", CMD_TYPE_MEDIA, CMD_ACTION_NEXT_TRACK, "next"},
    {"previous track", CMD_TYPE_MEDIA, CMD_ACTION_PREV_TRACK, "previous"},
    
    // Системные команды / System commands
    {"сон", CMD_TYPE_SYSTEM, CMD_ACTION_SYSTEM_SLEEP, "sleep"},
    {"блокировка", CMD_TYPE_SYSTEM, CMD_ACTION_SYSTEM_LOCK, "lock"},
    {"спящий режим", CMD_TYPE_SYSTEM, CMD_ACTION_SYSTEM_SLEEP, "sleep"},
    {"sleep", CMD_TYPE_SYSTEM, CMD_ACTION_SYSTEM_SLEEP, "sleep"},
    {"lock", CMD_TYPE_SYSTEM, CMD_ACTION_SYSTEM_LOCK, "lock"},
    {"hibernate", CMD_TYPE_SYSTEM, CMD_ACTION_SYSTEM_SLEEP, "sleep"},
};

static const int num_patterns = sizeof(command_patterns) / sizeof(command_pattern_t);

/**
 * @brief Преобразовать строку в нижний регистр
 * Convert string to lowercase
 */
static void to_lowercase(char* str) {
    for (int i = 0; str[i]; i++) {
        str[i] = tolower(str[i]);
    }
}

/**
 * @brief Проверить совпадение с шаблоном
 * Check pattern match
 */
static bool match_pattern(const char* text, const char* pattern) {
    char text_lower[256];
    char pattern_lower[256];
    
    strncpy(text_lower, text, sizeof(text_lower) - 1);
    strncpy(pattern_lower, pattern, sizeof(pattern_lower) - 1);
    
    text_lower[sizeof(text_lower) - 1] = '\0';
    pattern_lower[sizeof(pattern_lower) - 1] = '\0';
    
    to_lowercase(text_lower);
    to_lowercase(pattern_lower);
    
    return strstr(text_lower, pattern_lower) != NULL;
}

/**
 * @brief Распарсить команду
 * Parse command
 */
static bool parse_command(const char* text, voice_command_t* command) {
    for (int i = 0; i < num_patterns; i++) {
        if (match_pattern(text, command_patterns[i].pattern)) {
            command->type = command_patterns[i].type;
            command->action = command_patterns[i].action;
            strncpy(command->command, command_patterns[i].command, sizeof(command->command) - 1);
            command->command[sizeof(command->command) - 1] = '\0';
            
            // Извлечение параметров / Extract parameters
            // TODO: Добавить извлечение числовых параметров / TODO: Add numeric parameter extraction
            
            return true;
        }
    }
    
    return false;
}

/**
 * @brief Выполнить команду
 * Execute command
 */
static void execute_command(const voice_command_t* command, void* user_data) {
    ESP_LOGI(TAG, "🎯 Executing command: type=%d, action=%d, command='%s'", 
             command->type, command->action, command->command);
    
    switch (command->action) {
        case CMD_ACTION_KEY_PRESS:
            ESP_LOGI(TAG, "⌨️  Pressing key: %s", command->command);
            // TODO: Отправить HID команду клавиатуры / TODO: Send keyboard HID command
            break;
            
        case CMD_ACTION_MOUSE_CLICK:
            ESP_LOGI(TAG, "🖱️  Mouse click: %s", command->command);
            // TODO: Отправить HID команду мыши / TODO: Send mouse HID command
            break;
            
        case CMD_ACTION_MOUSE_MOVE:
            ESP_LOGI(TAG, "🖱️  Mouse move: %s", command->command);
            // TODO: Отправить HID команду движения мыши / TODO: Send mouse move HID command
            break;
            
        case CMD_ACTION_VOLUME_UP:
            ESP_LOGI(TAG, "🔊 Volume up");
            // TODO: Отправить команду громкости / TODO: Send volume command
            break;
            
        case CMD_ACTION_VOLUME_DOWN:
            ESP_LOGI(TAG, "🔉 Volume down");
            // TODO: Отправить команду громкости / TODO: Send volume command
            break;
            
        case CMD_ACTION_VOLUME_MUTE:
            ESP_LOGI(TAG, "🔇 Volume mute");
            // TODO: Отправить команду громкости / TODO: Send volume command
            break;
            
        case CMD_ACTION_PLAY_PAUSE:
            ESP_LOGI(TAG, "⏯️  Play/Pause");
            // TODO: Отправить медиа команду / TODO: Send media command
            break;
            
        case CMD_ACTION_NEXT_TRACK:
            ESP_LOGI(TAG, "⏭️  Next track");
            // TODO: Отправить медиа команду / TODO: Send media command
            break;
            
        case CMD_ACTION_PREV_TRACK:
            ESP_LOGI(TAG, "⏮️  Previous track");
            // TODO: Отправить медиа команду / TODO: Send media command
            break;
            
        case CMD_ACTION_SYSTEM_SLEEP:
            ESP_LOGI(TAG, "😴 System sleep");
            // TODO: Отправить системную команду / TODO: Send system command
            break;
            
        case CMD_ACTION_SYSTEM_LOCK:
            ESP_LOGI(TAG, "🔒 System lock");
            // TODO: Отправить системную команду / TODO: Send system command
            break;
            
        default:
            ESP_LOGW(TAG, "Unknown command action: %d", command->action);
            break;
    }
}

esp_err_t voice_command_processor_init(voice_command_processor_handle_t* handle) {
    if (!handle) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // Выделение памяти / Allocate memory
    *handle = malloc(sizeof(struct voice_command_processor));
    if (!*handle) {
        ESP_LOGE(TAG, "Failed to allocate memory for voice command processor");
        return ESP_ERR_NO_MEM;
    }
    
    // Инициализация структуры / Initialize structure
    memset(*handle, 0, sizeof(struct voice_command_processor));
    
    ESP_LOGI(TAG, "Voice command processor initialized with %d command patterns", num_patterns);
    return ESP_OK;
}

esp_err_t voice_command_processor_deinit(voice_command_processor_handle_t handle) {
    if (!handle) {
        return ESP_ERR_INVALID_ARG;
    }
    
    free(handle);
    ESP_LOGI(TAG, "Voice command processor deinitialized");
    
    return ESP_OK;
}

esp_err_t voice_command_processor_process_result(voice_command_processor_handle_t handle, 
                                                 const speech_result_t* speech_result) {
    if (!handle || !speech_result) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // Обновление статистики / Update statistics
    handle->stats.total_commands++;
    handle->confidence_sum += speech_result->confidence;
    handle->confidence_count++;
    handle->stats.average_confidence = handle->confidence_sum / handle->confidence_count;
    
    // Создание команды / Create command
    voice_command_t command = {0};
    strncpy(command.text, speech_result->text, sizeof(command.text) - 1);
    command.confidence = speech_result->confidence;
    
    // Парсинг команды / Parse command
    if (parse_command(speech_result->text, &command)) {
        handle->stats.recognized_commands++;
        
        ESP_LOGI(TAG, "✅ Command recognized: '%s' -> %s (confidence: %.2f)", 
                 speech_result->text, command.command, command.confidence);
        
        // Выполнение команды / Execute command
        if (handle->execution_callback) {
            handle->execution_callback(&command, handle->user_data);
        } else {
            execute_command(&command, handle->user_data);
        }
    } else {
        handle->stats.unknown_commands++;
        ESP_LOGW(TAG, "❓ Unknown command: '%s' (confidence: %.2f)", 
                 speech_result->text, speech_result->confidence);
    }
    
    return ESP_OK;
}

esp_err_t voice_command_processor_set_callback(voice_command_processor_handle_t handle,
                                               command_execution_callback_t callback, 
                                               void* user_data) {
    if (!handle) {
        return ESP_ERR_INVALID_ARG;
    }
    
    handle->execution_callback = callback;
    handle->user_data = user_data;
    
    return ESP_OK;
}

esp_err_t voice_command_processor_get_stats(voice_command_processor_handle_t handle, 
                                            command_stats_t* stats) {
    if (!handle || !stats) {
        return ESP_ERR_INVALID_ARG;
    }
    
    *stats = handle->stats;
    return ESP_OK;
}

esp_err_t voice_command_processor_reset_stats(voice_command_processor_handle_t handle) {
    if (!handle) {
        return ESP_ERR_INVALID_ARG;
    }
    
    memset(&handle->stats, 0, sizeof(command_stats_t));
    handle->confidence_sum = 0.0f;
    handle->confidence_count = 0;
    
    return ESP_OK;
}