#ifndef AUDIO_TASK_H
#define AUDIO_TASK_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Создать задачу обработки аудио / Create audio processing task
void create_audio_task(void);

#endif // AUDIO_TASK_H