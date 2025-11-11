# Голосовая клавиатура - Спецификация компонентов (Bill of Materials)

## Обзор документа
Этот документ содержит полный перечень необходимых электронных компонентов для создания голосовой клавиатуры на базе ESP32 C3 SuperMini. Включены цены, рекомендации по поставщикам и необходимые инструменты.

---

## Основные компоненты

### Главный контроллер
- **ESP32 C3 SuperMini** - Основной микроконтроллер с BLE и Wi-Fi
  - Количество: 1
  - Приблизительная стоимость: $3-5
  - Примечание: Компактная плата с необходимыми интерфейсами

### Аудиовход
- **I2S MEMS микрофон** - Цифровой микрофон для записи голоса
  - Рекомендуется: INMP441 или аналогичный
  - Количество: 1
  - Приблизительная стоимость: $2-4
  - Примечание: Цифровой интерфейс упрощает подключение

### Пользовательский интерфейс
- **Тактильная кнопка** - Управление записью
  - Размер: 6мм или 12мм
  - Количество: 1
  - Приблизительная стоимость: $0.10-0.50
  - Примечание: Для надёжного срабатывания

- **Светодиодный индикатор** - Индикация состояния записи
  - Размер: 3мм или 5мм (любой цвет)
  - Количество: 1
  - Приблизительная стоимость: $0.10-0.20
  - Примечание: Визуальная обратная связь

- **Токоограничивающий резистор** - Защита светодиода
  - Номинал: 220Ω или 330Ω
  - Количество: 1
  - Приблизительная стоимость: $0.05
  - Примечание: Предотвращает перегорание LED

### Питание и подключение
- **Разъём USB-C** (если отсутствует на плате)
  - Назначение: Программирование и питание
  - Количество: 1
  - Приблизительная стоимость: $0.50-1.00
  - Примечание: Современный стандарт подключения

### Дополнительные компоненты
- **LiPo аккумулятор** - Автономная работа
  - Ёмкость: 500мАч-1000мАч
  - Количество: 1
  - Приблизительная стоимость: $3-6
  - Примечание: Для портативного использования

- **Зарядное устройство** - Модуль TP4056
  - Количество: 1
  - Приблизительная стоимость: $1-2
  - Примечание: Безопасная зарядка LiPo

- **Печатная плата** - Заказная плата для сборки
  - Количество: 1
  - Приблизительная стоимость: $2-5 (малая партия)
  - Примечание: Профессиональный вид монтажа

## Общая оценочная стоимость
- **Базовая версия**: $6-10
- **Портативная версия**: $10-15

## Рекомендуемые поставщики
- **AliExpress** - Низкие цены, долгая доставка
- **LCSC** - Быстрая доставка из Китая
- **Digi-Key** - Надёжность, широкий ассортимент
- **Mouser** - Профессиональные компоненты
- **Местные магазины электроники** - Немедленная доступность

## Необходимые инструменты
- **Паяльник** - Для монтажа компонентов
- **Припой** - Для создания паяных соединений
- **Мультиметр** - Для проверки соединений
- **Макетная плата** - Для прототипирования
- **Соединительные провода** - Для временных подключений
- **USB-C кабель** - Для программирования

---

## Bill of Materials - Hardware Components

### Main Controller
- **ESP32 C3 SuperMini** - Main microcontroller with BLE and Wi-Fi
  - Quantity: 1
  - Approx. Cost: $3-5
  - Note: Compact board with required interfaces

### Audio Input
- **I2S MEMS Microphone** - Digital microphone for voice recording
  - Recommended: INMP441 or similar
  - Quantity: 1
  - Approx. Cost: $2-4
  - Note: Digital interface simplifies connection

### User Interface
- **Tactile Push Button** - Recording control
  - Size: 6mm or 12mm
  - Quantity: 1
  - Approx. Cost: $0.10-0.50
  - Note: For reliable actuation

- **LED Indicator** - Recording status
  - Size: 3mm or 5mm (any color)
  - Quantity: 1
  - Approx. Cost: $0.10-0.20
  - Note: Visual feedback

- **Current Limiting Resistor** - LED protection
  - Value: 220Ω or 330Ω
  - Quantity: 1
  - Approx. Cost: $0.05
  - Note: Prevents LED burnout

### Power & Connection
- **USB-C Connector** (if not on board)
  - Purpose: Programming and power
  - Quantity: 1
  - Approx. Cost: $0.50-1.00
  - Note: Modern connection standard

### Optional Components
- **LiPo Battery** - Portable operation
  - Capacity: 500mAh-1000mAh
  - Quantity: 1
  - Approx. Cost: $3-6
  - Note: For portable use

- **Battery Charging Circuit** - TP4056 module
  - Quantity: 1
  - Approx. Cost: $1-2
  - Note: Safe LiPo charging

- **PCB** - Custom board for assembly
  - Quantity: 1
  - Approx. Cost: $2-5 (small batch)
  - Note: Professional mounting appearance

## Total Estimated Cost
- **Basic Version**: $6-10
- **Portable Version**: $10-15

## Recommended Suppliers
- **AliExpress** - Low prices, long shipping
- **LCSC** - Fast shipping from China
- **Digi-Key** - Reliable, wide selection
- **Mouser** - Professional components
- **Local electronics stores** - Immediate availability

## Tools Required
- **Soldering iron** - For component mounting
- **Solder wire** - For creating soldered connections
- **Multimeter** - For connection testing
- **Breadboard** - For prototyping
- **Jumper wires** - For temporary connections
- **USB-C cable** - For programming