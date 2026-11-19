#pragma once
#include "pins.h"
#include <stdbool.h>
#include <stddef.h>
enum { FAULT_CLOCK=1, FAULT_CAN=2, FAULT_MEMORY=3, FAULT_ASSERT=4, FAULT_STACK=5, FAULT_UART=6 };
void board_init(void);
void board_panic(uint32_t code);
void board_status(bool on);
void board_alarm(uint8_t status);
void board_uart_write(const char *text, size_t length);
extern volatile uint32_t board_fault;
