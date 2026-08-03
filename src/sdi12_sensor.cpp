/*
 ******************************************************************************
 * @file           : sdi12_sensor.cpp
 * @brief          : SDI-12 library for STM32 microcontrollers.
 * 					 built for being dumb data pipeline, will be expaneded
 * 					 later to process sensor data
 *
 ******************************************************************************
 */

#include <sdi12.hpp>
#include <sdi12_sensor.hpp>

HAL_StatusTypeDef Sensor::makeReading(char* data) const {
	return SDI12_SendData(this->address, data, this->numMeasurements, true);
}
