/**
 * @file i2cclient.h
 * @brief Header file for i2cclient.cpp.
 * @details This file contains declarations for the classes and functions
 *          used in the Wemos server application.
 * @author Daan Breur
 * @author Erynn Scholtes
 */

#ifndef I2CCLIENT_H
#define I2CCLIENT_H

#include <arpa/inet.h>

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

#include "packets.h"

class I2CClient {
   private:
    int client_fd;

    struct sockaddr_in hub_address;

    std::thread receive_thread;

    std::atomic<bool> connected;
    std::atomic<bool> running;

    std::mutex receive_mutex;
    std::mutex queue_mutex;

    std::condition_variable queue_condition;

    std::queue<struct sensor_packet> read_packets_queue;

    /**
     * @brief Internal receive loop for handling incoming data from the I2C hub.
     * @details This method runs in a separate thread and continuously listens for incoming data
     * from the I2C hub. It processes the received data and stores it in a buffer for later use.
     * @warning This method should not be called directly. It is intended to be used internally
     * by the class.
     */
    void receiveLoop();

   public:
    struct DataReceiveReturn {
        uint8_t *data;
        size_t length;
    };

   public:
    /**
     * @brief Constructor for I2CClient class.
     * @details This constructor initializes the I2C client with the specified IP address and port.
     * @throws std::invalid_argument if the port number is invalid.
     * @warning This constructor does not start the I2C client. Use setup(), openConnection() and start()
     * instead.
     */
    I2CClient();
    ~I2CClient();

    I2CClient(const I2CClient &) = delete;
    I2CClient &operator=(const I2CClient &) = delete;
    I2CClient(I2CClient &&) = delete;
    I2CClient &operator=(I2CClient &&) = delete;

    /**
     * @brief Initializes the settings necessary for connecting to the I2C hub.
     * @details This method initializes the remote address details (IP address and port) for the I2C
     * hub to connect to.
     * @param ip The IP address of the I2C hub.
     * @param port The port number of the I2C hub.
     * @throws std::invalid_argument if an invalid IP address or port is passed
     */
    void setup(const std::string &ip, int port);

    /**
     * @brief Connects to the I2C hub.
     * @details This method establishes a connection to the I2C hub using the specified IP address
     * and port.
     * @return true if the connection is successful, false otherwise.
     */
    bool openConnection();

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
    void closeConnection();

    /**
     * @brief Internal method to send data to the I2C hub.
     * @param data The data to send to the I2C hub.
     * @param length The length of the data to send.
     * @throws std::runtime_error if sending data fails.
     */
    void sendRawData(uint8_t *data, size_t length);

    /**
     * @brief Sends packet data to the I2C hub.
     * @param t.b.d.
     * @throws std::runtime_error if sending data fails.
     */
    // void sendData();

    /**
     * @brief Receives data from the I2C hub.
     * @param block Whether or not to block until a packet can be retrieved
     * @return A struct containing the received packet data.
     * @throws std::runtime_error if receiving data fails.
     */
    struct sensor_packet retrievePacket(bool block = false);
};

#endif
