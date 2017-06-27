// Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef __alink_INFO_STORE_H__
#define __alink_INFO_STORE_H__
#include <stdio.h>
#include "esp_alink.h"

#ifdef  __cplusplus
extern "C" {
#endif

#define ALINK_SPACE_NAME    "ALINK_APP"
#define NVS_KEY_WIFI_CONFIG "wifi_config"

/**
 * @brief  Erase the alink's information
 *
 * @param key   the corresponding key, it will find the target type by the key
 *
 * @return
 *     - alink_ERR : Error, errno is set appropriately
 *     - alink_OK  : Success
 */
int alink_info_erase(const char *key);

/**
 * @brief Save the alink's information
 *
 * @param  key    the corresponding key, it will find the target type by the key
 * @param  value  the value to be saved in the target information
 * @param  length the length of the value
 *
 *     - alink_ERR : Error, errno is set appropriately
 *     - others    : The length of the stored information
 */
ssize_t alink_info_save(const char *key, const void *value, size_t length);

/**
 * @brief  Load the alink's information
 *
 * @param  key    the corresponding key, it will find the target type by the key
 * @param  value  the value to be saved in the target information
 * @param  length the length of the value
 *
 *     - alink_ERR : Error, errno is set appropriately
 *     - others   : The length of the stored information
 */
ssize_t alink_info_load(const char *key, void *value, size_t length);

#ifdef __cplusplus
}
#endif

#endif /* __alink_INFO_STORE_H__ */
