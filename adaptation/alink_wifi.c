#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "lwip/sockets.h"

#include "alink_platform.h"
#include "esp_alink.h"
#include "esp_alink_log.h"
#include "esp_info_store.h"

static platform_awss_recv_80211_frame_cb_t g_sniffer_cb = NULL;
static const char *TAG = "alink_wifi";
/**
 * @brief Get timeout interval, in millisecond, of per awss.
 *
 * @param None.
 * @return The timeout interval.
 * @see None.
 * @note The recommended value is 60,000ms.
 */
int platform_awss_get_timeout_interval_ms(void)
{
    return 3 * 60 * 1000;
}

/**
 * @brief Get timeout interval in millisecond to connect the default SSID if awss timeout happens.
 *
 * @param None.
 * @return The timeout interval.
 * @see None.
 * @note The recommended value is 0ms, which mean forever.
 */
int platform_awss_get_connect_default_ssid_timeout_interval_ms(void)
{
    return 0;
}

/**
 * @brief Get time length, in millisecond, of per channel scan.
 *
 * @param None.
 * @return The timeout interval.
 * @see None.
 * @note None. The recommended value is between 200ms and 400ms.
 */
int platform_awss_get_channelscan_interval_ms(void)
{
    return 300;
}

/**
 * @brief Switch to specific wifi channel.
 *
 * @param[in] primary_channel @n Primary channel.
 * @param[in] secondary_channel @n Auxiliary channel if 40Mhz channel is supported, currently
 *              this param is always 0.
 * @param[in] bssid @n A pointer to wifi BSSID on which awss lock the channel, most platform
 *              may ignore it.
 */
void platform_awss_switch_channel(char primary_channel,
                                  char secondary_channel, uint8_t bssid[ETH_ALEN])
{
    ESP_ERROR_CHECK(esp_wifi_set_channel(primary_channel, secondary_channel));
    //ret = system(buf);
}

static void IRAM_ATTR  wifi_sniffer_cb_(void *recv_buf, wifi_promiscuous_pkt_type_t type)
{
    char *buf = NULL;
    uint16_t len = 0;
    // if (type == WIFI_PKT_CTRL) return;
    wifi_promiscuous_pkt_t *sniffer = (wifi_promiscuous_pkt_t *)recv_buf;
    buf = (char *)sniffer->payload;
    len = sniffer->rx_ctrl.sig_len;
    g_sniffer_cb(buf, len, AWSS_LINK_TYPE_NONE, 1);
}

/**
 * @brief Set wifi running at monitor mode,
   and register a callback function which will be called when wifi receive a frame.
 *
 * @param[in] cb @n A function pointer, called back when wifi receive a frame.
 */
void platform_awss_open_monitor(_IN_ platform_awss_recv_80211_frame_cb_t cb)
{
    g_sniffer_cb = cb;
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_promiscuous(0));
    ESP_ERROR_CHECK(esp_wifi_set_promiscuous_rx_cb(wifi_sniffer_cb_));
    ESP_ERROR_CHECK(esp_wifi_set_promiscuous(1));
    ESP_ERROR_CHECK(esp_wifi_set_channel(6, 0));
}

/**
 * @brief Close wifi monitor mode, and set running at station mode.
 */
void platform_awss_close_monitor(void)
{
    ESP_ERROR_CHECK(esp_wifi_set_promiscuous(0));
    ESP_ERROR_CHECK(esp_wifi_set_promiscuous_rx_cb(NULL));
}


int platform_wifi_get_rssi_dbm(void)
{
    wifi_ap_record_t ap_infor;
    esp_wifi_sta_get_ap_info(&ap_infor);
    return ap_infor.rssi;
}

char *platform_wifi_get_mac(_OUT_ char mac_str[PLATFORM_MAC_LEN])
{
    ALINK_PARAM_CHECK(mac_str == NULL);
    uint8_t mac[6] = {0};
    ESP_ERROR_CHECK(esp_wifi_get_mac(ESP_IF_WIFI_STA, mac));
    snprintf(mac_str, PLATFORM_MAC_LEN, MACSTR, MAC2STR(mac));
    return mac_str;
}

