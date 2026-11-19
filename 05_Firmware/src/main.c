#include "board.h"
#include "protocol.h"
#include "can_link.h"
#include "ultrasonic.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <stdio.h>
#include <string.h>

static QueueHandle_t telemetry;
#if NODE_SENSOR
static QueueHandle_t alerts;
static void sampling_task(void *argument)
{
    (void)argument;
    uint8_t sequence = 0;
    TickType_t next = xTaskGetTickCount();
    for (;;) {
        Reading reading = ultrasonic_measure(sequence++);
        xQueueOverwrite(alerts, &reading);
        xQueueOverwrite(telemetry, &reading);
        board_status(reading.status != RANGE_FAULT);
        xTaskDelayUntil(&next, pdMS_TO_TICKS(100));
    }
}
static void alerts_task(void *argument)
{
    (void)argument;
    Reading reading;
    for (;;) {
        if (xQueueReceive(alerts, &reading, pdMS_TO_TICKS(250)) == pdTRUE)
            board_alarm(reading.status);
        else
            board_alarm(RANGE_FAULT);
    }
}
static void transmit_task(void *argument)
{
    (void)argument;
    Reading reading;
    for (;;) {
        if (xQueueReceive(telemetry, &reading, portMAX_DELAY) == pdTRUE)
            can_link_send(reading);
    }
}
#else
static void receive_task(void *argument)
{
    (void)argument;
    ReceiveState state = {0};
    for (;;) {
        Reading reading;
        for (unsigned count = 0; count < 3 && can_link_receive(&reading); ++count) {
            if (receive_fresh(&state, reading.sequence, HAL_GetTick()))
                xQueueOverwrite(telemetry, &reading);
        }
        vTaskDelay(pdMS_TO_TICKS(2));
    }
}
static void monitor_task(void *argument)
{
    (void)argument;
    static const char *states[] = {"LIBER", "ATENTIE", "STOP", "SENZOR_INVALID"};
    Reading reading;
    char line[88];
    const char banner[] = "Monitor CAN / Blue Pill / 500 kbit/s\r\n";
    board_uart_write(banner, sizeof banner - 1);
    for (;;) {
        if (xQueueReceive(telemetry, &reading, pdMS_TO_TICKS(RANGE_TIMEOUT_MS)) == pdTRUE) {
            if (reading.status == RANGE_FAULT)
                snprintf(line, sizeof line, "seq=%u  distanta=--  %s\r\n", reading.sequence, states[reading.status]);
            else
                snprintf(line, sizeof line, "seq=%u  distanta=%u mm  %s\r\n", reading.sequence, reading.mm, states[reading.status]);
            board_status(true);
        } else {
            strcpy(line, "CAN: fara date noi de 500 ms\r\n");
            board_status(false);
        }
        board_uart_write(line, strlen(line));
    }
}
#endif
static void start_task(TaskFunction_t function, const char *name, UBaseType_t priority)
{
    if (xTaskCreate(function, name, 384, NULL, priority, NULL) != pdPASS)
        board_panic(FAULT_MEMORY);
}
int main(void)
{
    board_init();
    can_link_init();
    telemetry = xQueueCreate(1, sizeof(Reading));
    if (!telemetry)
        board_panic(FAULT_MEMORY);
#if NODE_SENSOR
    ultrasonic_init();
    alerts = xQueueCreate(1, sizeof(Reading));
    if (!alerts)
        board_panic(FAULT_MEMORY);
    start_task(sampling_task, "Distanta", 3);
    start_task(alerts_task, "Avertizare", 2);
    start_task(transmit_task, "CAN_TX", 2);
#else
    start_task(receive_task, "CAN_RX", 3);
    start_task(monitor_task, "Monitor", 2);
#endif
    vTaskStartScheduler();
    board_panic(FAULT_MEMORY);
}
