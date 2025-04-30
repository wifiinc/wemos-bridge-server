/**
 * @file i2cclient.h
 * @brief Header file for i2cclient.cpp.
 * @details This file contains declarations for the classes and functions
 *          used in the Wemos server application.
 * @author Daan Breur
 */

#ifndef I2CCLIENT_H
#define I2CCLIENT_H

#include <atomic>
#include <mutex>
#include <string>
#include <thread>

class I2CClient {
   private:
    int client_fd;
    int port;
    int hub_port;
    struct sockaddr_in hub_address;

    std::string hub_ip;
    std::thread receive_thread;
    std::atomic<bool> connected;
    std::atomic<bool> running;
    std::mutex receive_mutex;

    /**
     * @brief Internal receive loop for handling incoming data from the I2C hub.
     * @details This method runs in a separate thread and continuously listens for incoming data
     * from the I2C hub. It processes the received data and stores it in a buffer for later use.
     * @warning This method should not be called directly. It is intended to be used internally by
     * the class.
     */
    void receiveLoop();

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
     * @return true if the connection is successful, false otherwise.
     */
    bool connect();

    /**
     * @brief Starts the I2C client.
     * @details This method starts the I2C client and begins listening for incoming data from the
     * I2C hub.
     * @throws std::runtime_error if the client is not connected to the hub.
     */
    void start();

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