uint32_t platform_wifi_get_ip(_OUT_ char ip_str[PLATFORM_IP_LEN])
{
    ALINK_PARAM_CHECK(ip_str == NULL);
    tcpip_adapter_ip_info_t infor;
    tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &infor);
    memcpy(ip_str, inet_ntoa(infor.ip.addr), PLATFORM_IP_LEN);
    return infor.ip.addr;
}

static alink_err_t sys_net_is_ready = ALINK_FALSE;
int platform_sys_net_is_ready(void)
{
    return sys_net_is_ready;
}

alink_err_t alink_event_send(alink_event_t event);
static SemaphoreHandle_t xSemConnet = NULL;
static alink_err_t event_handler(void *ctx, system_event_t *event)
{
    switch (event->event_id) {
        case SYSTEM_EVENT_STA_START:
            ALINK_LOGI("SYSTEM_EVENT_STA_START");
            ESP_ERROR_CHECK(esp_wifi_connect());
            break;

        case SYSTEM_EVENT_STA_GOT_IP:
            sys_net_is_ready = ALINK_TRUE;
            ALINK_LOGI("SYSTEM_EVENT_STA_GOT_IP");
            xSemaphoreGive(xSemConnet);
            alink_event_send(ALINK_EVENT_STA_GOT_IP);
            break;

        case SYSTEM_EVENT_STA_DISCONNECTED:
            ALINK_LOGI("SYSTEM_EVENT_STA_DISCONNECTED, free_heap: %d", esp_get_free_heap_size());
            sys_net_is_ready = ALINK_FALSE;
            alink_event_send(ALINK_EVENT_STA_DISCONNECTED);
            int ret = esp_wifi_connect();

            if (ret != ESP_OK) {
                ALINK_LOGE("esp_wifi_connect, ret: %d", ret);
            }

            break;

        default:
            break;
    }

    return ALINK_OK;
}

