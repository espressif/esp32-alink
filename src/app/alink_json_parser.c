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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "esp_system.h"
#include "cJSON.h"
#include "alink_log.h"

#define TAG "alink_json_parser"
alink_err_t __alink_json_parse(const char *json_str, const char *key, void *value, int value_type)
{
    ALINK_PARAM_CHECK(!json_str);
    ALINK_PARAM_CHECK(!key);
    ALINK_PARAM_CHECK(!value);

    cJSON *pJson = cJSON_Parse(json_str);
    ALINK_ERROR_CHECK(!pJson, ALINK_ERR, "cJSON_Parse");

    cJSON *pSub = cJSON_GetObjectItem(pJson, key);
    if (!pSub) {
        cJSON_Delete(pJson);
        return -EINVAL;
    }
    // ALINK_ERROR_CHECK(!pSub, -EINVAL, "cJSON_GetObjectItem %s", key);

    char *p = NULL;

    switch (value_type) {
    case 1:
        *((int *)value) = pSub->valueint;
        break;
    case 2:
        *((float *)value) = (float)(pSub->valuedouble);
        break;
    case 3:
        *((double *)value) = pSub->valuedouble;
        break;

    default:
        switch (pSub->type) {
        case cJSON_False:
            *((char *)value) = cJSON_False;
            break;
        case cJSON_True:
            *((char *)value) = cJSON_True;
            break;
        case cJSON_Number:
            *((char *)value) = pSub->valueint;
            break;
        case cJSON_String:
            memcpy(value, pSub->valuestring, strlen(pSub->valuestring) + 1);
            break;
        case cJSON_Object:
            p = cJSON_Print(pSub);
            if (!p) {
                cJSON_Delete(pJson);
            }
            ALINK_ERROR_CHECK(!p, -ENOMEM, "cJSON_Print");
            memcpy(value, p, strlen(p) + 1);
            free(p);
            break;
        default:
            ALINK_LOGE("does not support this type(%d) of data parsing", pSub->type);
            break;
        }
    }

    cJSON_Delete(pJson);
    return ALINK_OK;
}

ssize_t alink_json_pack_double(char *json_str, const char *key, double value)
{
    ALINK_PARAM_CHECK(!json_str);
    ALINK_PARAM_CHECK(!key);

    int ret = 0;
    if (*json_str != '{') {
        *json_str = '{';
    } else {
        ret = (strlen(json_str) - 1);
        json_str += ret;
        ALINK_ERROR_CHECK(*(json_str) != '}', -EINVAL, "json_str Not initialized to empty");
        *json_str = ',';
    }
    json_str++;
    ret++;

    ret += sprintf(json_str, "\"%s\": %lf}", key, (double)value);
    return ret;
}

ssize_t __alink_json_pack(char *json_str, const char *key, int value, int value_type)
{
    ALINK_PARAM_CHECK(!json_str);
    ALINK_PARAM_CHECK(!key);

    char identifier = '{';
    if (*key == '[') {
        identifier = '[';
        key = NULL;
    }

    int ret = 0;
    if (*json_str != identifier) {
        *json_str = identifier;
    } else {
        ret = (strlen(json_str) - 1);
        json_str += ret;
        // ALINK_ERROR_CHECK(*(json_str) != '}', -EINVAL, "json_str Not initialized to empty");
        *json_str = ',';
    }
    json_str++;
    ret++;

    /*
     *  drom0_0_seg (R) : org = 0x3F400010, len = 0x800000
     *  "abc……" type can not be determined
     */
    if ((!value_type) && ((value & 0x3f000000) == 0x3f000000)) {
        value_type = 3;
    }
    int tmp = 0;
    if (key) {
        tmp = sprintf(json_str, "\"%s\": ", key);
        json_str += tmp;
        ret += tmp;
    }

    switch (value_type) {
    case 1:
        tmp = sprintf(json_str, "%d", (int)value);
        break;
    case 2:
        tmp = sprintf(json_str, "%lf", (double)value);
        break;
    case 3:
        if (*((char *)value) == '{' || *((char *)value) == '[') {
            tmp = sprintf(json_str, "%s", (char *)value);
        } else {
            tmp = sprintf(json_str, "\"%s\"", (char *)value);
        }
        break;

    default:
        ALINK_LOGE("invalid type: %d", value_type);
        ret = ALINK_ERR;
        return ret;
    }
    // printf("ret : %d, strlen: %d, json_str: %s\n", ret, (int)strlen(json_str), json_str - ret + 1);
    *(json_str + tmp) = '}';
    *(json_str + tmp + 1) = '\0';
    if (identifier == '[') {
        *(json_str + tmp) = ']';
    }
    // *(json_str + ret) = 0;
    ret += tmp;
    return ret;
}
