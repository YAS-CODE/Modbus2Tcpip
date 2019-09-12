/*
 * ModbusController_test.cpp
 *
 *  Created on: Feb 13, 2015
 *      Author: imtiazahmed
 */

#include <iostream>
#include "ModbusController.h"

using namespace std;

int main() {


	ModbusController modbusController;

	uint16_t result[1];
	modbusController.sendCommand(31, 203, 1, result);


	return 0;
}
