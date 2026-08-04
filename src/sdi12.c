/*
 ******************************************************************************
 * @file           : sdi12.c
 * @brief          : SDI-12 library for STM32 microcontrollers.
 * 
 ******************************************************************************
 * @attention
 *
 * !! SDI-12 uses 5v logic, ensure your microcontrollers pins are 5V
 * tolerant !!
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

#include <sdi12.h>

SDI12_TypeDef sdi12;

#define SDI12_COM_Pin       GPIO_PIN_9
#define SDI12_COM_GPIO_Port GPIOA
#define GPIO_AF_USART1      GPIO_AF1_USART1

/* Private member functions */
static HAL_StatusTypeDef SDI12_QueryDevice(const char cmd[], const uint8_t cmd_len, char *response, const uint8_t response_len);
static HAL_StatusTypeDef SDI12_ReceiveLine(char buffer[], const uint8_t max, uint8_t *const count);

/*
 * Initialise with UART, TX Pin and TX Pin GPIO Port.
 */
void SDI12_Init(UART_HandleTypeDef *huart) {
    sdi12.Huart = huart;
    sdi12.Pin = SDI12_COM_Pin;
    sdi12.Port = SDI12_COM_GPIO_Port;
}

/*
 * Main function to deal with SDI12 commands and responses.
 * Struture is as follows.
 *
 * Break (12 ms)
      │                          380 - 810 ms
      ▼      Command              Response
    ┌───┐  ┌─┐ ┌─┐ ┌─┐           ┌─┐ ┌─┐ ┌─┐
    │   │  │ │ │ │ │ │           │ │ │ │ │ │
────┘   └──┘ └─┘ └─┘ └───────────┘ └─┘ └─┘ └─
          ▲               Max
          │          ◄───15 ms───►
      Marking (8.3 ms)
 *
 *
 * Uses a single-wire for UART TX/RX, and cycles to GPIO for
 * break and marking
 */
static HAL_StatusTypeDef SDI12_QueryDevice(const char cmd[], const uint8_t cmd_len, char response[], const uint8_t response_len) {

    // Setup GPIO pin for break
    GPIO_InitTypeDef GPIO_InitStruct = { 0 };
    GPIO_InitStruct.Pin = sdi12.Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(sdi12.Port, &GPIO_InitStruct);

    // Break must be >= 12 ms
    HAL_GPIO_WritePin(sdi12.Port, (uint16_t) sdi12.Pin, GPIO_PIN_SET);
    HAL_Delay(12);

    // Marking must be >= 8.3 ms
    HAL_GPIO_WritePin(sdi12.Port, (uint16_t) sdi12.Pin, GPIO_PIN_RESET);
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Alternate = GPIO_AF_USART1;
    HAL_GPIO_Init(sdi12.Port, &GPIO_InitStruct);
    HAL_Delay(9);

    __HAL_UART_DISABLE(sdi12.Huart);
    HAL_HalfDuplex_EnableTransmitter(sdi12.Huart);
    __HAL_UART_ENABLE(sdi12.Huart);

    // Transmit
    HAL_StatusTypeDef res;

    res = HAL_UART_Transmit(sdi12.Huart, (uint8_t*) cmd, cmd_len, 1000);
    if (res != HAL_OK) {
        return res;
    }

    uint8_t count = 0;
    res = SDI12_ReceiveLine(response, response_len, &count);
    return res;
}

/*
 * Receive a CR/LF terminated string from the sensor.
 *
 * buffer - the buffer to store received characters. Will contain the CR/LF characters
 *          if they are read off the data line and they fit within the buffer.
 * max - the maximum number of characters to receive - ie the size of buffer.
 * count - the number of characters received is written to count.
 *
 * HAL_ERROR or HAL_TIMEOUT may be returned, otherwise HAL_OK.
 */
static HAL_StatusTypeDef SDI12_ReceiveLine(char buffer[], const uint8_t max, uint8_t *const count) {
    if (buffer == 0 || max == 0 || count == 0) {
        return HAL_ERROR;
    }

    // Put the SDI-12 pin into RX mode so the sensor response can be read.
    __HAL_UART_DISABLE(sdi12.Huart);
    HAL_HalfDuplex_EnableReceiver(sdi12.Huart);
    __HAL_UART_ENABLE(sdi12.Huart);

    // Receive up to max chars, break on CR/LF pair.
    HAL_StatusTypeDef res = HAL_OK;
    uint8_t i = 0;
    uint8_t c;
    while (i < max) {
        // Wait for up to 100ms for each character before timing out. This is to
        // handle waiting for the first character after a command has been sent,
        // where you are meant to have 3 retries with the last being after 100ms.
        res = HAL_UART_Receive(sdi12.Huart, &c, 1, 110);
        if (res == HAL_TIMEOUT) {
            break;
        }

        buffer[i++] = c;
        if (c == 0x0a) {
            break;
        }
    }

    while (i > 0) {
        c = buffer[i - 1];
        if (c == 0x0a || c == 0x0d) {
            buffer[i - 1] = 0;
            i--;
        } else {
            break;
        }
    }

    *count = i;
    return res;
}

/*
 * Simple SDI12 command to determine if a device is active on the queried address.
 * Expected response {'0', '\r', '\n'} where '0' is the address.
 */
HAL_StatusTypeDef SDI12_AckActive(const char addr) {
    char cmd[3] = { addr, '!', 0x00 };
    char response[3] = { 0, 0, 0 };
    HAL_StatusTypeDef result = SDI12_QueryDevice(cmd, 2, response, 3);
    return result;
}

