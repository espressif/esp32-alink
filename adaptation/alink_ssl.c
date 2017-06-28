/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2017 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on ESPRESSIF SYSTEMS ESP8266 only, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include <string.h>

#include "openssl/ssl.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"

#include "esp_alink.h"
#include "esp_alink_log.h"
#include "alink_platform.h"

static SSL_CTX *ctx = NULL;
static const char *TAG = "alink_ssl";
static void *alink_ssl_mutex = NULL;

void *platform_ssl_connect(_IN_ void *tcp_fd, _IN_ const char *server_cert, _IN_ int server_cert_len)
{
    ALINK_PARAM_CHECK(server_cert == NULL);

    if (platform_sys_net_is_ready() == ALINK_FALSE) {
        ALINK_LOGW("wifi disconnect");
        return NULL;
    }

    SSL *ssl = NULL;
    int socket = (int)tcp_fd;
    int ret = -1;

    if (alink_ssl_mutex == NULL) {
        alink_ssl_mutex = platform_mutex_init();
    }

    platform_mutex_lock(alink_ssl_mutex);

    ctx = SSL_CTX_new(TLSv1_1_client_method());

    if (!ctx) {
        ALINK_LOGE("SSL_CTX_new, ret: %p, free_heap :%u", ctx, esp_get_free_heap_size());
        goto err_exit;
    }

    ALINK_LOGD("set SSL context read buffer size");
    SSL_CTX_set_default_read_buffer_len(ctx, 2048);
    ssl = SSL_new(ctx);

    if (!ssl) {
        ALINK_LOGE("SSL_new, ret: %p, free_heap :%u", ssl, esp_get_free_heap_size());
        goto err_exit;
    }

    SSL_set_fd(ssl, socket);
    X509 *ca_cert = d2i_X509(NULL, (unsigned char *)server_cert, server_cert_len);

    if (ca_cert == NULL) {
        ALINK_LOGE("d2i_X509, ret: %p, free_heap :%u", ssl, esp_get_free_heap_size());
        goto err_exit;
    }

    ret = SSL_add_client_CA(ssl, ca_cert);

    if (ret != pdTRUE) {
        ALINK_LOGE("SSL_add_client_CA, ret:%d", ret);
        goto err_exit;
    }

    ALINK_ERROR_CHECK(ret != pdTRUE, NULL, "SSL_add_client_CA, ret:%d", ret);

    ret = SSL_connect(ssl);

    if (ret != pdTRUE) {
        ALINK_LOGE("SSL_connect, ret: %d", ret);
        goto err_exit;
    }

    platform_mutex_unlock(alink_ssl_mutex);
    return ssl;

err_exit:
    platform_mutex_unlock(alink_ssl_mutex);

    if (ctx) {
        SSL_CTX_free(ctx);
        ctx = NULL;
    }

    if (ssl) {
        platform_ssl_close(ssl);
    }

    return NULL;
}

int platform_ssl_send(_IN_ void *ssl, _IN_ const char *buffer, _IN_ int length)
{
    ALINK_PARAM_CHECK(length <= 0);
    ALINK_PARAM_CHECK(buffer == NULL);
    ALINK_ERROR_CHECK(ssl == NULL, ALINK_ERR, "Parameter error, ssl:%p", ssl);

    alink_err_t ret;

    if (alink_ssl_mutex == NULL) {
        alink_ssl_mutex = platform_mutex_init();
    }

    platform_mutex_lock(alink_ssl_mutex);
    ALINK_LOGV("SSL_write start");
    ret = SSL_write((SSL *)ssl, buffer, length);
    ALINK_LOGV("SSL_write end");
    platform_mutex_unlock(alink_ssl_mutex);
    ALINK_ERROR_CHECK(ret <= 0, ALINK_ERR, "SSL_write, ret:%d, errno:%d", ret, errno);
    return ret;
}

int platform_ssl_recv(_IN_ void *ssl, _OUT_ char *buffer, _IN_ int length)
{
    ALINK_PARAM_CHECK(ssl == NULL);
    ALINK_PARAM_CHECK(buffer == NULL);
    int ret = -1;

    if (alink_ssl_mutex == NULL) {
        alink_ssl_mutex = platform_mutex_init();
    }

    platform_mutex_lock(alink_ssl_mutex);
    ALINK_LOGV("SSL_read start");
    ret = SSL_read((SSL *)ssl, buffer, length);
    ALINK_LOGV("SSL_read end");
    platform_mutex_unlock(alink_ssl_mutex);

    if (ret <= 0) {
        perror("SSL_read");
    }

    ALINK_ERROR_CHECK(ret <= 0, ALINK_ERR, "SSL_read, ret:%d, errno:%d", ret, errno);
    return ret;
}

int platform_ssl_close(_IN_ void *ssl)
{
    ALINK_PARAM_CHECK(ssl == NULL);
    alink_err_t ret = -1;

    if (alink_ssl_mutex == NULL) {
        alink_ssl_mutex = platform_mutex_init();
    }

    platform_mutex_lock(alink_ssl_mutex);
    ret = SSL_shutdown((SSL *)ssl);

    if (ret != pdTRUE) {
        ALINK_LOGW("SSL_shutdown: ret:%d, ssl: %p", ret, ssl);
    }

    int fd = SSL_get_fd((SSL *)ssl);

    if (ssl) {
        SSL_free(ssl);
        ssl = NULL;
    }

    if (ctx) {
        SSL_CTX_free(ctx);
        ctx = NULL;
    }

    if (fd >= 0) {
        close(fd);
    } else {
        ALINK_LOGE("SSL_get_fd:%d", fd);
    }

    platform_mutex_unlock(alink_ssl_mutex);

    return (ret == pdTRUE) ? ALINK_OK : ALINK_ERR;
}
