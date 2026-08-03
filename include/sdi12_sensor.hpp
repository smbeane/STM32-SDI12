/*
 ******************************************************************************
 * @file           : sdi12_sensor.hpp
 * @brief          : SDI-12 library for STM32 microcontrollers.
 *
 ******************************************************************************
 */

#ifndef SDI12_SENSOR_HPP
#define SDI12_SENSOR_HPP

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32g0xx_hal.h"

#ifdef __cplusplus
} // extern "C"
#endif

#ifdef __cplusplus
class Sensor {
public:
	Sensor(): address('0'), commandType('M'), numMeasurements(0) {}

	Sensor(char addr, char cmdType, size_t num_meas)
		: address(addr), commandType(cmdType), numMeasurements(num_meas) {}

	~Sensor() = default;

	HAL_StatusTypeDef makeReading(char* data) const;

	char getAddress() const { return address; }
	char getCommandType() const { return commandType; }

private:

	char address;
	char commandType;
	size_t numMeasurements;
};

#endif // __cplusplus

#endif // SDI12_SENSOR_HPP
