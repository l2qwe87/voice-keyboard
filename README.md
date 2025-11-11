# Голосовая клавиатура - Реализация на ESP32

## Обзор проекта
Голосовая клавиатура на базе ESP32 C3 SuperMini, которая преобразует речь в текст и вводит его как HID клавиатуру.

---

## Аппаратные компоненты
- **ESP32 C3 SuperMini** - Основной микроконтроллер
- **I2S Микрофон (MEMS)** - Цифровой микрофон для записи голоса
- **Тактильная кнопка** - Управление записью
- **Светодиодный индикатор** - Визуальная индикация состояния
- **Подключение USB-C** - Программирование и питание

## Конфигурация выводов
- **Кнопка**: GPIO 0
- **Светодиод**: GPIO 1  
- **I2S WS**: GPIO 2 (Word Select)
- **I2S SD**: GPIO 3 (Serial Data)
- **I2S SCK**: GPIO 4 (Serial Clock)

## Функциональные возможности
- **BLE HID эмуляция клавиатуры** - Беспроводное подключение к устройствам
- **I2S запись аудио** - 16кГц, 16-бит качество записи
- **Управление записью через кнопку** - Простое управление записью
- **LED индикация состояния** - Визуальная обратная связь
- **Базовый вывод текста как клавиатурный ввод** - Преобразование речи в текст

## Статус разработки

### Этап 1: Планирование и документация ✅ ЗАВЕРШЁН
- [x] Технические требования и спецификации
- [x] Схема подключений и выбор компонентов
- [x] Документация проекта (README, BOM, CIRCUIT)
- [x] Настройка ESP-IDF среды разработки
- [x] Создание базовой структуры проекта

### Этап 2: Прототипирование (В ПРОЦЕССЕ)
- [x] Базовая структура ESP-IDF проекта
- [ ] Реализация I2S захвата аудио с микрофона INMP441
- [ ] Управление внешней тактильной кнопкой (GPIO 0)
- [ ] LED индикация состояния записи (GPIO 1)
- [ ] Базовая BLE HID клавиатура

### Этап 3: Распознавание речи (СЛЕДУЮЩИЙ)
- [ ] Интеграция Vosk API для офлайн распознавания
- [ ] Потоковая обработка аудио данных
- [ ] Обработка голосовых команд
- [ ] Преобразование текста в HID ввод

### Этап 4: Оптимизация и тестирование
- [ ] Оптимизация энергопотребления
- [ ] Тестирование качества распознавания
- [ ] Совместимость с различными ОС
- [ ] Дизайн корпуса и механическая сборка

## Сборка и прошивка

### Для ESP-IDF (рекомендуется)
```bash
# Настройка окружения ESP-IDF (v5.5.1)
# ESP-IDF путь: Z:\Apps\esp-idf\v5.5.1\esp-idf
# Python путь: C:\Python312\python.exe

# Активация ESP-IDF окружения
cd Z:\Apps\esp-idf\v5.5.1\esp-idf
export.bat

# Клонирование проекта
git clone https://github.com/l2qwe87/voice-keyboard.git
cd voice-keyboard/voice-keyboard-firmware

# Установка target (ESP32-C3)
idf.py set-target esp32c3

# Сборка проекта
idf.py build

# Загрузка в ESP32
idf.py -p COM3 flash

# Мониторинг последовательного порта
idf.py -p COM3 monitor
```

### Для PlatformIO (альтернатива)
```bash
# Установка PlatformIO если не установлен
pip install platformio

# Сборка проекта
pio run

# Загрузка в ESP32
pio run --target upload

# Мониторинг последовательного порта
pio device monitor
```

## Использование устройства

### Первичная настройка
1. **Подключите устройство к компьютеру** через USB для питания и программирования
2. **Сопряжение с компьютером** через BLE (устройство отображается как "Voice Keyboard")
3. **Проверьте работу базовых функций** - LED индикация, отклик кнопки

### Ежедневное использование
1. **Нажмите и удерживайте кнопку** для начала записи голоса
2. **Говорите чётко** в микрофон с расстояния 10-30см
3. **Отпустите кнопку** для остановки записи и начала обработки
4. **Текст будет автоматически напечатан** в активном поле ввода
5. **LED индикатор** показывает состояние устройства (включён = запись)

## Следующие шаги разработки

### Краткосрочные цели (1-2 месяца)
- **Интеграция с API распознавания речи** (Vosk/Whisper)
- **Добавление Wi-Fi подключения** для облачной обработки
- **Реализация сжатия аудио** для экономии трафика
- **Добавление опций конфигурации** через NVS

### Среднесрочные цели (3-6 месяцев)
- **Поддержка нескольких языков** распознавания
- **Улучшение качества распознавания** через фильтрацию шума
- **Добавление пользовательских команд** и горячих клавиш
- **Оптимизация энергопотребления** для увеличения времени работы

### Долгосрочные цели (6+ месяцев)
- **Создание мобильного приложения** для конфигурации
- **Интеграция с облачными сервисами** для улучшения качества
- **Массовое производство** и дистрибуция

## Требования к окружению разработки
- **ESP-IDF v5.5.1** для компиляции под ESP32-C3
- **Python 3.12** для инструментов ESP-IDF
- **Git** для управления версиями
- **VSCode с расширением ESP-IDF** (рекомендуется)
- **Windows 10/11** (текущая конфигурация)

