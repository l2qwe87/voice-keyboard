/**
 * @file hid_config.h
 * @brief USB HID configuration header
 * @author Voice Keyboard Team
 * @date 2025
 * 
 * Заголовочный файл конфигурации USB HID
 * Header file for USB HID configuration
 */

#ifndef HID_CONFIG_H
#define HID_CONFIG_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

// HID интерфейсы / HID interfaces
#define HID_INTERFACE_KEYBOARD    0
#define HID_INTERFACE_MOUSE       1
#define HID_INTERFACE_COUNT       2

// HID конфигурация USB / HID USB configuration
#define HID_USB_VID             0x2E8A    // Espressif VID
#define HID_USB_PID             0x0001    // Custom PID
#define HID_USB_MANUFACTURER    "Espressif"
#define HID_USB_PRODUCT         "Voice Keyboard"
#define HID_USB_SERIAL          "123456"

// Клавиатура HID / Keyboard HID
#define HID_KEYBOARD_EP_IN      0x81      // Endpoint IN
#define HID_KEYBOARD_EP_OUT     0x01      // Endpoint OUT
#define HID_KEYBOARD_EP_SIZE    8         // Endpoint size
#define HID_KEYBOARD_INTERVAL   10        // Polling interval (ms)

// Мышь HID / Mouse HID
#define HID_MOUSE_EP_IN        0x82      // Endpoint IN
#define HID_MOUSE_EP_OUT       0x02      // Endpoint OUT
#define HID_MOUSE_EP_SIZE      4         // Endpoint size
#define HID_MOUSE_INTERVAL     10        // Polling interval (ms)

// Модификаторы клавиатуры / Keyboard modifiers
typedef enum {
    HID_MODIFIER_LEFT_CTRL   = 0x01,
    HID_MODIFIER_LEFT_SHIFT  = 0x02,
    HID_MODIFIER_LEFT_ALT    = 0x04,
    HID_MODIFIER_LEFT_GUI    = 0x08,
    HID_MODIFIER_RIGHT_CTRL  = 0x10,
    HID_MODIFIER_RIGHT_SHIFT = 0x20,
    HID_MODIFIER_RIGHT_ALT   = 0x40,
    HID_MODIFIER_RIGHT_GUI   = 0x80
} hid_keyboard_modifier_t;

// Коды клавиш / Key codes
typedef enum {
    HID_KEY_A = 0x04,
    HID_KEY_B = 0x05,
    HID_KEY_C = 0x06,
    HID_KEY_D = 0x07,
    HID_KEY_E = 0x08,
    HID_KEY_F = 0x09,
    HID_KEY_G = 0x0A,
    HID_KEY_H = 0x0B,
    HID_KEY_I = 0x0C,
    HID_KEY_J = 0x0D,
    HID_KEY_K = 0x0E,
    HID_KEY_L = 0x0F,
    HID_KEY_M = 0x10,
    HID_KEY_N = 0x11,
    HID_KEY_O = 0x12,
    HID_KEY_P = 0x13,
    HID_KEY_Q = 0x14,
    HID_KEY_R = 0x15,
    HID_KEY_S = 0x16,
    HID_KEY_T = 0x17,
    HID_KEY_U = 0x18,
    HID_KEY_V = 0x19,
    HID_KEY_W = 0x1A,
    HID_KEY_X = 0x1B,
    HID_KEY_Y = 0x1C,
    HID_KEY_Z = 0x1D,
    HID_KEY_1 = 0x1E,
    HID_KEY_2 = 0x1F,
    HID_KEY_3 = 0x20,
    HID_KEY_4 = 0x21,
    HID_KEY_5 = 0x22,
    HID_KEY_6 = 0x23,
    HID_KEY_7 = 0x24,
    HID_KEY_8 = 0x25,
    HID_KEY_9 = 0x26,
    HID_KEY_0 = 0x27,
    HID_KEY_ENTER = 0x28,
    HID_KEY_ESCAPE = 0x29,
    HID_KEY_BACKSPACE = 0x2A,
    HID_KEY_TAB = 0x2B,
    HID_KEY_SPACE = 0x2C,
    HID_KEY_CAPS_LOCK = 0x39,
    HID_KEY_F1 = 0x3A,
    HID_KEY_F2 = 0x3B,
    HID_KEY_F3 = 0x3C,
    HID_KEY_F4 = 0x3D,
    HID_KEY_F5 = 0x3E,
    HID_KEY_F6 = 0x3F,
    HID_KEY_F7 = 0x40,
    HID_KEY_F8 = 0x41,
    HID_KEY_F9 = 0x42,
    HID_KEY_F10 = 0x43,
    HID_KEY_F11 = 0x44,
    HID_KEY_F12 = 0x45,
    HID_KEY_RIGHT_ARROW = 0x4F,
    HID_KEY_LEFT_ARROW = 0x50,
    HID_KEY_DOWN_ARROW = 0x51,
    HID_KEY_UP_ARROW = 0x52,
} hid_keyboard_key_t;

