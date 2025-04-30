/**
 * @file wemosserver.h
 * @brief Header file for wemosserver.cpp.
 * @details This file contains declarations for the classes and functions
 *          used in the Wemos server application.
 * @author Daan Breur
 */

#ifndef WEMOSSERVER_H
#define WEMOSSERVER_H

#include <string>

#include "i2cclient.h"
#include "slavemanager.h"

class WemosServer {
   private:
    int server_fd;
    int port;
    int hub_port;
    std::string hub_ip;
    I2CClient i2c_client;
    SlaveManager slave_manager;

    void handleClient(int client_fd, const struct sockaddr_in &client_address);

    void processSensorData(const uint8_t *data, size_t length);

   public:
    /**
     * @brief Constructor for WemosServer class.
     * @details This constructor initializes the server with the specified port, hub IP address, and
     * hub port.
     * @param _port The port number on which the server will listen for incoming connections.
     * @param _hub_ip The IP address of the I2C hub.
     * @param _hub_port The port number of the I2C hub.
     * @throws std::invalid_argument if the port number is invalid.
     * @warning This constructor does not start the server loop. The loop() method
     *          should be called separately to start accepting client connections.
     */
    WemosServer(int _port, const std::string &_hub_ip, int _hub_port);
    ~WemosServer();

    WemosServer(const WemosServer &) = delete;
    WemosServer &operator=(const WemosServer &) = delete;
    WemosServer(WemosServer &&) = delete;
    WemosServer &operator=(WemosServer &&) = delete;

    /**
     * @brief Sets up the server socket and starts listening for incoming connections.
     * @details This method creates a socket, binds it to the specified port, and
     *          starts listening for incoming client connections. It also sets the
     *          socket options to allow address reuse.
     * @throws std::runtime_error if socket creation, binding, or listening fails.
     * @warning This method should be called before starting the server loop.
     */
    void socket_setup();

    /**
     * @brief Sets up the I2C client for communication with the I2C hub.
     */
    void setup_i2c_client();

    void start();

    void tearDown();
};

#endif
