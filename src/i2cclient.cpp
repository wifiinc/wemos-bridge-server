/**
 * @file i2cclient.cpp
 * @brief Implementation of I2CClient class.
 * @author Daan Breur
 */

#include "i2cclient.h"

I2CClient::I2CClient(const std::string &ip, int port) {}

I2CClient::~I2CClient() { disconnect(); }

void I2CClient::connect() {}

void I2CClient::disconnect() {}

void I2CClient::send(uint8_t *data, size_t length) {}

struct I2CClient::DataReceiveReturn receive() {}