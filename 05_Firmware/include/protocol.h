#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
enum { RANGE_CLEAR, RANGE_NEAR, RANGE_STOP, RANGE_FAULT };
enum { CAN_DISTANCE_ID=0x321, CAN_DISTANCE_LENGTH=5, RANGE_TIMEOUT_MS=500 };
typedef struct {
    uint16_t mm;
    uint8_t status;
    uint8_t sequence;
} Reading;
Reading reading_from_pulse(uint16_t duration_us, uint8_t sequence);
void reading_encode(Reading reading, uint8_t payload[CAN_DISTANCE_LENGTH]);
bool reading_decode(const uint8_t *payload, size_t length, Reading *out);
typedef struct {
    uint32_t last_ms;
    uint8_t last_sequence;
    bool seen;
} ReceiveState;
bool receive_fresh(ReceiveState *state, uint8_t sequence, uint32_t now_ms);
