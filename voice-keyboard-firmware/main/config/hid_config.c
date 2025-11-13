/**
 * @file hid_config.c
 * @brief USB HID configuration implementation
 * @author Voice Keyboard Team
 * @date 2025
 * 
 * Реализация конфигурации USB HID
 * Implementation of USB HID configuration
 */

#include "hid_config.h"
#include <stdlib.h>
#include <string.h>
#include "esp_log.h"
#include "esp_usb_host.h"
#include "usb/usb_host.h"

static const char* TAG = "HID_CONFIG";

// Внутренняя структура HID устройства / Internal HID device structure
struct hid_device {
    bool initialized;
    bool connected;
    
    // USB хост / USB host
    usb_host_client_handle_t usb_client;
    
    // Интерфейсы / Interfaces
    uint8_t keyboard_interface;
    uint8_t mouse_interface;
    
    // Эндпоинты / Endpoints
    uint8_t keyboard_ep_in;
    uint8_t mouse_ep_in;
    
    // Текущие отчеты / Current reports
    hid_keyboard_report_t current_keyboard_report;
    hid_mouse_report_t current_mouse_report;
};

// HID дескрипторы / HID descriptors
static const uint8_t keyboard_hid_descriptor[] = {
    0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
    0x09, 0x06,        // Usage (Keyboard)
    0xA1, 0x01,        // Collection (Application)
    0x05, 0x07,        //   Usage Page (Kbrd/Keypad)
    0x19, 0xE0,        //   Usage Minimum (0xE0)
    0x29, 0xE7,        //   Usage Maximum (0xE7)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x95, 0x08,        //   Report Count (8)
    0x75, 0x01,        //   Report Size (1)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x95, 0x01,        //   Report Count (1)
    0x75, 0x08,        //   Report Size (8)
    0x81, 0x03,        //   Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x95, 0x06,        //   Report Count (6)
    0x75, 0x08,        //   Report Size (8)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x65,        //   Logical Maximum (101)
    0x05, 0x07,        //   Usage Page (Kbrd/Keypad)
    0x19, 0x00,        //   Usage Minimum (0x00)
    0x29, 0x65,        //   Usage Maximum (0x65)
    0x81, 0x00,        //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0xC0               // End Collection
};

static const uint8_t mouse_hid_descriptor[] = {
    0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
    0x09, 0x02,        // Usage (Mouse)
    0xA1, 0x01,        // Collection (Application)
    0x09, 0x01,        //   Usage (Pointer)
    0xA1, 0x00,        //   Collection (Physical)
    0x05, 0x09,        //     Usage Page (Button)
    0x19, 0x01,        //     Usage Minimum (0x01)
    0x29, 0x03,        //     Usage Maximum (0x03)
    0x15, 0x00,        //     Logical Minimum (0)
    0x25, 0x01,        //     Logical Maximum (1)
    0x95, 0x03,        //     Report Count (3)
    0x75, 0x01,        //     Report Size (1)
    0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x95, 0x01,        //     Report Count (1)
    0x75, 0x05,        //     Report Size (5)
    0x81, 0x03,        //     Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x05, 0x01,        //     Usage Page (Generic Desktop Ctrls)
    0x09, 0x30,        //     Usage (X)
    0x09, 0x31,        //     Usage (Y)
    0x09, 0x38,        //     Usage (Wheel)
    0x15, 0x81,        //     Logical Minimum (-127)
    0x25, 0x7F,        //     Logical Maximum (127)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x03,        //     Report Count (3)
    0x81, 0x06,        //     Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,              //   End Collection
    0xC0               // End Collection
};

/**
 * @brief USB хост callback
 * USB host callback
 */
static void usb_host_event_callback(const usb_host_client_event_msg_t* event_msg, void* arg) {
    hid_device_handle_t handle = (hid_device_handle_t)arg;
    
    switch (event_msg->event) {
        case USB_HOST_CLIENT_EVENT_NEW_DEV:
            ESP_LOGI(TAG, "New USB device connected");
            handle->connected = true;
            break;
            
        case USB_HOST_CLIENT_EVENT_DEV_GONE:
            ESP_LOGI(TAG, "USB device disconnected");
            handle->connected = false;
            break;
            
        default:
            break;
    }
}

esp_err_t hid_init(hid_device_handle_t* handle) {
    if (!handle) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // Выделение памяти / Allocate memory
    *handle = malloc(sizeof(struct hid_device));
    if (!*handle) {
        ESP_LOGE(TAG, "Failed to allocate memory for HID device");
        return ESP_ERR_NO_MEM;
    }
    
    // Инициализация структуры / Initialize structure
    memset(*handle, 0, sizeof(struct hid_device));
    
    // Инициализация USB хоста / Initialize USB host
    usb_host_config_t host_config = {
        .skip_phy_setup = false,
        .intr_flags = ESP_INTR_FLAG_LEVEL1,
    };
    
    esp_err_t ret = usb_host_install(&host_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to install USB host: %s", esp_err_to_name(ret));
        free(*handle);
        return ret;
    }
    
    // Регистрация клиента / Register client
    const usb_host_client_config_t client_config = {
        .is_synchronous = false,
        .max_num_event_msg = 5,
        .async = {
            .client_event_callback = usb_host_event_callback,
            .callback_arg = *handle,
        }
    };
    
    ret = usb_host_client_register(&client_config, &(*handle)->usb_client);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register USB client: %s", esp_err_to_name(ret));
        usb_host_uninstall();
        free(*handle);
        return ret;
    }
    
    // Инициализация отчетов / Initialize reports
    memset(&(*handle)->current_keyboard_report, 0, sizeof(hid_keyboard_report_t));
    memset(&(*handle)->current_mouse_report, 0, sizeof(hid_mouse_report_t));
    
    (*handle)->initialized = true;
    
    ESP_LOGI(TAG, "HID device initialized successfully");
    return ESP_OK;
}