/*
 * Used to populate a list of connected device addresses.
 */
void SDI12_DevicesOnBus(char *const devices) {
    uint8_t index = 0;
    for (char i = '0'; i <= '9'; i++) {
        HAL_StatusTypeDef result = SDI12_AckActive(i);
        if (result == HAL_OK) {
            devices[index++] = i;
        }

        HAL_Delay(200);
    }
}

/*
 * Issue the 'aI!' command.
 */
HAL_StatusTypeDef SDI12_GetId(const char addr, char response[], uint8_t response_len) {
    char cmd[] = { addr, 'I', '!', 0x00 };
    HAL_StatusTypeDef result = SDI12_QueryDevice(cmd, 3, response, response_len);
    return result;
}

/*
 * Change a devices SDI12 address.
 * May not work on all devices. Only those who support this feature.
 * Expected response {'1', '\r', '\n'} where '1' is the new address
 */
HAL_StatusTypeDef SDI12_ChangeAddr(char *from_addr, char *to_addr) {
    char cmd[5] = { *from_addr, 'A', *to_addr, '!', 0x00 };
    char response[3] = { 0 };
    HAL_StatusTypeDef result = SDI12_QueryDevice(cmd, 4, response, 3);
    return result;
}

/*
 * Start measurement. This command tells the sensor you want to take a measurement.
 * However, this command does not return any data. Instead it gives you
 * information regarding what a measurement would contain and how long it would take
 * to be captured.
 * To get data the user must call the send data command (D0!). -> SDI12_SendData(...)
 * Expected response {'0', '0', '0', '1', '3', '\r', '\n'}
 * Expected response as = atttn -> address (a), 3 numbers representing processing time (t)
 * and n results (n).
 */
HAL_StatusTypeDef SDI12_StartMeasurement(const char addr, SDI12_Measure_TypeDef *measurement_info) {
    char cmd[4] = { addr, 'M', '!', 0x00 };
    char response[7] = { 0 };
    HAL_StatusTypeDef result = SDI12_QueryDevice(cmd, 3, response, 7);

    // Check for valid response
    if (response[0] != '\0') {
        // Address of queried device (a)
        measurement_info->Address = response[0];

        // Extract time from response to be converted to uint16_t
        char time_buf[3];
        for (uint8_t i = 1; i < 4; i++) {
            time_buf[i - 1] = response[i];
        }

        // Convert time_buf (ttt) into uint16_t
        sscanf(time_buf, "%hd", &(measurement_info->Time));

        // Number of values to expect in measurement (n)
        measurement_info->NumValues = response[4] - '0'; // char to uint8_t
    };

    return result;
}

/*
 * Send measurement data from the sensor. Must be called after a M, C or V
 * command. Called until the number of measurements (obtained in a M command)
 * are received.
 * The populated array (data) should have sufficient size to hold all values
 * returned from the sensor (approx upto 10 * 75 = 750).
 */
HAL_StatusTypeDef SDI12_SendData(const char addr, char *data, const size_t num_measurements, const bool is_continuous) {

    uint16_t index = 0; // Holds position in data array
    uint8_t n_values = 0; // Holds index of number of values received


    char command;
    if (is_continuous) 	command = 'R';
    else 			   	command = 'D';

    // Loop through until all the data has been captured (matching NumValues)
    char cmd[] = { addr, command, 0, '!', 0x00 };
    for (char i = '0'; i < '9'; i++) {
        cmd[2] = i;
        char response[MAX_RESPONSE_SIZE + 1] = { 0 };
        HAL_StatusTypeDef result = SDI12_QueryDevice(cmd, 4, response,
        MAX_RESPONSE_SIZE);
        if (result != HAL_OK) {
            return result;
        }

        uint8_t res_index = 0;
        for (uint8_t x = 1; x < MAX_RESPONSE_SIZE; x++) {
            // Total number of + and - should equal measurement_info->NumValues if all values have been received
            if (response[x] == '+' || response[x] == '-') {
                n_values++;
            }
            // No need to go search beyond received data
            if (response[x] == '\0') {
                break;
            }

            res_index++;
        }

        if (res_index > 0) {
            memcpy(&data[index], &response[1], res_index);
            index += res_index;
            data[index] = 0;
        }

        // All values received
        if (n_values == num_measurements) {
            return HAL_OK;
        }
    }

    return HAL_ERROR;
}

/**
 * Parse idenfitication of SDI12 Sensor based on SDI12 Manual
 * Section 4.4.2
 *
 * Function assumes that response fits parameters described in manual
 */
void ParseSDI12Identification (const char* response, SDI12Identification* out) {
	if (strlen(response) < 17 || !out) return;

	out->address = response[0];

	const size_t VENDOR_OFFSET = 3;
	const size_t VENDOR_LEN = 8;
	memset(out->vendor, 0, sizeof(out->vendor));
	memcpy(out->vendor, &response[VENDOR_OFFSET], VENDOR_LEN);

	for (int i = VENDOR_LEN - 1; i >= 0; i--) {
		if (out->vendor[i] == ' ') out->vendor[i] = '\0';
		else break;
	}

	const size_t MODEL_OFFSET = 11;
	const size_t MODEL_LEN = 6;

	memset(out->model, 0, sizeof(out->model));
	memcpy(out->model, &response[MODEL_OFFSET], MODEL_LEN);

	for (int i = MODEL_LEN - 1; i >= 0; i--) {
		if (out->model[i] == ' ') out->model[i] = '\0';
		else break;
	}



}
