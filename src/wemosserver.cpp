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

#include <sys/socket.h>

#include <iostream>
#include <stdexcept>
#include <string>

/**
 * @brief Constructor for WemosServer class.
 * @details This constructor initializes the server with the specified port, hub IP address, and hub
 * port.
 * @param port The port number on which the server will listen for incoming connections.
 * @param hub_ip The IP address of the I2C hub.
 * @param hub_port The port number of the I2C hub.
 * @throws std::invalid_argument if the port number is invalid.
 * @warning This constructor does not start the server loop. The loop() method
 *          should be called separately to start accepting client connections.
 */
WemosServer::WemosServer(int port, const std::string &hub_ip, int hub_port) {}

WemosServer::~WemosServer() {}

/**
 * @brief Sets up the server socket and starts listening for incoming connections.
 * @details This method creates a socket, binds it to the specified port, and
 *          starts listening for incoming client connections. It also sets the
 *          socket options to allow address reuse.
 * @throws std::runtime_error if socket creation, binding, or listening fails.
 * @warning This method should be called before starting the server loop.
 */
void WemosServer::socket_setup() {}

void WemosServer::loop() {}

void WemosServer::tearDown() {}