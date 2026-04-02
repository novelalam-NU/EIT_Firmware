#ifndef WIRELESS_H
#define WIRELESS_H

#include <stdbool.h>
#include <stdint.h>

bool wireless_hardware_init(void);
bool wireless_wait_for_ip(uint32_t timeout_ms);

#endif
