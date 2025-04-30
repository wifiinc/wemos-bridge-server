/**
 * @file wemosserver.cpp
 * @brief Implementation of WemosServer class.
 * @details This file contains the implementation of the WemosServer class,
 *          which handles the server functionality for the Wemos device.
 *          It includes methods for setting up the server, handling client
 *          connections, and communicating with the I2C hub.
 * @author Daan Breur
 */
#include "wemosserver.h"

#include <arpa/inet.h>
#include <sys/socket.h>

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

/**
 * @brief Maximum number of clients that can be connected to the server.
 */
#define MAX_CLIENTS 100

WemosServer::WemosServer(int _port, const std::string &_hub_ip, int _hub_port)
    : server_fd(-1),
      port(_port),
      hub_ip(_hub_ip),
      hub_port(_hub_port),
      i2c_client(hub_ip, hub_port) {
    if (_port <= 0 || _port > 65535) {
        throw std::invalid_argument("Invalid port number");
    }

    struct sockaddr_in address;
    if (inet_pton(AF_INET, _hub_ip.c_str(), &(address.sin_addr)) <= 0) {
        throw std::invalid_argument("Invalid IP address");
    }

    if (_hub_port <= 0 || _hub_port > 65535) {
        throw std::invalid_argument("Invalid hub port number");
    }
}

WemosServer::~WemosServer() {
    tearDown();
    // other shit
}

void WemosServer::socket_setup() {
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        throw std::runtime_error("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    const int enable_opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &enable_opt, sizeof(enable_opt)) < 0) {
        perror("setsockopt failed");
        throw std::runtime_error("Setsockopt failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        throw std::runtime_error("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("listen failed");
        throw std::runtime_error("Listen failed");
        exit(EXIT_FAILURE);
    }

    std::cout << "Listening on port " << port << " (max " << MAX_CLIENTS << " clients)"
              << std::endl;
}

void WemosServer::setup_i2c_client() { i2c_client.connect(); }

void WemosServer::start() {}

void WemosServer::tearDown() {}