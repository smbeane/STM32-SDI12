/*
 ******************************************************************************
 * @file           : sdi12.h
 * @brief          : SDI-12 library for STM32 microcontrollers.
 *
 ******************************************************************************
 * @attention
 *
 * !! SDI-12 uses 5 V logic, ensure your microcontrollers pins are 5V
 * compatible !!
 *
 ******************************************************************************
 * @currently_supports
 *  - Acknowledge active (a!)
 *  - Send idenfification (aI!)
 *  - Change address (aAb!)
 *  - Start measurement (aM!)
 *  - Send data (aD0!)
 *  - Start verification (aV!)
 ******************************************************************************
 */

#ifndef SDI12_H
#define SDI12_H

#include "main.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define MAX_RESPONSE_SIZE 75

/*
 * GPIO Pin, Port and UART for SDI12 functions.
 */
typedef struct {
    UART_HandleTypeDef *Huart;
    uint32_t Pin;
    GPIO_TypeDef *Port;
} SDI12_TypeDef;

/*
 * SDI12 Identification for sensors
 */
typedef struct {
	char address;
	char vendor[9];
	char model[7];
} SDI12Identification;

/*
 * SDI12 Measurement response parse
 */
typedef struct {
    char Address;
    uint16_t Time;
    uint8_t NumValues;
} SDI12_Measure_TypeDef;

void SDI12_Init(UART_HandleTypeDef *huart);
HAL_StatusTypeDef SDI12_QueryDevice(const char cmd[], const uint8_t cmd_len, char *response, const uint8_t response_len);
HAL_StatusTypeDef SDI12_AckActive(const char addr);
void SDI12_DevicesOnBus(char *const devices);
HAL_StatusTypeDef SDI12_GetId(const char addr, char response[], uint8_t response_len);
HAL_StatusTypeDef SDI12_ChangeAddr(char *from_addr, char *to_addr);
HAL_StatusTypeDef SDI12_StartMeasurement(const char addr, SDI12_Measure_TypeDef *measure_info);
HAL_StatusTypeDef SDI12_SendData(const char addr, char *data, const size_t num_measurements, const char command);
void ParseSDI12Identification (const char* response, SDI12Identification* out);

#endif // SDI12_H
