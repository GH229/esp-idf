
/*
 * SPDX-FileCopyrightText: 2015-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __ESP_BT_H__
#define __ESP_BT_H__

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "sdkconfig.h"
#include "esp_task.h"

#include "nimble/nimble_npl.h"
#include "../../../../controller/esp32c6/esp_bt_cfg.h"
#include "hal/efuse_hal.h"

#ifdef CONFIG_BT_LE_HCI_INTERFACE_USE_UART
#include "driver/uart.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Bluetooth mode for controller enable/disable.
 */
typedef enum {
    ESP_BT_MODE_IDLE       = 0x00,   /*!< Bluetooth is not running */
    ESP_BT_MODE_BLE        = 0x01,   /*!< Run BLE mode */
    ESP_BT_MODE_CLASSIC_BT = 0x02,   /*!< Run Classic BT mode */
    ESP_BT_MODE_BTDM       = 0x03,   /*!< Run dual mode */
} esp_bt_mode_t;

/**
 * @brief Bluetooth controller enable/disable/initialised/de-initialised status.
 */
typedef enum {
    ESP_BT_CONTROLLER_STATUS_IDLE = 0,   /*!< Controller is in idle state */
    ESP_BT_CONTROLLER_STATUS_INITED,     /*!< Controller is in initialising state */
    ESP_BT_CONTROLLER_STATUS_ENABLED,    /*!< Controller is in enabled state */
    ESP_BT_CONTROLLER_STATUS_NUM,        /*!< Controller is in disabled state */
} esp_bt_controller_status_t;

/**
 * @brief BLE tx power type
 *        ESP_BLE_PWR_TYPE_CONN_HDL0-8: for each connection, and only be set after connection completed.
 *                                      when disconnect, the correspond TX power is not effected.
 *        ESP_BLE_PWR_TYPE_ADV : for advertising/scan response.
 *        ESP_BLE_PWR_TYPE_SCAN : for scan.
 *        ESP_BLE_PWR_TYPE_DEFAULT : if each connection's TX power is not set, it will use this default value.
 *                                   if neither in scan mode nor in adv mode, it will use this default value.
 *        If none of power type is set, system will use ESP_PWR_LVL_P3 as default for ADV/SCAN/CONN0-9.
 */
typedef enum {
    ESP_BLE_PWR_TYPE_CONN_HDL0  = 0,            /*!< For connection handle 0 */
    ESP_BLE_PWR_TYPE_CONN_HDL1  = 1,            /*!< For connection handle 1 */
    ESP_BLE_PWR_TYPE_CONN_HDL2  = 2,            /*!< For connection handle 2 */
    ESP_BLE_PWR_TYPE_CONN_HDL3  = 3,            /*!< For connection handle 3 */
    ESP_BLE_PWR_TYPE_CONN_HDL4  = 4,            /*!< For connection handle 4 */
    ESP_BLE_PWR_TYPE_CONN_HDL5  = 5,            /*!< For connection handle 5 */
    ESP_BLE_PWR_TYPE_CONN_HDL6  = 6,            /*!< For connection handle 6 */
    ESP_BLE_PWR_TYPE_CONN_HDL7  = 7,            /*!< For connection handle 7 */
    ESP_BLE_PWR_TYPE_CONN_HDL8  = 8,            /*!< For connection handle 8 */
    ESP_BLE_PWR_TYPE_ADV        = 9,            /*!< For advertising */
    ESP_BLE_PWR_TYPE_SCAN       = 10,           /*!< For scan */
    ESP_BLE_PWR_TYPE_DEFAULT    = 11,           /*!< For default, if not set other, it will use default value */
    ESP_BLE_PWR_TYPE_NUM        = 12,           /*!< TYPE numbers */
} esp_ble_power_type_t;

/**
 * @brief Bluetooth TX power level(index), it's just a index corresponding to power(dbm).
 *         Connection Tx power should only be get after connection created.
 * @param  power_type : The type of which tx power, could set Advertising/Connection/Default and etc
 * @return             >= 0 - Power level, < 0 - Invalid
 */
esp_power_level_t esp_ble_tx_power_get(esp_ble_power_type_t power_type);

/**
 * @brief  De-initialize BT controller to free resource and delete task.
 *         You should stop advertising and scanning, as well as
 *         disconnect all existing connections before de-initializing BT controller.
 *
 * This function should be called only once, after any other BT functions are called.
 * This function is not whole completed, esp_bt_controller_init cannot called after this function.
 * @return  ESP_OK - success, other - failed
 */
esp_err_t esp_bt_controller_deinit(void);

/**
 * @brief Enable BT controller.
 *               Due to a known issue, you cannot call esp_bt_controller_enable() a second time
 *               to change the controller mode dynamically. To change controller mode, call
 *               esp_bt_controller_disable() and then call esp_bt_controller_enable() with the new mode.
 * @param mode : the mode(BLE/BT/BTDM) to enable. For compatible of API, retain this argument.
 * @return       ESP_OK - success, other - failed
 */
esp_err_t esp_bt_controller_enable(esp_bt_mode_t mode);

