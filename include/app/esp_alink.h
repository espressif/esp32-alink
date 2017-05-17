#ifndef __ALINK_USER_CONFIG_H__
#define __ALINK_USER_CONFIG_H__
#include "esp_log.h"
#include "alink_export.h"
#include "platform.h"
#include "assert.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "lwip/sockets.h"
#include "json_parser.h"
#include <stdio.h>

typedef int32_t alink_err_t;

#ifndef ALINK_TRUE
#define ALINK_TRUE  1
#endif
#ifndef ALINK_FALSE
#define ALINK_FALSE 0
#endif
#ifndef ALINK_OK
#define ALINK_OK    0
#endif
#ifndef ALINK_ERR
#define ALINK_ERR   -1
#endif

#ifndef _IN_
#define _IN_            /*!< indicate that this is a input parameter. */
#endif
#ifndef _OUT_
#define _OUT_           /*!< indicate that this is a output parameter. */
#endif
#ifndef _INOUT_
#define _INOUT_         /*!< indicate that this is a io parameter. */
#endif
#ifndef _IN_OPT_
#define _IN_OPT_        /*!< indicate that this is a optional input parameter. */
#endif
#ifndef _OUT_OPT_
#define _OUT_OPT_       /*!< indicate that this is a optional output parameter. */
#endif
#ifndef _INOUT_OPT_
#define _INOUT_OPT_     /*!< indicate that this is a optional io parameter. */
#endif

#define ALINK_LOGE( format, ... ) ESP_LOGE(TAG, "[%s, %d]:" format, __func__, __LINE__, ##__VA_ARGS__)
#define ALINK_LOGW( format, ... ) ESP_LOGW(TAG, "[%s, %d]:" format, __func__, __LINE__, ##__VA_ARGS__)
#define ALINK_LOGI( format, ... ) ESP_LOGI(TAG, format, ##__VA_ARGS__)
#define ALINK_LOGD( format, ... ) ESP_LOGD(TAG, "[%s, %d]:" format, __func__, __LINE__, ##__VA_ARGS__)
#define ALINK_LOGV( format, ... ) ESP_LOGV(TAG, format, ##__VA_ARGS__)

#define ALINK_ERROR_CHECK(con, err, format, ...) if(con) {ALINK_LOGE(format, ##__VA_ARGS__); perror(__func__); return err;}
#define ALINK_PARAM_CHECK(con) if(con) {ALINK_LOGE("Parameter error: %s", #con); assert(0 && #con);}

/*!< description */

#ifndef CONFIG_LOG_ALINK_LEVEL
#define CONFIG_LOG_ALINK_LEVEL CONFIG_LOG_DEFAULT_LEVEL
#endif

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

#undef LOG_LOCAL_LEVEL
#define LOG_LOCAL_LEVEL CONFIG_LOG_ALINK_LEVEL

#ifdef CONFIG_ALINK_PASSTHROUGH
#define ALINK_PASSTHROUGH
#endif

#define ALINK_CHIPID              "esp32"
#define MODULE_NAME               "ESP-WROOM-32"
#define ALINK_DATA_LEN            512

typedef enum {
    ALINK_EVENT_CLOUD_CONNECTED = 0,/*!< ESP32 connected from alink cloude */
    ALINK_EVENT_CLOUD_DISCONNECTED, /*!< ESP32 disconnected from alink cloude */
    ALINK_EVENT_GET_DEVICE_DATA,    /*!< Alink cloud requests data from the device */
    ALINK_EVENT_SET_DEVICE_DATA,    /*!< Alink cloud to send data to the device */
    ALINK_EVENT_POST_CLOUD_DATA,    /*!< The device sends data to alink cloud  */
    ALINK_EVENT_STA_GOT_IP,         /*!< ESP32 station got IP from connected AP */
    ALINK_EVENT_STA_DISCONNECTED,   /*!< ESP32 station disconnected from AP */
    ALINK_EVENT_CONFIG_NETWORK,     /*!< The equipment enters the distribution mode */
    ALINK_EVENT_FACTORY_RESET,     /*!< The equipment enters the distribution mode */
    ALINK_EVENT_ACTIVATE_DEVICE,     /*!< The equipment enters the distribution mode */
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
 * @return
 *     - ALINK_OK : Succeed
 *     - others : fail
 */
typedef alink_err_t (*alink_event_cb_t)(alink_event_t event);

/**
 * @brief  Initialize event loop
 *         Create the event handler and task
 *
 * @param  cb application specified event callback
 *
 * @return
 *     - ALINK_OK : Succeed
 *     - others : fail
 */
alink_err_t esp_alink_event_init(_IN_ alink_event_cb_t cb);

/**
 * @brief  Initialize alink config and start alink task
 *
 * @param  product_info config provice alink init configuration
 *
 * @return
 *     - ALINK_OK : Succeed
 *     - others : fail
 */
alink_err_t esp_alink_init(_IN_ const void *product_info);

/**
 * @brief  1
 *
 * @param  up_cmd        [description]
 * @param  size          [description]
 * @param  micro_seconds [description]
 *
 * @return               [description]
 */
ssize_t esp_alink_write(_IN_ const void *up_cmd, size_t size, int micro_seconds);

/**
 * @brief  1
 *
 * @param  down_cmd      [description]
 * @param  size          [description]
 * @param  micro_seconds [description]
 *
 * @return               [description]
 */
ssize_t esp_alink_read(_OUT_ void *down_cmd, size_t size, int  micro_seconds);

#endif
