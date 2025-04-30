/**
 * @file i2cclient.cpp
 * @brief Implementation of I2CClient class.
 * @author Daan Breur
 */

#include "i2cclient.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <iostream>

I2CClient::I2CClient(const std::string &ip, int port) {}

I2CClient::~I2CClient() { disconnect(); }

bool I2CClient::connect() {
    if (client_fd >= 0) {
        close(client_fd);
        client_fd = -1;
    }
    connected = false;

    std::cout << "Connecting to I2C hub at " << hub_ip << ":" << hub_port << std::endl;

    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd < 0) {
        std::cerr << "Socket creation failed" << std::endl;
        return false;
    }

    memset(&hub_address, 0, sizeof(hub_address));
    hub_address.sin_family = AF_INET;
    hub_address.sin_port = htons(hub_port);

    if (inet_pton(AF_INET, hub_ip.c_str(), &hub_address.sin_addr) <= 0) {
        std::cerr << "Invalid IP address: " << strerror(errno) << std::endl;
        close(client_fd);
        client_fd = -1;
        return false;
    }

    if (connect(client_fd, (struct sockaddr *)&hub_address, sizeof(hub_address)) < 0) {
        std::cerr << "Connection failed: " << strerror(errno) << std::endl;
        close(client_fd);
        client_fd = -1;
        return false;
    }

    std::cout << "Connected to I2C hub at " << hub_ip << ":" << hub_port << std::endl;
    connected = true;

    return true;
}

void I2CClient::start() {
    if (!connected) {
        std::cerr << "Not connected to I2C hub" << std::endl;
        return;
    }

    running = true;
    receive_thread = std::thread(&I2CClient::receiveLoop, this);
}

void I2CClient::disconnect() {}

void I2CClient::send(uint8_t *data, size_t length) {}

struct I2CClient::DataReceiveReturn receive() {}