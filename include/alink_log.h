#ifndef __ALINK_LOG_H__
#define __ALINK_LOG_H__
#include "esp_log.h"
#include "errno.h"
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

#ifndef CONFIG_LOG_ALINK_LEVEL
#define CONFIG_LOG_ALINK_LEVEL CONFIG_LOG_DEFAULT_LEVEL
#endif
#undef LOG_LOCAL_LEVEL
#define LOG_LOCAL_LEVEL CONFIG_LOG_ALINK_LEVEL


#endif