## Устранение неисправностей

### Частые проблемы
- **Устройство не определяется**: Проверьте подключение USB и драйверы
- **BLE не сопрягается**: Убедитесь, что устройство в режиме рекламы
- **Плохое качество записи**: Проверьте подключение микрофона и настройки I2S
- **Текст не печатается**: Проверьте HID конфигурацию и сопряжение BLE

### Отладка
- Используйте `idf.py monitor` для просмотра логов
- Проверьте конфигурацию выводов в `sdkconfig`
- Убедитесь в правильности установки инструментов ESP-IDF

---

## Voice Keyboard - ESP32 Implementation

## Project Overview
ESP32 C3 SuperMini based voice keyboard that converts speech to text and inputs it as a HID keyboard.

---

## Hardware Components
- **ESP32 C3 SuperMini** - Main microcontroller
- **I2S Microphone (MEMS)** - Digital microphone for voice recording
- **Push Button** - Recording control
- **LED Indicator** - Visual status indication
- **USB-C Connection** - Programming and power

## Pin Configuration
- **Button**: GPIO 0
- **LED**: GPIO 1  
- **I2S WS**: GPIO 2 (Word Select)
- **I2S SD**: GPIO 3 (Serial Data)
- **I2S SCK**: GPIO 4 (Serial Clock)

## Features
- **BLE HID Keyboard Emulation** - Wireless device connection
- **I2S Audio Recording** - 16kHz, 16-bit recording quality
- **Button-Controlled Recording** - Simple recording control
- **LED Status Indication** - Visual feedback
- **Basic Text-to-Keyboard Output** - Speech to text conversion

## Development Status

### Phase 1: Planning & Documentation ✅ COMPLETED
- [x] Technical requirements and specifications
- [x] Circuit design and component selection
- [x] Project documentation (README, BOM, CIRCUIT)
- [x] ESP-IDF development environment setup
- [x] Basic ESP-IDF project structure

### Phase 2: Prototyping (IN PROGRESS)
- [x] Basic ESP-IDF project structure
- [ ] I2S audio capture with INMP441 microphone
- [ ] External tactile button control (GPIO 0)
- [ ] LED status indication (GPIO 1)
- [ ] Basic BLE HID keyboard implementation

### Phase 3: Speech Recognition (NEXT)
- [ ] Vosk API integration for offline recognition
- [ ] Audio data stream processing
- [ ] Voice command processing
- [ ] Text to HID input conversion

### Phase 4: Optimization & Testing
- [ ] Power consumption optimization
- [ ] Recognition quality testing
- [ ] Cross-platform compatibility
- [ ] Enclosure design and mechanical assembly

## Building and Flashing

### For ESP-IDF (Recommended)
```bash
# Setup ESP-IDF environment (v5.5.1)
# ESP-IDF path: Z:\Apps\esp-idf\v5.5.1\esp-idf
# Python path: C:\Python312\python.exe

# Activate ESP-IDF environment
cd Z:\Apps\esp-idf\v5.5.1\esp-idf
export.bat

# Clone project
git clone https://github.com/l2qwe87/voice-keyboard.git
cd voice-keyboard/voice-keyboard-firmware

# Set target (ESP32-C3)
idf.py set-target esp32c3

# Build project
idf.py build

# Flash to ESP32
idf.py -p COM3 flash

# Monitor serial output
idf.py -p COM3 monitor
```

### For PlatformIO (Alternative)
```bash
# Install PlatformIO if not already installed
pip install platformio

# Build project
pio run

# Upload to ESP32
pio run --target upload

# Monitor serial output
pio device monitor
```

## Usage

### Initial Setup
1. **Connect device to computer** via USB for power and programming
2. **Pair with computer** via BLE (device appears as "Voice Keyboard")
3. **Test basic functions** - LED indication, button response

### Daily Usage
1. **Press and hold button** to start voice recording
2. **Speak clearly** into microphone from 10-30cm distance
3. **Release button** to stop recording and start processing
4. **Text will be automatically typed** in active input field
5. **LED indicator** shows device status (ON = recording)

## Next Development Steps

### Short-term Goals (1-2 months)
- **Integrate speech recognition API** (Vosk/Whisper)
- **Add Wi-Fi connectivity** for cloud processing
- **Implement audio compression** for bandwidth efficiency
- **Add configuration options** via NVS

### Medium-term Goals (3-6 months)
- **Multi-language support** for recognition
- **Improve recognition quality** through noise filtering
- **Add custom commands** and hotkeys
- **Optimize power consumption** for longer battery life

### Long-term Goals (6+ months)
- **Create mobile app** for configuration
- **Integrate cloud services** for quality improvement
- **Mass production** and distribution

## Development Environment Requirements
- **ESP-IDF v5.5.1** for ESP32-C3 compilation
- **Python 3.12** for ESP-IDF tools
- **Git** for version control
- **VSCode with ESP-IDF extension** (recommended)
- **Windows 10/11** (current configuration)

## Troubleshooting

### Common Issues
- **Device not recognized**: Check USB connection and drivers
- **BLE not pairing**: Ensure device is in advertising mode
- **Poor recording quality**: Check microphone connection and I2S settings
- **Text not typing**: Verify HID configuration and BLE pairing

### Debugging
- Use `idf.py monitor` to view logs
- Check pin configuration in `sdkconfig`
- Ensure ESP-IDF tools are properly installed