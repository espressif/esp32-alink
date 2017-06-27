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

#include "string.h"
#include "stdlib.h"
#include "esp_info_store.h"
#include "nvs.h"
#include "nvs_flash.h"

#define TAG "alink_info_store"

int alink_info_erase(const char *key)
{
    ALINK_PARAM_CHECK(!key);

    alink_err_t ret   = -1;
    nvs_handle handle = 0;

    ret = nvs_open(ALINK_SPACE_NAME, NVS_READWRITE, &handle);
    ALINK_ERROR_CHECK(ret != ESP_OK, ALINK_ERR, "nvs_open ret:%x", ret);

    if (!strcmp(key, ALINK_SPACE_NAME)) {
        ret = nvs_erase_all(handle);
    } else {
        ret = nvs_erase_key(handle, key);
    }
    nvs_commit(handle);
    ALINK_ERROR_CHECK(ret != ESP_OK, ALINK_ERR, "nvs_erase_key ret:%x", ret);
    return ALINK_OK;
}

ssize_t alink_info_save(const char *key, const void *value, size_t length)
{
    ALINK_PARAM_CHECK(!key);
    ALINK_PARAM_CHECK(!value);
    ALINK_PARAM_CHECK(length <= 0);

    alink_err_t ret   = -1;
    nvs_handle handle = 0;

    ret = nvs_open(ALINK_SPACE_NAME, NVS_READWRITE, &handle);
    ALINK_ERROR_CHECK(ret != ESP_OK, ALINK_ERR, "nvs_open ret:%x", ret);

    /**
     * Reduce the number of flash writes
     */
    char *tmp = (char *)malloc(length);
    ret = nvs_get_blob(handle, key, tmp, &length);
    if ((ret == ESP_OK) && !memcmp(tmp, value, length)) {
        free(tmp);
        return length;
    }
    free(tmp);

    ret = nvs_set_blob(handle, key, value, length);
    nvs_commit(handle);
    nvs_close(handle);
    ALINK_ERROR_CHECK(ret != ESP_OK, ALINK_ERR, "nvs_set_blob ret:%x", ret);

    return length;
}

ssize_t alink_info_load(const char *key, void *value, size_t length)
{
    ALINK_PARAM_CHECK(!key);
    ALINK_PARAM_CHECK(!value);
    ALINK_PARAM_CHECK(length <= 0);

    alink_err_t ret   = -1;
    nvs_handle handle = 0;

    ret = nvs_open(ALINK_SPACE_NAME, NVS_READWRITE, &handle);
    ALINK_ERROR_CHECK(ret != ESP_OK, ALINK_ERR, "nvs_open ret:%x", ret);
    ret = nvs_get_blob(handle, key, value, &length);
    nvs_close(handle);
    if (ret == ESP_ERR_NVS_NOT_FOUND) {
        ALINK_LOGW("No data storage,the load data is empty");
        return ALINK_ERR;
    }
    ALINK_ERROR_CHECK(ret != ESP_OK, ALINK_ERR, "nvs_get_blob ret:%x", ret);
    return length;
}