int platform_awss_connect_ap(
    _IN_ uint32_t connection_timeout_ms,
    _IN_ char ssid[PLATFORM_MAX_SSID_LEN],
    _IN_ char passwd[PLATFORM_MAX_PASSWD_LEN],
    _IN_OPT_ enum AWSS_AUTH_TYPE auth,
    _IN_OPT_ enum AWSS_ENC_TYPE encry,
    _IN_OPT_ uint8_t bssid[ETH_ALEN],
    _IN_OPT_ uint8_t channel)
{
    wifi_config_t wifi_config;

    if (xSemConnet == NULL) {
        xSemConnet = xSemaphoreCreateBinary();
        esp_event_loop_set_cb(event_handler, NULL);
    }

    ESP_ERROR_CHECK(esp_wifi_stop());
    ESP_ERROR_CHECK(esp_wifi_get_config(WIFI_IF_STA, &wifi_config));
    memcpy(wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
    memcpy(wifi_config.sta.password, passwd, sizeof(wifi_config.sta.password));
    ALINK_LOGI("ap ssid: %s, password: %s", wifi_config.sta.ssid, wifi_config.sta.password);
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    BaseType_t err = xSemaphoreTake(xSemConnet, connection_timeout_ms / portTICK_RATE_MS);

    if (err != pdTRUE) {
        ESP_ERROR_CHECK(esp_wifi_stop());
    }

    ALINK_ERROR_CHECK(err != pdTRUE, ALINK_ERR, "xSemaphoreTake ret:%x wait: %d", err, connection_timeout_ms);

    if (!strcmp(ssid, "aha")) {
        return ALINK_OK;
    }

    err = esp_info_save(NVS_KEY_WIFI_CONFIG, &wifi_config, sizeof(wifi_config_t));

    if (err < 0) {
        ALINK_LOGE("alink information save failed");
    }

    return ALINK_OK;
}

/**
 * @brief send 80211 raw frame in current channel with basic rate(1Mbps)
 *
 * @param[in] type @n see enum platform_awss_frame_type, currently only FRAME_BEACON
 *                      FRAME_PROBE_REQ is used
 * @param[in] buffer @n 80211 raw frame, include complete mac header & FCS field
 * @param[in] len @n 80211 raw frame length
 * @return
   @verbatim
   =  0, send success.
   = -1, send failure.
   = -2, unsupported.
   @endverbatim
 * @see None.
 * @note awss use this API send raw frame in wifi monitor mode & station mode
 */
extern esp_err_t esp_wifi_80211_tx(wifi_interface_t ifx, const void *buffer, int len);
int platform_wifi_send_80211_raw_frame(_IN_ enum platform_awss_frame_type type,
                                       _IN_ uint8_t *buffer, _IN_ int len)
{
    ALINK_PARAM_CHECK(!buffer);

    int ret = esp_wifi_80211_tx(ESP_IF_WIFI_STA, buffer, len);
    ALINK_ERROR_CHECK(ret != ALINK_OK, ALINK_ERR, "esp_wifi_80211_tx, ret: 0x%x", ret);

    return ALINK_OK;
}

platform_wifi_mgnt_frame_cb_t g_callback = NULL;
static uint8_t g_vendor_oui[3];
static void ssc_vnd_filter_cb(void *ctx, wifi_vendor_ie_type_t type,
                              const uint8_t sa[6], const uint8_t *vnd_ie, int rssi)
{
    ALINK_PARAM_CHECK(!vnd_ie);

    if (vnd_ie[2] == g_vendor_oui[0] && vnd_ie[3] == g_vendor_oui[1] && vnd_ie[4] == g_vendor_oui[2]) {
        g_callback((uint8_t *)vnd_ie, (vnd_ie[1] + 2), rssi, 1);
    }
}

/**
 * @brief enable/disable filter specific management frame in wifi station mode
 *
 * @param[in] filter_mask @n see mask macro in enum platform_awss_frame_type,
 *                      currently only FRAME_PROBE_REQ_MASK & FRAME_BEACON_MASK is used
 * @param[in] vendor_oui @n oui can be used for precise frame match, optional
 * @param[in] callback @n see platform_wifi_mgnt_frame_cb_t, passing 80211
 *                      frame or ie to callback. when callback is NULL
 *                      disable sniffer feature, otherwise enable it.
 * @return
   @verbatim
   =  0, success
   = -1, fail
   = -2, unsupported.
   @endverbatim
 * @see None.
 * @note awss use this API to filter specific mgnt frame in wifi station mode
 */
int platform_wifi_enable_mgnt_frame_filter(_IN_ uint32_t filter_mask,
        _IN_OPT_ uint8_t vendor_oui[3], _IN_
        platform_wifi_mgnt_frame_cb_t callback)
{

    alink_err_t ret = 0;
    ALINK_ERROR_CHECK(filter_mask != (FRAME_PROBE_REQ_MASK | FRAME_BEACON_MASK),
                      -2, "frame is no support, frame: 0x%x", filter_mask);
    g_callback = callback;
    memcpy(g_vendor_oui, vendor_oui, sizeof(g_vendor_oui));

    ret = esp_wifi_set_vendor_ie_cb(ssc_vnd_filter_cb, NULL);
    ALINK_ERROR_CHECK(ret != ALINK_OK, ALINK_ERR, "esp_wifi_set_vendor_ie_cb, ret: %d", ret);

    return ALINK_OK;
}

/**
 * @brief launch a wifi scan operation
 *
 * @param[in] cb @n pass ssid info(scan result) to this callback one by one
 * @return 0 for wifi scan is done, otherwise return -1
 * @see None.
 * @note
 *      This API should NOT exit before the invoking for cb is finished.
 *      This rule is something like the following :
 *      platform_wifi_scan() is invoked...
 *      ...
 *      for (ap = first_ap; ap <= last_ap; ap = next_ap){
 *        cb(ap)
 *      }
 *      ...
 *      platform_wifi_scan() exit...
 */
int platform_wifi_scan(platform_wifi_scan_result_cb_t cb)
{
    uint16_t wifi_ap_num = 0;
    wifi_ap_record_t *ap_info = NULL;

    wifi_scan_config_t scan_config;
    memset(&scan_config, 0, sizeof(scan_config));
    ESP_ERROR_CHECK(esp_wifi_scan_start(&scan_config, true));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&wifi_ap_num));

    ALINK_LOGI("ap number: %d", wifi_ap_num);
    ap_info = (wifi_ap_record_t *)malloc(sizeof(wifi_ap_record_t) * wifi_ap_num);
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&wifi_ap_num, ap_info));

    for (int i = 0; i < wifi_ap_num; ++i) {
        cb((char *)ap_info[i].ssid, (uint8_t *)ap_info[i].bssid, ap_info[i].authmode, AWSS_ENC_TYPE_INVALID,
           ap_info[i].primary, ap_info[i].rssi, 1);
    }

    ESP_ERROR_CHECK(esp_wifi_scan_stop());
    free(ap_info);
    return ALINK_OK;
}

