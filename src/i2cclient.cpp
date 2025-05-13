/**
 * @file i2cclient.cpp
 * @brief Implementation of I2CClient class.
 * @author Daan Breur
 * @author Erynn Scholtes
 */

#include "i2cclient.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstdint>
#include <exception>
#include <iostream>
#include <queue>
#include <stdexcept>

#include "packets.h"

#define BUFFER_SIZE 1024

I2CClient::I2CClient() : client_fd(-1), connected(false), running(false) {
    memset(&hub_address, 0, sizeof(hub_address));
}

I2CClient::~I2CClient() {
    if (connected) closeConnection();
}

// first unlocks the mutex passed, then continues in the while loop
// WARNING: ! only use when the mutex is currently locked !
// just "continue;" otherwise
#define THREAD_RELINQUISH(mut) \
    {                          \
        mut.unlock();          \
        continue;              \
    }

void I2CClient::receiveLoop() {
    uint8_t receive_buffer[BUFFER_SIZE] = {0};
    struct pollfd pf;

    pf.fd = client_fd;
    pf.events = POLLIN;

    while (true == running && true == connected) {
        // TODO: revise error handling within the loop;
        // maybe always stop loop on error, instead of just continuing?
        // problem for another time :clueless:
        // - Erynn

        receive_mutex.lock();

        // poll offers an easy way to wait up to one second between iterations
        int sockets_ready = poll(&pf, 1, 1000);

        if (sockets_ready < 1) {
            // something went wrong
            if (sockets_ready == -1) perror("poll() failed");  // error happened, else timeout

            THREAD_RELINQUISH(receive_mutex);
        }

        // if we get here, there is guaranteed to be readable data.
        // either this data is because the other end disconnected, or because there
        // is proper data to read from the wire
        int amount_read = recv(pf.fd, receive_buffer, BUFFER_SIZE, MSG_DONTWAIT);

        if (amount_read == -1) {
            // error occured, errno set
            perror("recv() failed");

            THREAD_RELINQUISH(receive_mutex);
        } else if (amount_read == 0) {
            // socket disconnected
            connected = false;
            running = false;
            client_fd = -1;

            THREAD_RELINQUISH(receive_mutex);
        }

        receive_mutex.unlock();

        std::cout << "Received " << amount_read << " bytes from Raspberry PI I2C controller."
                  << std::endl;

        for (int i = 0; i < amount_read; ++i) {
            printf("%02X ", receive_buffer[i]);
            printf("\n");
        }

        {
            size_t buffer_offset = 0;

            while (buffer_offset + sizeof(struct sensor_header) <= amount_read) {
                const struct sensor_header *head = (const struct sensor_header *)receive_buffer;
                uint8_t length = head->length;
                uint8_t p_type = (uint8_t)head->ptype;

                uint8_t s_type = receive_buffer[sizeof(*head)];

                if (buffer_offset + length > amount_read) {
                    // oopsie woopsie; incomplete packet from RPI
                    printf(
                        "We received an incomplete packet from the Raspberry Pi I2C controller; "
                        "Discarding...\n");
                    break;
                }
            }

            struct sensor_packet packet = {0};
            int to_copy = amount_read;
            if (to_copy > sizeof(packet)) to_copy = sizeof(packet);
            memcpy(&packet, receive_buffer, to_copy);

            queue_mutex.lock();
            read_packets_queue.push(packet);
            queue_mutex.unlock();
            queue_condition
                .notify_one();  // maybe switch this with the line before if issues occur - Erynn
        }
    }

    std::terminate();
}

void I2CClient::setup(const std::string &hub_ip, int hub_port) {
    if (inet_pton(AF_INET, hub_ip.c_str(), &hub_address.sin_addr) <= 0) {
        perror("inet_pton()");
        throw std::invalid_argument("Invalid IP address");
    }

    if (hub_port <= 0 || hub_port > 65535) throw std::invalid_argument("Invalid port number");

    hub_address.sin_family = AF_INET;
    hub_address.sin_port = htons(hub_port);
}

bool I2CClient::openConnection() {
    if (client_fd >= 0) {
        close(client_fd);
        client_fd = -1;
    }
    connected = false;

    std::string ip(inet_ntoa(hub_address.sin_addr));
    uint16_t port = ntohs(hub_address.sin_port);

    std::cout << "Connecting to I2C hub at " << ip << ":" << port << std::endl;

    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd < 0) {
        std::cerr << "Socket creation failed" << std::endl;
        return false;
    }

    if (connect(client_fd, (struct sockaddr *)&hub_address, sizeof(hub_address)) < 0) {
        int err = errno;
        std::cerr << "Connection failed: " << strerror(err) << std::endl;
        close(client_fd);
        client_fd = -1;
        return false;
    }

    std::cout << "Connected to I2C hub at " << ip << ":" << port << std::endl;
    connected = true;

    return true;
}

void I2CClient::start() {
    if (!connected) {
        std::cerr
            << "Could not start communicating with I2C-bridge because not connected to I2C hub"
            << std::endl;
        throw std::runtime_error("Not connected to I2C-bridge");
    }

    running = true;
    receive_thread = std::thread(&I2CClient::receiveLoop, this);
}

void I2CClient::closeConnection() {
    if (!connected) {
        std::cerr << "Could not close the connection to I2C-bridge because not connected to I2C "
                     "hub (either already closed, or never connected in the first place)"
                  << std::endl;
        return;
    }

    running = false;
    receive_thread.join();
}

void I2CClient::sendRawData(uint8_t *data, size_t length) {
    if (send(client_fd, data, length, 0) == -1) {
        perror("send() failed");
        throw std::runtime_error("Sending data to I2C-bridge failed");
    }
}

struct sensor_packet I2CClient::retrievePacket(bool block) {
    queue_mutex.lock();

    if (read_packets_queue.size() < 1 && !block) {
        // there are no packets available to retrieve,
        // and we're not blocking
        throw std::runtime_error("No packet data available to retrieve from I2C-bridge");
    } else if (read_packets_queue.size() < 1) {
        // there are no packets available, but we block until there is
        std::unique_lock lk(queue_mutex);
        queue_condition.wait(lk);
    }

    struct sensor_packet return_packet;
    memcpy(&return_packet, &read_packets_queue.front(), sizeof(return_packet));
    read_packets_queue.pop();

    queue_mutex.unlock();

    return return_packet;
}