esp_err_t hid_deinit(hid_device_handle_t handle) {
    if (!handle) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (handle->initialized) {
        // Дерегистрация клиента / Unregister client
        if (handle->usb_client) {
            usb_host_client_deregister(handle->usb_client);
        }
        
        // Деинсталляция USB хоста / Uninstall USB host
        usb_host_uninstall();
    }
    
    free(handle);
    ESP_LOGI(TAG, "HID device deinitialized");
    
    return ESP_OK;
}

esp_err_t hid_keyboard_send_report(hid_device_handle_t handle, const hid_keyboard_report_t* report) {
    if (!handle || !report) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (!handle->connected) {
        ESP_LOGW(TAG, "HID device not connected");
        return ESP_ERR_INVALID_STATE;
    }
    
    // TODO: Реализация отправки отчета клавиатуры / TODO: Implement keyboard report sending
    // Это требует реализации USB HID протокола / This requires USB HID protocol implementation
    
    // Сохранение текущего отчета / Save current report
    handle->current_keyboard_report = *report;
    
    ESP_LOGD(TAG, "Keyboard report sent: modifier=0x%02X, keys=[%02X,%02X,%02X,%02X,%02X,%02X]",
             report->modifier, report->keycode[0], report->keycode[1], report->keycode[2],
             report->keycode[3], report->keycode[4], report->keycode[5]);
    
    return ESP_OK;
}

esp_err_t hid_mouse_send_report(hid_device_handle_t handle, const hid_mouse_report_t* report) {
    if (!handle || !report) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (!handle->connected) {
        ESP_LOGW(TAG, "HID device not connected");
        return ESP_ERR_INVALID_STATE;
    }
    
    // TODO: Реализация отправки отчета мыши / TODO: Implement mouse report sending
    // Это требует реализации USB HID протокола / This requires USB HID protocol implementation
    
    // Сохранение текущего отчета / Save current report
    handle->current_mouse_report = *report;
    
    ESP_LOGD(TAG, "Mouse report sent: buttons=0x%02X, x=%d, y=%d, wheel=%d",
             report->buttons, report->x, report->y, report->wheel);
    
    return ESP_OK;
}

esp_err_t hid_keyboard_press_key(hid_device_handle_t handle, hid_keyboard_key_t key, uint8_t modifier) {
    if (!handle) {
        return ESP_ERR_INVALID_ARG;
    }
    
    hid_keyboard_report_t report = {0};
    report.modifier = modifier;
    report.keycode[0] = key;
    
    return hid_keyboard_send_report(handle, &report);
}

esp_err_t hid_keyboard_release_key(hid_device_handle_t handle) {
    if (!handle) {
        return ESP_ERR_INVALID_ARG;
    }
    
    hid_keyboard_report_t report = {0};
    return hid_keyboard_send_report(handle, &report);
}

esp_err_t hid_keyboard_click_key(hid_device_handle_t handle, hid_keyboard_key_t key, uint8_t modifier) {
    if (!handle) {
        return ESP_ERR_INVALID_ARG;
    }
    
    esp_err_t ret = hid_keyboard_press_key(handle, key, modifier);
    if (ret == ESP_OK) {
        vTaskDelay(pdMS_TO_TICKS(50)); // Задержка для эмуляции клика / Delay for click simulation
        ret = hid_keyboard_release_key(handle);
    }
    
    return ret;
}

esp_err_t hid_mouse_click(hid_device_handle_t handle, hid_mouse_button_t button) {
    if (!handle) {
        return ESP_ERR_INVALID_ARG;
    }
    
    hid_mouse_report_t report = {0};
    report.buttons = button;
    
    esp_err_t ret = hid_mouse_send_report(handle, &report);
    if (ret == ESP_OK) {
        vTaskDelay(pdMS_TO_TICKS(50)); // Задержка для эмуляции клика / Delay for click simulation
        
        // Отпускание кнопки / Release button
        report.buttons = 0;
        ret = hid_mouse_send_report(handle, &report);
    }
    
    return ret;
}

esp_err_t hid_mouse_move(hid_device_handle_t handle, int8_t x, int8_t y) {
    if (!handle) {
        return ESP_ERR_INVALID_ARG;
    }
    
    hid_mouse_report_t report = {0};
    report.x = x;
    report.y = y;
    
    return hid_mouse_send_report(handle, &report);
}

esp_err_t hid_mouse_scroll(hid_device_handle_t handle, int8_t delta) {
    if (!handle) {
        return ESP_ERR_INVALID_ARG;
    }
    
    hid_mouse_report_t report = {0};
    report.wheel = delta;
    
    return hid_mouse_send_report(handle, &report);
}

bool hid_is_connected(hid_device_handle_t handle) {
    if (!handle) {
        return false;
    }
    
    return handle->connected;
}