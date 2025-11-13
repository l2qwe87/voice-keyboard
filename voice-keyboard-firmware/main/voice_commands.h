#ifndef VOICE_COMMANDS_H
#define VOICE_COMMANDS_H

#include <stdint.h>
#include <stdbool.h>
#include "config/speech_recognition.h"

// Инициализация системы голосовых команд / Initialize voice command system
esp_err_t voice_commands_init(void);

// Обработать результат распознавания речи / Process speech recognition result
esp_err_t voice_commands_process_result(const speech_result_t* result);

// Получить callback для результатов распознавания / Get callback for speech recognition results
speech_result_callback_t voice_commands_get_callback(void);

#endif // VOICE_COMMANDS_H