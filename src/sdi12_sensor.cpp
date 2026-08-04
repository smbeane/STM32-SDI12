/*
 ******************************************************************************
 * @file           : sdi12_sensor.cpp
 * @brief          : SDI-12 library for STM32 microcontrollers.
 * 					 built for being dumb data pipeline, will be expaneded
 * 					 later to process sensor data
 *
 ******************************************************************************
 */

#include <sdi12_sensor.hpp>

extern "C" {
	#include <sdi12.h>
}

HAL_StatusTypeDef Sensor::makeReading(char* data) const {
	return SDI12_SendData(this->address, data, this->numMeasurements, true);
}
