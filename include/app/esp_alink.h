#ifndef __ALINK_USER_CONFIG_H__
#define __ALINK_USER_CONFIG_H__
#include <stdio.h>
#include "alink_export.h"
#include "platform.h"
#include "assert.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "lwip/sockets.h"
#include "json_parser.h"

#include "alink_log.h"

/*!< description */
#ifndef CONFIG_WIFI_WAIT_TIME
#define CONFIG_WIFI_WAIT_TIME     60
#endif
#ifndef CONFIG_ALINK_RESET_KEY_IO
#define CONFIG_ALINK_RESET_KEY_IO 0
#endif

#ifndef CONFIG_ALINK_TASK_PRIOTY
#define CONFIG_ALINK_TASK_PRIOTY  6
#endif

#define WIFI_WAIT_TIME            (CONFIG_WIFI_WAIT_TIME * 1000)
#define ALINK_RESET_KEY_IO        CONFIG_ALINK_RESET_KEY_IO
#define DEFAULU_TASK_PRIOTY       CONFIG_ALINK_TASK_PRIOTY


#ifdef CONFIG_ALINK_PASSTHROUGH
#define ALINK_PASSTHROUGH
#endif

#define ALINK_CHIPID              "esp32"
#define MODULE_NAME               "ESP-WROOM-32"
#define ALINK_DATA_LEN            512

#define EVENT_HANDLER_CB_STACK    (4 * 1024)
typedef enum {
    ALINK_EVENT_CLOUD_CONNECTED = 0,/*!< ESP32 connected from alink cloude */
    ALINK_EVENT_CLOUD_DISCONNECTED, /*!< ESP32 disconnected from alink cloude */
    ALINK_EVENT_GET_DEVICE_DATA,    /*!< Alink cloud requests data from the device */
    ALINK_EVENT_SET_DEVICE_DATA,    /*!< Alink cloud to send data to the device */
    ALINK_EVENT_POST_CLOUD_DATA,    /*!< The device sends data to alink cloud  */
    ALINK_EVENT_STA_GOT_IP,         /*!< ESP32 station got IP from connected AP */
    ALINK_EVENT_STA_DISCONNECTED,   /*!< ESP32 station disconnected from AP */
    ALINK_EVENT_CONFIG_NETWORK,     /*!< The equipment enters the distribution mode */
    ALINK_EVENT_UPDATE_ROUTER,      /*!< Request to configure the router */
    ALINK_EVENT_FACTORY_RESET,      /*!< Request to restore factory settings */
    ALINK_EVENT_ACTIVATE_DEVICE,    /*!< Request activation device */
} alink_event_t;

typedef struct alink_product {
    const char *name;
    const char *model;
    const char *version;
    const char *key;
    const char *secret;
    const char *key_sandbox;
    const char *secret_sandbox;
} alink_product_t;

/**
 * @brief  Application specified event callback function
 *
 * @param  event event type defined in this file
 *
 * @note The memory space for the event callback function defaults to 4kBety
 *
 * @return
 *     - ALINK_OK : Succeed
 *     - others : fail
 */
typedef alink_err_t (*alink_event_cb_t)(alink_event_t event);

/**
 * @brief  Send the event to the event handler
 *
 * @param  event  Generated events
 *
 * @return
 *     - ALINK_OK : Succeed
 *     - ALINK_ERR :   Fail
 */
alink_err_t alink_event_send(alink_event_t event);

/**
 * @brief  Initialize alink config and start alink task
 *         Initialize event loop Create the event handler and task
 *
 * @param  product_info config provice alink init configuration
 *         event_handler_cb application specified event callback
 *
 * @return
 *     - ALINK_OK : Succeed
 *     - ALINK_ERR :   Fail
 */
alink_err_t alink_init(_IN_ const void *product_info,
                       _IN_ const alink_event_cb_t event_handler_cb);

/**
 * @brief  attempts to read up to count bytes from file descriptor fd into the
 *         buffer starting at buf.
 *
 * @param  up_cmd  Store the read data
 * @param  size  Write the size of the data
 * @param  micro_seconds  seconds before the function timeout, set to -1 if wait forever
 *
 * @return
 *     - ALINK_ERR : Error, errno is set appropriately
 *     - Others : Write the size of the data
 */
ssize_t alink_write(_IN_ const void *up_cmd, size_t size, int micro_seconds);

/**
 * @brief  attempts to read up to count bytes from file descriptor fd into the
 *         buffer starting at buf.
 *
 * @param  down_cmd  Store the read data
 * @param  size  Read the size of the data
 * @param  micro_seconds  seconds before the function timeout, set to -1 if wait forever
 *
 * @return
 *     - ALINK_ERR : Error, errno is set appropriately
 *     - Others : Read the size of the data
 */
ssize_t alink_read(_OUT_ void *down_cmd, size_t size, int  micro_seconds);

/**
 * @brief  Clear wifi information, restart the device into the config network mode
 *
 * @return
 *     - ALINK_OK : Succeed
 *     - ALINK_ERR :   Fail
 */
alink_err_t alink_update_router();

/**
 * @brief  Clear all the information of the device and return to the factory status
 *
 * @return
 *     - ALINK_OK : Succeed
 *     - ALINK_ERR :   Fail
 */
alink_err_t alink_factory_setting();

/**
 * @brief
 *
 * @param arg [description]
 */
void alink_key_trigger(void *arg);

#endif
