#ifndef VOICE_COMMANDS_H
#define VOICE_COMMANDS_H

#include <stdint.h>
#include <stdbool.h>

// Результат распознавания речи / Speech recognition result
typedef struct {
    char command[64];        // Распознанная команда / Recognized command
    float confidence;        // Уверенность распознавания / Recognition confidence
    bool is_final;          // Финальный результат / Final result
} speech_result_t;

// Инициализация системы голосовых команд / Initialize voice command system
esp_err_t voice_commands_init(void);

// Обработать результат распознавания речи / Process speech recognition result
esp_err_t voice_commands_process_result(const speech_result_t* result);

// Получить callback для результатов распознавания / Get callback for speech recognition results
speech_result_callback_t voice_commands_get_callback(void);

#endif // VOICE_COMMANDS_H