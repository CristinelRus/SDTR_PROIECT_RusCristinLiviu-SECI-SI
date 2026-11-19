#include "board.h"
#include "can_link.h"
static CAN_HandleTypeDef handle;
CanStats can_stats;
void can_link_init(void)
{
    __HAL_RCC_CAN1_CLK_ENABLE();
    __HAL_AFIO_REMAP_CAN1_2();
    GPIO_InitTypeDef pin = {0};
    pin.Pin = CAN_TX_PIN; pin.Mode = GPIO_MODE_AF_PP; pin.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &pin);
    pin.Pin = CAN_RX_PIN; pin.Mode = GPIO_MODE_INPUT; pin.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOB, &pin);
    handle.Instance = CAN1;
    handle.Init.Prescaler = 4;
    handle.Init.Mode = CAN_MODE_NORMAL;
    handle.Init.SyncJumpWidth = CAN_SJW_1TQ;
    handle.Init.TimeSeg1 = CAN_BS1_15TQ;
    handle.Init.TimeSeg2 = CAN_BS2_2TQ;
    handle.Init.AutoBusOff = ENABLE;
    /* Pastrez doar citirea curenta daca magistrala nu poate confirma transmisia. */
    handle.Init.AutoRetransmission = DISABLE;
    handle.Init.ReceiveFifoLocked = DISABLE;
    handle.Init.TransmitFifoPriority = ENABLE;
    if (HAL_CAN_Init(&handle) != HAL_OK)
        board_panic(FAULT_CAN);
    CAN_FilterTypeDef filter = {0};
    filter.FilterMode = CAN_FILTERMODE_IDMASK;
    filter.FilterScale = CAN_FILTERSCALE_32BIT;
    filter.FilterIdHigh = CAN_DISTANCE_ID << 5;
    filter.FilterMaskIdHigh = 0x7ffu << 5;
    filter.FilterMaskIdLow = 6;
    filter.FilterFIFOAssignment = CAN_FILTER_FIFO0;
    filter.FilterActivation = ENABLE;
    filter.SlaveStartFilterBank = 14;
    if (HAL_CAN_ConfigFilter(&handle, &filter) != HAL_OK || HAL_CAN_Start(&handle) != HAL_OK)
        board_panic(FAULT_CAN);
}
bool can_link_send(Reading reading)
{
    static bool was_bus_off;
    bool bus_off = (CAN1->ESR & CAN_ESR_BOFF) != 0;
    if (bus_off && !was_bus_off) ++can_stats.bus_off_events;
    was_bus_off = bus_off;
    if (bus_off || HAL_CAN_GetTxMailboxesFreeLevel(&handle) == 0) {
        ++can_stats.dropped;
        return false;
    }
    CAN_TxHeaderTypeDef header = {0};
    header.StdId = CAN_DISTANCE_ID;
    header.IDE = CAN_ID_STD;
    header.RTR = CAN_RTR_DATA;
    header.DLC = CAN_DISTANCE_LENGTH;
    uint8_t payload[8] = {0};
    uint32_t mailbox;
    reading_encode(reading, payload);
    if (HAL_CAN_AddTxMessage(&handle, &header, payload, &mailbox) != HAL_OK) {
        ++can_stats.dropped;
        return false;
    }
    ++can_stats.queued;
    return true;
}
bool can_link_receive(Reading *reading)
{
    if (HAL_CAN_GetRxFifoFillLevel(&handle, CAN_RX_FIFO0) == 0)
        return false;
    CAN_RxHeaderTypeDef header;
    uint8_t payload[8];
    if (HAL_CAN_GetRxMessage(&handle, CAN_RX_FIFO0, &header, payload) != HAL_OK)
        return false;
    if (header.StdId != CAN_DISTANCE_ID || header.IDE != CAN_ID_STD ||
        header.RTR != CAN_RTR_DATA || !reading_decode(payload, header.DLC, reading)) {
        ++can_stats.invalid;
        return false;
    }
    ++can_stats.received;
    return true;
}
