#include "board.h"
#include "protocol.h"
#include "FreeRTOS.h"
#include "task.h"

volatile uint32_t board_fault;
#if !NODE_SENSOR
static UART_HandleTypeDef uart;
#endif

static void configure(GPIO_TypeDef *port, uint32_t pins, uint32_t mode, uint32_t pull)
{
    GPIO_InitTypeDef config = {0};
    config.Pin = pins;
    config.Mode = mode;
    config.Pull = pull;
    config.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(port, &config);
}
void board_status(bool on)
{
    HAL_GPIO_WritePin(GPIOC, STATUS_LED_PIN, on ? GPIO_PIN_RESET : GPIO_PIN_SET);
}
void board_panic(uint32_t code)
{
    __disable_irq();
    board_fault = code;
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    configure(GPIOB, LED_NEAR_PIN | LED_STOP_PIN | BUZZER_PIN, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL);
    GPIOB->BRR = BUZZER_PIN;
    GPIOB->BSRR = LED_NEAR_PIN | LED_STOP_PIN;
    configure(GPIOC, STATUS_LED_PIN, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL);
    for (;;) {
        for (uint32_t i = 0; i < code; ++i) {
            board_status(true);
            for (volatile uint32_t t = 0; t < 180000; ++t) __NOP();
            board_status(false);
            for (volatile uint32_t t = 0; t < 180000; ++t) __NOP();
        }
        for (volatile uint32_t t = 0; t < 1600000; ++t) __NOP();
    }
}
static void configure_clock(void)
{
    RCC_OscInitTypeDef osc = {0};
    osc.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    osc.HSEState = RCC_HSE_ON;
    osc.PLL.PLLState = RCC_PLL_ON;
    osc.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    osc.PLL.PLLMUL = RCC_PLL_MUL9;
    osc.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    /* Cer cristalul de 8 MHz pentru a mentine temporizarea CAN intre cele doua placi. */
    if (HAL_RCC_OscConfig(&osc) != HAL_OK)
        board_panic(FAULT_CLOCK);
    RCC_ClkInitTypeDef clk = {0};
    clk.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    clk.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    clk.AHBCLKDivider = RCC_SYSCLK_DIV1;
    clk.APB1CLKDivider = RCC_HCLK_DIV2;
    clk.APB2CLKDivider = RCC_HCLK_DIV1;
    if (HAL_RCC_ClockConfig(&clk, FLASH_LATENCY_2) != HAL_OK)
        board_panic(FAULT_CLOCK);
    if (HAL_RCC_GetPCLK1Freq() != 36000000u || SystemCoreClock != 72000000u)
        board_panic(FAULT_CLOCK);
}
void board_init(void)
{
    HAL_Init();
    configure_clock();
    __HAL_RCC_AFIO_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
    GPIOC->BSRR = STATUS_LED_PIN;
    configure(GPIOC, STATUS_LED_PIN, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL);
#if NODE_SENSOR
    GPIOB->BRR = LED_NEAR_PIN | LED_STOP_PIN | BUZZER_PIN;
    configure(GPIOB, LED_NEAR_PIN | LED_STOP_PIN | BUZZER_PIN, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL);
    GPIOA->BRR = SONAR_TRIG_PIN;
    configure(GPIOA, SONAR_TRIG_PIN, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL);
    configure(GPIOA, SONAR_ECHO_PIN, GPIO_MODE_INPUT, GPIO_PULLDOWN);
#else
    __HAL_RCC_USART1_CLK_ENABLE();
    configure(GPIOA, UART_TX_PIN, GPIO_MODE_AF_PP, GPIO_NOPULL);
    configure(GPIOA, UART_RX_PIN, GPIO_MODE_INPUT, GPIO_PULLUP);
    uart.Instance = USART1;
    uart.Init.BaudRate = 115200;
    uart.Init.WordLength = UART_WORDLENGTH_8B;
    uart.Init.StopBits = UART_STOPBITS_1;
    uart.Init.Parity = UART_PARITY_NONE;
    uart.Init.Mode = UART_MODE_TX_RX;
    uart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    uart.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&uart) != HAL_OK)
        board_panic(FAULT_UART);
#endif
}
void board_alarm(uint8_t status)
{
#if NODE_SENSOR
    HAL_GPIO_WritePin(GPIOB, LED_NEAR_PIN, status == RANGE_NEAR || status == RANGE_FAULT ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, LED_STOP_PIN, status == RANGE_STOP || status == RANGE_FAULT ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, BUZZER_PIN, status == RANGE_STOP ? GPIO_PIN_SET : GPIO_PIN_RESET);
#else
    (void)status;
#endif
}
void board_uart_write(const char *text, size_t length)
{
#if !NODE_SENSOR
    if (HAL_UART_Transmit(&uart, (uint8_t *)text, (uint16_t)length, 30) != HAL_OK)
        board_panic(FAULT_UART);
#else
    (void)text;
    (void)length;
#endif
}
void xPortSysTickHandler(void);
void SysTick_Handler(void)
{
    HAL_IncTick();
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
        xPortSysTickHandler();
}
void vApplicationMallocFailedHook(void) { board_panic(FAULT_MEMORY); }
void vApplicationStackOverflowHook(TaskHandle_t task, char *name)
{
    (void)task;
    (void)name;
    board_panic(FAULT_STACK);
}
void HardFault_Handler(void) { board_panic(FAULT_ASSERT); }