#define AES_BLOCK_SIZE 16
/**
 * @brief initialize AES struct.
 *
 * @param[in] key:
 * @param[in] iv:
 * @param[in] dir: AES_ENCRYPTION or AES_DECRYPTION
 * @return AES128_t
 */
#include "mbedtls/aes.h"
typedef struct {
    mbedtls_aes_context ctx;
    uint8_t iv[16];
} platform_aes_t;

p_aes128_t platform_aes128_init(
    _IN_ const uint8_t *key,
    _IN_ const uint8_t *iv,
    _IN_ AES_DIR_t dir)
{
    ALINK_PARAM_CHECK(!key);
    ALINK_PARAM_CHECK(!iv);

    alink_err_t ret = 0;
    platform_aes_t *p_aes128 = NULL;
    p_aes128 = (platform_aes_t *)calloc(1, sizeof(platform_aes_t));
    ALINK_ERROR_CHECK(!p_aes128, NULL, "calloc");

    mbedtls_aes_init(&p_aes128->ctx);

    if (dir == PLATFORM_AES_ENCRYPTION) {
        ret = mbedtls_aes_setkey_enc(&p_aes128->ctx, key, 128);
    } else {
        ret = mbedtls_aes_setkey_dec(&p_aes128->ctx, key, 128);
    }

    if (ret != ALINK_OK) {
        free(p_aes128);
    }

    ALINK_ERROR_CHECK(ret != ALINK_OK, NULL, "mbedtls_aes_setkey_enc");

    memcpy(p_aes128->iv, iv, 16);
    return (p_aes128_t *)p_aes128;
}

/**
 * @brief release AES struct.
 *
 * @param[in] aes:
 * @return
   @verbatim
     = 0: succeeded
     = -1: failed
   @endverbatim
 * @see None.
 * @note None.
 */
int platform_aes128_destroy(_IN_ p_aes128_t aes)
{
    ALINK_PARAM_CHECK(!aes);
    mbedtls_aes_free(&((platform_aes_t *)aes)->ctx);
    free(aes);
    return ALINK_OK;
}

/**
 * @brief encrypt data with aes (cbc/128bit key).
 *
 * @param[in] aes: AES handler
 * @param[in] src: plain data
 * @param[in] blockNum: plain data number of 16 bytes size
 * @param[out] dst: cipher data
 * @return
   @verbatim
     = 0: succeeded
     = -1: failed
   @endverbatim
 * @see None.
 * @note None.
 */