/**
 * @brief  Disable BT controller
 * @return       ESP_OK - success, other - failed
 */
esp_err_t esp_bt_controller_disable(void);

/** @brief esp_vhci_host_callback
 *  used for vhci call host function to notify what host need to do
 */
typedef struct esp_vhci_host_callback {
    void (*notify_host_send_available)(void);               /*!< callback used to notify that the host can send packet to controller */
    int (*notify_host_recv)(uint8_t *data, uint16_t len);   /*!< callback used to notify that the controller has a packet to send to the host*/
} esp_vhci_host_callback_t;

/** @brief esp_vhci_host_check_send_available
 *  used for check actively if the host can send packet to controller or not.
 *  @return true for ready to send, false means cannot send packet
 */
bool esp_vhci_host_check_send_available(void);

/** @brief esp_vhci_host_send_packet
 * host send packet to controller
 *
 * Should not call this function from within a critical section
 * or when the scheduler is suspended.
 *
 * @param data the packet point
 * @param len the packet length
 */
void esp_vhci_host_send_packet(uint8_t *data, uint16_t len);

/** @brief esp_vhci_host_register_callback
 * register the vhci reference callback
 * struct defined by vhci_host_callback structure.
 * @param callback esp_vhci_host_callback type variable
 * @return ESP_OK - success, ESP_FAIL - failed
 */
esp_err_t esp_vhci_host_register_callback(const esp_vhci_host_callback_t *callback);

/** @brief esp_bt_controller_mem_release
 * release the controller memory as per the mode
 *
 * This function releases the BSS, data and other sections of the controller to heap. The total size is about 70k bytes.
 *
 * esp_bt_controller_mem_release(mode) should be called only before esp_bt_controller_init()
 * or after esp_bt_controller_deinit().
 *
 * Note that once BT controller memory is released, the process cannot be reversed. It means you cannot use the bluetooth
 * mode which you have released by this function.
 *
 * If your firmware will later upgrade the Bluetooth controller mode (BLE -> BT Classic or disabled -> enabled)
 * then do not call this function.
 *
 * If the app calls esp_bt_controller_enable(ESP_BT_MODE_BLE) to use BLE only then it is safe to call
 * esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT) at initialization time to free unused BT Classic memory.
 *
 * If the mode is ESP_BT_MODE_BTDM, then it may be useful to call API esp_bt_mem_release(ESP_BT_MODE_BTDM) instead,
 * which internally calls esp_bt_controller_mem_release(ESP_BT_MODE_BTDM) and additionally releases the BSS and data
 * consumed by the BT/BLE host stack to heap. For more details about usage please refer to the documentation of
 * esp_bt_mem_release() function
 *
 * @param mode : the mode want to release memory
 * @return ESP_OK - success, other - failed
 */
esp_err_t esp_bt_controller_mem_release(esp_bt_mode_t mode);

/** @brief esp_bt_mem_release
 * release controller memory and BSS and data section of the BT/BLE host stack as per the mode
 *
 * This function first releases controller memory by internally calling esp_bt_controller_mem_release().
 * Additionally, if the mode is set to ESP_BT_MODE_BTDM, it also releases the BSS and data consumed by the BT/BLE host stack to heap
 *
 * Note that once BT memory is released, the process cannot be reversed. It means you cannot use the bluetooth
 * mode which you have released by this function.
 *
 * If your firmware will later upgrade the Bluetooth controller mode (BLE -> BT Classic or disabled -> enabled)
 * then do not call this function.
 *
 * If you never intend to use bluetooth in a current boot-up cycle, you can call esp_bt_mem_release(ESP_BT_MODE_BTDM)
 * before esp_bt_controller_init or after esp_bt_controller_deinit.
 *
 * For example, if a user only uses bluetooth for setting the WiFi configuration, and does not use bluetooth in the rest of the product operation".
 * In such cases, after receiving the WiFi configuration, you can disable/deinit bluetooth and release its memory.
 * Below is the sequence of APIs to be called for such scenarios:
 *
 *      esp_bluedroid_disable();
 *      esp_bluedroid_deinit();
 *      esp_bt_controller_disable();
 *      esp_bt_controller_deinit();
 *      esp_bt_mem_release(ESP_BT_MODE_BTDM);
 *
 * @param mode : the mode whose memory is to be released
 * @return ESP_OK - success, other - failed
 */
esp_err_t esp_bt_mem_release(esp_bt_mode_t mode);

/**
 * @brief Returns random static address or -1 if not present.
 * @return ESP_OK - success, other - failed
 */
extern int esp_ble_hw_get_static_addr(esp_ble_addr_t *addr);

#if CONFIG_BT_LE_CONTROLLER_LOG_ENABLED
/**
 * @brief dump all log information cached in buffers.
 * @param output : true for log dump, false will take no effect
 */
void esp_ble_controller_log_dump_all(bool output);
#endif // CONFIG_BT_LE_CONTROLLER_LOG_ENABLED

#ifdef __cplusplus
}
#endif

#endif /* __ESP_BT_H__ */
