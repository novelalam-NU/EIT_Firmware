#ifndef WIRELESS_H
#define WIRELESS_H

#include <stdbool.h>
#include <stdint.h>
#include "calibration.h"

#define PORT_NUM (1423)
// #define IP_ADDR ("129.105.10.117")

/* Wi-Fi station credentials used by wireless_hardware_init(). */
#define WIFI_SSID ("NA iPhone")
#define WIFI_PASSWORD ("11111111")

/**
 * @brief Initialize the wireless subsystem in station mode.
 *
 * This sets up NVS, netif, event loop, Wi-Fi driver, event handlers,
 * station credentials, and starts Wi-Fi.
 *
 * @return true if initialization succeeds.
 * @return false if any setup step fails.
 */
bool wireless_hardware_init(void);

/**
 * @brief Wait for the station interface to obtain an IP address.
 *
 * @param timeout_ms Maximum wait time in milliseconds.
 * @return true if an IP is obtained before timeout.
 * @return false if timeout occurs or wait prerequisites are not met.
 */
bool wireless_wait_for_ip(uint32_t timeout_ms);

/**
 * @brief Create the UDP socket used for sending packets.
 *
 * @return Socket file descriptor on success, or -1 on failure.
 */
int create_udp_socket(void);

/**
 * @brief Send one UDP datagram using the active socket.
 *
 * @param buf Pointer to the payload buffer.
 * @param buf_len Number of bytes to send.
 * @return 0 on success, -1 on failure.
 */
int send_udp_datagram( const uint8_t* buf, uint16_t buf_size);

/**
 * @brief FreeRTOS task entry for periodic UDP transmission.
 *
 * This task function is intended to run in its own FreeRTOS task context
 * and repeatedly send UDP payloads using the wireless module socket helpers.
 *
 * @param arg Opaque user argument passed by xTaskCreate/xTaskCreatePinnedToCore.
 */
void UDP_task(void* arg);

#endif