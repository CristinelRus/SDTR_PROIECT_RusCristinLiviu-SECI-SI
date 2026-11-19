#pragma once
#include "stm32f1xx_hal.h"
#define SONAR_ECHO_PIN GPIO_PIN_0
#define SONAR_TRIG_PIN GPIO_PIN_1
#define LED_NEAR_PIN GPIO_PIN_0
#define LED_STOP_PIN GPIO_PIN_1
#define BUZZER_PIN GPIO_PIN_5
#define CAN_RX_PIN GPIO_PIN_8
#define CAN_TX_PIN GPIO_PIN_9
#define UART_TX_PIN GPIO_PIN_9
#define UART_RX_PIN GPIO_PIN_10
#define STATUS_LED_PIN GPIO_PIN_13

/* Leg PA1 la TRIG prin 74AHCT1G125 la 5 V; tin /OE la GND si decuplez cu 100 nF. */
/* Reduc ECHO spre PA0 cu 10 kOhm in serie si 18 kOhm de la PA0 la GND. */
/* Comand LED-urile din PB0/PB1 prin 330 Ohm; comand buzzerul din PB5 prin MOSFET logic. */
/* Leg SN65HVD230 la 3V3, PB9=TXD, PB8=RXD; leg RS la GND si pun cate 120 Ohm la capetele CAN. */
/* Folosesc buzzer activ de 5 V; pun 100 Ohm pe poarta MOSFET-ului si 100 kOhm spre GND. */
/* Citesc monitorul pe PA9/PA10, UART 115200, cu adaptor USB-UART de logica 3,3 V. */
/* Programez prin PA13=SWDIO, PA14=SWCLK, GND; pastrez BOOT0 si BOOT1 la zero. */
/* Alimentez fiecare placa dintr-o singura sursa; unesc masele si verific polaritatea headerelor. */
/* Verific fizic marcajul cipului, cristalul de 8 MHz si alimentarea de 3,3 V pe fiecare clona. */
