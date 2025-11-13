#include "voice_commands.h"
#include "hid_task.h"
#include "esp_log.h"
#include "speech_recognition.h"
#include <string.h>

static const char *TAG = "VOICE_COMMANDS";

// Простая таблица команд / Simple command table
typedef struct {
    const char* keyword;
    uint8_t modifier;
    uint8_t keycode;
} voice_command_t;

// Таблица голосовых команд / Voice command table
static const voice_command_t command_table[] = {
    {"пробел", 0x00, HID_KEY_SPACE},           // space
    {"ввод", 0x00, HID_KEY_ENTER},             // enter
    {"таб", 0x00, HID_KEY_TAB},                // tab
    {"удалить", 0x00, HID_KEY_DELETE},         // delete
    {"бэкспейс", 0x00, HID_KEY_BACKSPACE},     // backspace
    {"экранировать", 0x00, HID_KEY_ESCAPE},    // escape
    {"стрелка вверх", 0x00, HID_KEY_UP},       // up arrow
    {"стрелка вниз", 0x00, HID_KEY_DOWN},      // down arrow
    {"стрелка влево", 0x00, HID_KEY_LEFT},     // left arrow
    {"стрелка вправо", 0x00, HID_KEY_RIGHT},    // right arrow
    {"контрол с", 0x01, HID_KEY_C},            // Ctrl+C
    {"контрол в", 0x01, HID_KEY_V},            // Ctrl+V
    {"контрол з", 0x01, HID_KEY_Z},            // Ctrl+Z
    {"альт таб", 0x04, HID_KEY_TAB},           // Alt+Tab
    {"шифт таб", 0x02, HID_KEY_TAB},           // Shift+Tab
    {"капс лок", 0x00, HID_KEY_CAPS_LOCK},     // Caps Lock
};

static const size_t command_count = sizeof(command_table) / sizeof(voice_command_t);

// Callback для результатов распознавания речи / Callback for speech recognition results
static void speech_result_callback(const speech_result_t* result) {
    if (!result || !result->is_final) {
        return;
    }
    
    ESP_LOGI(TAG, "Processing command: '%s' (confidence: %.2f)", result->command, result->confidence);
    
    // Обработать распознанную команду / Process recognized command
    voice_commands_process_result(result);
}

esp_err_t voice_commands_init(void) {
    ESP_LOGI(TAG, "Voice commands system initialized");
    return ESP_OK;
}

esp_err_t voice_commands_process_result(const speech_result_t* result) {
    if (!result || !result->is_final) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // Поиск команды в таблице / Search command in table
    for (size_t i = 0; i < command_count; i++) {
        if (strstr(result->command, command_table[i].keyword) != NULL) {
            ESP_LOGI(TAG, "Command matched: '%s' -> modifier: 0x%02X, keycode: 0x%02X", 
                     command_table[i].keyword, command_table[i].modifier, command_table[i].keycode);
            
            // Отправить HID команду / Send HID command
            hid_send_key(command_table[i].modifier, command_table[i].keycode);
            return ESP_OK;
        }
    }
    
    ESP_LOGW(TAG, "Unknown command: '%s'", result->command);
    return ESP_ERR_NOT_FOUND;
}

speech_result_callback_t voice_commands_get_callback(void) {
    return speech_result_callback;
}