// Кнопки мыши / Mouse buttons
typedef enum {
    HID_MOUSE_BUTTON_LEFT   = 0x01,
    HID_MOUSE_BUTTON_RIGHT  = 0x02,
    HID_MOUSE_BUTTON_MIDDLE = 0x04
} hid_mouse_button_t;

// Отчет клавиатуры / Keyboard report
typedef struct {
    uint8_t modifier;      // Модификаторы / Modifiers
    uint8_t reserved;      // Зарезервировано / Reserved
    uint8_t keycode[6];    // Коды клавиш / Key codes
} hid_keyboard_report_t;

// Отчет мыши / Mouse report
typedef struct {
    uint8_t buttons;       // Кнопки / Buttons
    int8_t x;              // Смещение по X / X displacement
    int8_t y;              // Смещение по Y / Y displacement
    int8_t wheel;          // Колесо / Wheel
    int8_t pan;            // Pan
} hid_mouse_report_t;

// Дескриптор HID / HID handle
typedef struct hid_device* hid_device_handle_t;

/**
 * @brief Инициализация HID устройства
 * Initialize HID device
 */
esp_err_t hid_init(hid_device_handle_t* handle);

/**
 * @brief Деинициализация HID устройства
 * Deinitialize HID device
 */
esp_err_t hid_deinit(hid_device_handle_t handle);

/**
 * @brief Отправить отчет клавиатуры
 * Send keyboard report
 */
esp_err_t hid_keyboard_send_report(hid_device_handle_t handle, const hid_keyboard_report_t* report);

/**
 * @brief Отправить отчет мыши
 * Send mouse report
 */
esp_err_t hid_mouse_send_report(hid_device_handle_t handle, const hid_mouse_report_t* report);

/**
 * @brief Нажать клавишу
 * Press key
 */
esp_err_t hid_keyboard_press_key(hid_device_handle_t handle, hid_keyboard_key_t key, uint8_t modifier);

/**
 * @brief Отпустить клавишу
 * Release key
 */
esp_err_t hid_keyboard_release_key(hid_device_handle_t handle);

/**
 * @brief Нажать и отпустить клавишу
 * Press and release key
 */
esp_err_t hid_keyboard_click_key(hid_device_handle_t handle, hid_keyboard_key_t key, uint8_t modifier);

/**
 * @brief Кликнуть мышью
 * Click mouse
 */
esp_err_t hid_mouse_click(hid_device_handle_t handle, hid_mouse_button_t button);

/**
 * @brief Двинуть мышью
 * Move mouse
 */
esp_err_t hid_mouse_move(hid_device_handle_t handle, int8_t x, int8_t y);

/**
 * @brief Прокрутить колесо
 * Scroll wheel
 */
esp_err_t hid_mouse_scroll(hid_device_handle_t handle, int8_t delta);

/**
 * @brief Проверить состояние HID
 * Check HID status
 */
bool hid_is_connected(hid_device_handle_t handle);

#endif // HID_CONFIG_H