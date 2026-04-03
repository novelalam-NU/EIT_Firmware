#include "wireless.h"

#include <errno.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "lwip/inet.h"
#include "lwip/sockets.h"

#define WIFI_CONNECTED_BIT BIT0

static const char *TAG = "WIRELESS";
static EventGroupHandle_t wifi_event_group;

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    (void)arg;

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_LOGI(TAG, "Wi-Fi started, connecting...");
        esp_wifi_connect();
        return;
    }

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED) {
        ESP_LOGI(TAG, "Wi-Fi connected to AP");
        return;
    }

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        wifi_event_sta_disconnected_t *disc = (wifi_event_sta_disconnected_t *)event_data;
        xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
        ESP_LOGW(TAG, "Wi-Fi disconnected (reason=%u), retrying...", (unsigned)disc->reason);
        esp_wifi_connect();
        return;
    }

    if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
    }
}

bool wireless_wait_for_ip(uint32_t timeout_ms)
{
    if (wifi_event_group == NULL) {
        ESP_LOGE(TAG, "Wi-Fi event group not initialized");
        return false;
    }

    EventBits_t bits = xEventGroupWaitBits(
        wifi_event_group,
        WIFI_CONNECTED_BIT,
        pdFALSE,
        pdFALSE,
        pdMS_TO_TICKS(timeout_ms));

    return ((bits & WIFI_CONNECTED_BIT) != 0);
}

bool wireless_hardware_init(void)
{
    wifi_event_group = xEventGroupCreate();
    if (wifi_event_group == NULL) {
        ESP_LOGE(TAG, "Failed to create Wi-Fi event group");
        return false;
    }

    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        err = nvs_flash_erase();
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "nvs_flash_erase failed: %s", esp_err_to_name(err));
            return false;
        }
        err = nvs_flash_init();
    }
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_flash_init failed: %s", esp_err_to_name(err));
        return false;
    }

    err = esp_netif_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_netif_init failed: %s", esp_err_to_name(err));
        return false;
    }

    err = esp_event_loop_create_default();
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "esp_event_loop_create_default failed: %s", esp_err_to_name(err));
        return false;
    }

    esp_netif_t *wifi_obj_ptr = esp_netif_create_default_wifi_sta();
    if (wifi_obj_ptr == NULL) {
        ESP_LOGE(TAG, "Failed to esp_netif_create_default_wifi_sta");
        return false;
    }

    const wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    err = esp_wifi_init(&cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_init failed: %s", esp_err_to_name(err));
        return false;
    }

    err = esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "WIFI_EVENT register failed: %s", esp_err_to_name(err));
        return false;
    }

    err = esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "IP_EVENT register failed: %s", esp_err_to_name(err));
        return false;
    }

    wifi_config_t wifi_config_details = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASSWORD,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };

    err = esp_wifi_set_mode(WIFI_MODE_STA);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_set_mode failed: %s", esp_err_to_name(err));
        return false;
    }

    err = esp_wifi_set_config(WIFI_IF_STA, &wifi_config_details);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_set_config failed: %s", esp_err_to_name(err));
        return false;
    }

    err = esp_wifi_start();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_start failed: %s", esp_err_to_name(err));
        return false;
    }

    return true;
}


/* UDP broadcast destination (255.255.255.255); requires SO_BROADCAST on the socket. */
static struct sockaddr_in target = {
    .sin_family = AF_INET,
    .sin_port = htons(PORT_NUM),
    .sin_addr.s_addr = htonl(INADDR_BROADCAST),
};

/* Static fd to use */
static int fd;


int create_udp_socket(void)
{
    target.sin_addr.s_addr = htonl(INADDR_BROADCAST);

    fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (fd < 0) {
        ESP_LOGE(TAG, "socket() failed: errno=%d (%s)", errno, strerror(errno));
        return -1;
    }

    int broadcast = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) < 0) {
        ESP_LOGE(TAG, "SO_BROADCAST failed: errno=%d (%s)", errno, strerror(errno));
        close(fd);
        fd = -1;
        return -1;
    }

    return 0;
}


int send_udp_datagram(const uint8_t *buf, uint16_t buf_size)
{
    if (fd < 0) {
        ESP_LOGE(TAG, "send_udp_datagram: socket not created");
        return -1;
    }
    if (buf == NULL || buf_size == 0) {
        return -1;
    }

    int sent = sendto(fd, buf, buf_size, 0, (const struct sockaddr *)&target, sizeof(target));
    if (sent < 0) {
        ESP_LOGE(TAG, "sendto failed: errno=%d (%s)", errno, strerror(errno));
        return -1;
    }
    if ((unsigned)sent != (unsigned)buf_size) {
        ESP_LOGW(TAG, "partial send: %d/%u bytes", sent, (unsigned)buf_size);
        return -1;
    }

    return 0;
}

/* UDP task */
void UDP_task(void* arg) {
    for (;;) {

        /* Wait for measurement task to be done with a frame*/
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);


        /* send the ewma_amp buffer as a udp datagram */
        if (send_udp_datagram((const uint8_t *)ewma_amp, sizeof(ewma_amp)) != 0) {
            ESP_LOGE(TAG, "Failed to send UDP packet");
        }

    }
}


