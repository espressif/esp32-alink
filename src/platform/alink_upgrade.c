#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_partition.h"

#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "esp_alink.h"

#define BUFFSIZE 1024

static const char *TAG = "alink_upgrade";

/* update handle : set by esp_ota_begin(), must be freed via esp_ota_end() */
esp_ota_handle_t update_handle = 0 ;
const esp_partition_t *update_partition = NULL;
static int binary_file_length = 0;

void platform_flash_program_start(void)
{
    alink_err_t err;
    const esp_partition_t *configured = esp_ota_get_boot_partition();
    const esp_partition_t *running = esp_ota_get_running_partition();

    assert(configured == running); /* fresh from reset, should be running from configured boot partition */
    ALINK_LOGI("Running partition type %d subtype %d (offset 0x%08x)",
               configured->type, configured->subtype, configured->address);

    update_partition = esp_ota_get_next_update_partition(NULL);
    ALINK_LOGI("Writing to partition subtype %d at offset 0x%x",
               update_partition->subtype, update_partition->address);
    assert(update_partition != NULL);

    err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &update_handle);
    ALINK_ERROR_CHECK(err != ESP_OK, ; , "esp_ota_begin failed, error=%d", err);

    ALINK_LOGI("esp_ota_begin succeeded");
}

int platform_flash_program_write_block(_IN_ char *buffer, _IN_ uint32_t length)
{
    ALINK_PARAM_CHECK(length == 0);
    ALINK_PARAM_CHECK(buffer == NULL);

    alink_err_t err;
    err = esp_ota_write( update_handle, (const void *)buffer, length);
    ALINK_ERROR_CHECK(err != ESP_OK, ALINK_ERR, "Error: esp_ota_write failed! err=0x%x", err);

    binary_file_length += length;
    ALINK_LOGI("Have written image length %d", binary_file_length);
    return ALINK_OK;
}

int platform_flash_program_stop(void)
{
    alink_err_t err;
    ALINK_LOGI("Total Write binary data length : %d", binary_file_length);
    err = esp_ota_end(update_handle);
    ALINK_ERROR_CHECK(err != ESP_OK, ALINK_ERR, "esp_ota_end failed!");

    err = esp_ota_set_boot_partition(update_partition);
    ALINK_ERROR_CHECK(err != ESP_OK, ALINK_ERR, "esp_ota_set_boot_partition failed! err=0x%x", err);

    return ALINK_OK;
}
