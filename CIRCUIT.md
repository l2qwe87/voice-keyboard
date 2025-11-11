# Голосовая клавиатура - Схема подключения (Circuit Diagram)

## Обзор документа
Этот документ описывает схему подключения компонентов для голосовой клавиатуры на базе ESP32 C3 SuperMini. Включены спецификации компонентов, расчёты питания и рекомендации по проектированию PCB.

---

## Подключения ESP32 C3 SuperMini

```
ESP32 C3 SuperMini
┌─────────────────┐
│                 │
│ GPIO 0 ────────┼───┬─── [ВНЕШНЯЯ КНОПКА] ──── GND
│                 │   │   (через разъём/провода)
│ GPIO 1 ────────┼───┴─── [220Ω] ──── [LED] ──── GND
│                 │
│ GPIO 2 ────────┼─── WS (Word Select)
│                 │
│ GPIO 3 ────────┼─── SD (Serial Data)
│                 │
│ GPIO 4 ────────┼─── SCK (Serial Clock)
│                 │
│ 3V3 ───────────┼─── VCC (Microphone Power)
│                 │
│ GND ───────────┼─── GND (Microphone Ground)
│                 │
│ USB-C ─────────┼─── Programming & Power
│                 │
└─────────────────┘

ВНЕШНЯЯ КНОПКА (6мм/12мм)
┌─────────────────┐
│                 │
│ Контакт 1 ──────┼─── К GPIO 0 ESP32
│                 │
│ Контакт 2 ──────┼─── К GND ESP32
│                 │
└─────────────────┘
(Подключается через Dupont провода или разъём JST)

I2S Микрофон (INMP441)
┌─────────────────┐
│                 │
│ VCC ────────────┼─── 3.3V
│                 │
│ GND ────────────┼─── GND
│                 │
│ SCK ────────────┼─── GPIO 4
│                 │
│ WS ─────────────┼─── GPIO 2
│                 │
│ SD ─────────────┼─── GPIO 3
│                 │
│ L/R ────────────┼─── GND (Выбор левого канала)
│                 │
└─────────────────┘
```

## Дополнительная схема питания

```
LiPo Аккумулятор ────┐
                    │
               [TP4056]
                    │
            3.7V ───┼─── ESP32 VIN
                    │
            GND ────┼─── ESP32 GND
```

## Спецификации компонентов

### ESP32 C3 SuperMini
- Микроконтроллер: ESP32-C3
- GPIO: 11 доступных выводов
- Коммуникации: Wi-Fi, Bluetooth 5 (LE)
- Питание: 3.3V, 5V через USB
- Примечание: Компактный форм-фактор SuperMini

### I2S Микрофон (INMP441)
- Тип: Цифровой MEMS микрофон
- Интерфейс: I2S
- Частота дискретизации: До 32кГц
- Разрядность: 24-бит
- Питание: 3.3V
- SNR: 61дБ
- Чувствительность: -26дБFS
- Примечание: Цифровой интерфейс упрощает подключение

### Внешняя кнопка
- Тип: Внешняя тактильная кнопка (6мм/12мм)
- Конфигурация: Pull-up (внутренний ESP32)
- Срабатывание: Active LOW
- Подключение: Через Dupont провода или разъём JST-XH/PH2.0
- Примечание: Внешнее размещение для удобного доступа
- Длина проводов: 10-20см для гибкого монтажа

### Светодиодный индикатор
- Тип: Стандартный LED
- Токоограничение: Резистор 220Ω
- Функция: Индикация записи (ВКЛ = запись)
- Примечание: Визуальная обратная связь для пользователя

## Требования к питанию

### Потребляемый ток
- ESP32 C3 (активный режим): ~80мА
- ESP32 C3 (режим сна): ~10мкА
- I2S Микрофон: ~1.7мА
- Светодиод: ~10мА

### Общий ток потребления
- Режим записи: ~92мА
- Режим ожидания: ~10мкА + BLE реклама ~15мА

### Время работы от батареи (500мАч LiPo)
- Непрерывная запись: ~5.4 часов
- Ожидание с BLE: ~33 часов
- Смешанное использование: ~8-12 часов

## Рекомендации по проектированию PCB

### Целостность сигналов
- Держать I2S дорожки короткими и параллельными
- Добавлять развязывающие конденсаторы рядом с ESP32
- Разделять аналоговую и цифровую землю

