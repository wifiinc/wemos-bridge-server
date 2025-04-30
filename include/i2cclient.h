/**
 * @file i2cclient.h
 * @brief Header file for i2cclient.cpp.
 * @details This file contains declarations for the classes and functions
 *          used in the Wemos server application.
 * @author Daan Breur
 */

#ifndef I2CCLIENT_H
#define I2CCLIENT_H

#include <string>

class I2CClient {
   private:
    int client_fd;
    int port;
    int hub_port;
    std::string hub_ip;

   public:
    /**
     * @brief Constructor for I2CClient class.
     * @details This constructor initializes the I2C client with the specified IP address and port.
     * @param ip The IP address of the I2C hub.
     * @param port The port number of the I2C hub.
     * @throws std::invalid_argument if the port number is invalid.
     * @warning This constructor does not start the I2C client. The connect() method
     */
    I2CClient(const std::string &, int);
    ~I2CClient();

    I2CClient(const I2CClient &) = delete;
    I2CClient &operator=(const I2CClient &) = delete;
    I2CClient(I2CClient &&) = delete;
    I2CClient &operator=(I2CClient &&) = delete;

    /**
     * @brief Connects to the I2C hub.
     * @details This method establishes a connection to the I2C hub using the specified IP address
     * and port.
     * @throws std::runtime_error if the connection fails.
     */
    void connect();

    /**
     * @brief Disconnects from the I2C hub.
     * @details This method closes the connection to the I2C hub.
     */
    void disconnect();

    /**
     * @brief Sends data to the I2C hub.
     * @param data The data to send to the I2C hub.
     * @param length The length of the data to send.
     * @throws std::runtime_error if sending data fails.
     */
    void send(uint8_t *data, size_t length);

    /**
     * @brief Receives data from the I2C hub.
     * @return A struct containing the received data and its length.
     * @throws std::runtime_error if receiving data fails.
     */
    struct DataReceiveReturn receive();

    struct DataReceiveReturn {
        uint8_t *data;
        size_t length;
    };
};

#endif
