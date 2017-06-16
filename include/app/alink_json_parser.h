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

#ifndef __ALINK_JOSN_H__
#define __ALINK_JOSN_H__
#include "esp_system.h"
#include "alink_log.h"

#ifdef __cplusplus
extern "C" {
#endif /*!< _cplusplus */

/**
 * @brief This api is used only for alink json internal use
 */
alink_err_t __alink_json_parse(const char *json_str, const char *key, void *value, int value_type);
alink_err_t __alink_json_pack(char *json_str, const char *key, int value, int value_type);

/**
 * @brief  alink_err_t alink_json_parse(const char *json_str, const char *key, void *value)
 *         Parse the json formatted string
 *
 * @param  json_str The string pointer to be parsed
 * @param  key      Nuild value pairs
 * @param  value    You must ensure that the incoming type is consistent with the
 *                  post-resolution type
 *
 * @note   Does not support the analysis of array types
 *
 * @return
 *     - ALINK_OK  Success
 *     - ALINK_ERR Parameter error
 */
#define alink_json_parse(json_str, key, value) \
    __alink_json_parse((const char *)(json_str), (const char *)(key), value, \
        __builtin_types_compatible_p(typeof(value), short *) * 1\
        + __builtin_types_compatible_p(typeof(value), int *) * 1\
        + __builtin_types_compatible_p(typeof(value), uint16_t *) * 1\
        + __builtin_types_compatible_p(typeof(value), uint32_t *) * 1\
        + __builtin_types_compatible_p(typeof(value), float *) * 2\
        + __builtin_types_compatible_p(typeof(value), double *) * 3)

/**
 * @brief  alink_json_pack(char *json_str, const char *key, int/double/char value);
 *         Create a json string
 *
 * @param  json_str Save the generated json string
 * @param  key      Build value pairs
 * @param  value    This is a generic, support int/double/char/char *
 *
 * @note   If the value is double or float type only retains the integer part,
 *         requires complete data calling  alink_json_pack_double()
 *
 * @return
 *     - generates the length of the json string  Success
 *     - ALINK_ERR                                 Parameter error
 */
#define alink_json_pack(json_str, key, value) \
    __alink_json_pack((char *)(json_str), (const char *)(key), (int)(value), \
        __builtin_types_compatible_p(typeof(value), char) * 1\
        + __builtin_types_compatible_p(typeof(value), short) * 1\
        + __builtin_types_compatible_p(typeof(value), int) * 1\
        + __builtin_types_compatible_p(typeof(value), uint8_t) * 1\
        + __builtin_types_compatible_p(typeof(value), uint16_t) * 1\
        + __builtin_types_compatible_p(typeof(value), uint32_t) * 1\
        + __builtin_types_compatible_p(typeof(value), float) * 2\
        + __builtin_types_compatible_p(typeof(value), double) * 2\
        + __builtin_types_compatible_p(typeof(value), char *) * 3\
        + __builtin_types_compatible_p(typeof(value), const char *) * 3\
        + __builtin_types_compatible_p(typeof(value), unsigned char *) * 3\
        + __builtin_types_compatible_p(typeof(value), const unsigned char *) * 3)

/**
 * @brief  Create a double type json string, Make up for the lack of alink_json_pack()
 *
 * @param  json_str Save the generated json string
 * @param  key      Build value pairs
 * @param  value    The value to be stored
 *
 * @return
 *     - generates the length of the json string  Success
 *     - ALINK_ERR                                 Parameter error
 */
alink_err_t alink_json_pack_double(char *json_str, const char *key, double value);

#ifdef __cplusplus
}
#endif /*!< _cplusplus */

#endif /*!< __ALINK_JOSN_H__ */