### Питание
- Добавлять конденсатор 100мкФ на входе питания
- Использовать LDO 3.3V при питании от батареи
- Рассмотреть управление питанием микрофона

### Механические аспекты
- Внешняя кнопка размещается отдельно от основной платы
- Длина проводов 10-20см для удобного размещения
- Позиционировать LED для хорошей видимости
- Учитывать размещение разъёма USB-C
- Рассмотреть возможность крепления кнопки на корпусе или кабеле

---

## Circuit Diagram - Hardware Connections

## ESP32 C3 SuperMini Connections

```
ESP32 C3 SuperMini
┌─────────────────┐
│                 │
│ GPIO 0 ────────┼───┬─── [EXTERNAL BUTTON] ──── GND
│                 │   │   (via connector/wires)
│ GPIO 1 ────────┼───┴─── [220Ω] ──── [LED] ──── GND
│                 │
│ GPIO 2 ────────┼─── WS (Word Select)
│                 │
│ GPIO 3 ────────┼─── SD (Serial Data)
│                 │
│ GPIO 4 ────────┼─── SCK (Serial Clock)
│                 │
│ 3V3 ───────────┼─── VCC (Microphone Power)
│                 │
│ GND ───────────┼─── GND (Microphone Ground)
│                 │
│ USB-C ─────────┼─── Programming & Power
│                 │
└─────────────────┘

EXTERNAL BUTTON (6mm/12mm)
┌─────────────────┐
│                 │
│ Contact 1 ──────┼─── To ESP32 GPIO 0
│                 │
│ Contact 2 ──────┼─── To ESP32 GND
│                 │
└─────────────────┘
(Connected via Dupont wires or JST connector)

I2S Microphone (INMP441)
┌─────────────────┐
│                 │
│ VCC ────────────┼─── 3.3V
│                 │
│ GND ────────────┼─── GND
│                 │
│ SCK ────────────┼─── GPIO 4
│                 │
│ WS ─────────────┼─── GPIO 2
│                 │
│ SD ─────────────┼─── GPIO 3
│                 │
│ L/R ────────────┼─── GND (Left channel select)
│                 │
└─────────────────┘
```

## Optional Battery Circuit

```
LiPo Battery ────┐
                 │
            [TP4056]
                 │
         3.7V ───┼─── ESP32 VIN
                 │
         GND ────┼─── ESP32 GND
```

## Component Specifications

### ESP32 C3 SuperMini
- Microcontroller: ESP32-C3
- GPIO: 11 available pins
- Communication: Wi-Fi, Bluetooth 5 (LE)
- Power: 3.3V, 5V via USB
- Note: Compact SuperMini form factor

### I2S Microphone (INMP441)
- Type: Digital MEMS microphone
- Interface: I2S
- Sample Rate: Up to 32kHz
- Bit Depth: 24-bit
- Power Supply: 3.3V
- SNR: 61dB
- Sensitivity: -26dBFS
- Note: Digital interface simplifies connection

### External Button
- Type: External tactile push button (6mm/12mm)
- Configuration: Pull-up (internal ESP32)
- Action: Active LOW
- Connection: Via Dupont wires or JST-XH/PH2.0 connector
- Note: External mounting for easy access
- Wire length: 10-20cm for flexible mounting

### LED Indicator
- Type: Standard LED
- Current Limiting: 220Ω resistor
- Function: Recording status (ON = recording)
- Note: Visual feedback for user

## Power Requirements

### Operating Current
- ESP32 C3 (active): ~80mA
- ESP32 C3 (sleep): ~10μA
- I2S Microphone: ~1.7mA
- LED: ~10mA

### Total Current
- Recording: ~92mA
- Standby: ~10μA + BLE advertising ~15mA

### Battery Life (500mAh LiPo)
- Continuous recording: ~5.4 hours
- Standby with BLE: ~33 hours
- Mixed usage: ~8-12 hours

## PCB Design Considerations

### Signal Integrity
- Keep I2S traces short and parallel
- Add decoupling capacitors near ESP32
- Separate analog and digital grounds

### Power Supply
- Add 100μF capacitor at power input
- Use 3.3V LDO if battery powered
- Consider power switching for microphone

### Mechanical
- External button mounted separately from main board
- Wire length 10-20cm for flexible placement
- Position LED for visibility
- Consider USB-C connector placement
- Consider button mounting on enclosure or cable