#include "protocol.h"
static uint8_t range_status(uint16_t mm)
{
    return mm < 250 ? RANGE_STOP : mm < 600 ? RANGE_NEAR : RANGE_CLEAR;
}
Reading reading_from_pulse(uint16_t duration_us, uint8_t sequence)
{
    Reading result = { UINT16_MAX, RANGE_FAULT, sequence };
    if (duration_us >= 116 && duration_us <= 23324) {
        result.mm = (uint16_t)(((uint32_t)duration_us * 343u + 1000u) / 2000u);
        result.status = range_status(result.mm);
    }
    return result;
}
void reading_encode(Reading reading, uint8_t payload[CAN_DISTANCE_LENGTH])
{
    payload[0] = 1;
    payload[1] = reading.sequence;
    payload[2] = (uint8_t)reading.mm;
    payload[3] = (uint8_t)(reading.mm >> 8);
    payload[4] = reading.status;
}
bool reading_decode(const uint8_t *payload, size_t length, Reading *out)
{
    if (!payload || !out || length != CAN_DISTANCE_LENGTH || payload[0] != 1)
        return false;
    Reading reading = {
        (uint16_t)(payload[2] | ((uint16_t)payload[3] << 8)),
        payload[4], payload[1]
    };
    if (reading.status > RANGE_FAULT)
        return false;
    if (reading.status == RANGE_FAULT) {
        if (reading.mm != UINT16_MAX)
            return false;
    } else if (reading.mm < 20 || reading.mm > 4000 || reading.status != range_status(reading.mm)) {
        return false;
    }
    *out = reading;
    return true;
}
bool receive_fresh(ReceiveState *state, uint8_t sequence, uint32_t now_ms)
{
    uint8_t advance = (uint8_t)(sequence - state->last_sequence);
    if (state->seen && (uint32_t)(now_ms - state->last_ms) < RANGE_TIMEOUT_MS &&
        (advance == 0 || advance > 127))
        return false;
    state->seen = true;
    state->last_sequence = sequence;
    state->last_ms = now_ms;
    return true;
}