int platform_aes128_cbc_encrypt(
    _IN_ p_aes128_t aes,
    _IN_ const void *src,
    _IN_ size_t blockNum,
    _OUT_ void *dst)
{
    ALINK_PARAM_CHECK(!aes);
    ALINK_PARAM_CHECK(!src);
    ALINK_PARAM_CHECK(!dst);

    alink_err_t ret = 0;
    platform_aes_t *p_aes128 = (platform_aes_t *)aes;

    for (int i = 0; i < blockNum; ++i) {
        ret = mbedtls_aes_crypt_cbc(&p_aes128->ctx, MBEDTLS_AES_ENCRYPT, AES_BLOCK_SIZE,
                                    p_aes128->iv, src, dst);
        src += 16;
        dst += 16;
    }

    ALINK_ERROR_CHECK(ret != ALINK_OK, ALINK_ERR, "mbedtls_aes_crypt_cbc, ret: %d", ret);
    return ALINK_OK;
}

/**
 * @brief decrypt data with aes (cbc/128bit key).
 *
 * @param[in] aes: AES handler
 * @param[in] src: cipher data
 * @param[in] blockNum: plain data number of 16 bytes size
 * @param[out] dst: plain data
 * @return
   @verbatim
     = 0: succeeded
     = -1: failed
   @endverbatim
 * @see None.
 * @note None.
 */
int platform_aes128_cbc_decrypt(
    _IN_ p_aes128_t aes,
    _IN_ const void *src,
    _IN_ size_t blockNum,
    _OUT_ void *dst)
{
    ALINK_PARAM_CHECK(!aes);
    ALINK_PARAM_CHECK(!src);
    ALINK_PARAM_CHECK(!dst);

    alink_err_t ret = 0;
    platform_aes_t *p_aes128 = (platform_aes_t *)aes;

    for (int i = 0; i < blockNum; ++i) {
        ret = mbedtls_aes_crypt_cbc(&p_aes128->ctx, MBEDTLS_AES_DECRYPT, AES_BLOCK_SIZE,
                                    p_aes128->iv, src, dst);
        src += 16;
        dst += 16;
    }

    ALINK_ERROR_CHECK(ret != ALINK_OK, ALINK_ERR,
                      "mbedtls_aes_crypt_cbc, ret: %d, blockNum: %d", ret, blockNum);
    return ALINK_OK;
}

/**
 * @brief get the information of the connected AP.
 *
 * @param[out] ssid: array to store ap ssid. It will be null if ssid is not required.
 * @param[out] passwd: array to store ap password. It will be null if ap password is not required.
 * @param[out] bssid: array to store ap bssid. It will be null if bssid is not required.
 * @return
   @verbatim
     = 0: succeeded
     = -1: failed
   @endverbatim
 * @see None.
 * @note None.
 */

int platform_wifi_get_ap_info(
    _OUT_ char ssid[PLATFORM_MAX_SSID_LEN],
    _OUT_ char passwd[PLATFORM_MAX_PASSWD_LEN],
    _OUT_ uint8_t bssid[ETH_ALEN])
{
    alink_err_t ret = 0;
    wifi_ap_record_t ap_info;
    memset(&ap_info, 0, sizeof(wifi_ap_record_t));
    ESP_ERROR_CHECK(esp_wifi_sta_get_ap_info(&ap_info));

    if (ssid) {
        memcpy(ssid, ap_info.ssid, PLATFORM_MAX_SSID_LEN);
    }

    if (bssid) {
        memcpy(bssid, ap_info.bssid, ETH_ALEN);
    }

    if (!passwd) {
        return ALINK_OK;
    }

    wifi_config_t wifi_config;
    ret = esp_info_load(NVS_KEY_WIFI_CONFIG, &wifi_config, sizeof(wifi_config_t));

    if (ret > 0 && !memcmp(ap_info.ssid, wifi_config.sta.ssid, strlen((char *)ap_info.ssid))) {
        memcpy(passwd, wifi_config.sta.password, PLATFORM_MAX_PASSWD_LEN);
        ALINK_LOGV("wifi passwd: %s", passwd);
    } else {
        memset(passwd, 0, PLATFORM_MAX_PASSWD_LEN);
    }

    return ALINK_OK;
}
