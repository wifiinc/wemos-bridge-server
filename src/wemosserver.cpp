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
#include <asm-generic/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <iostream>
#include <list>
#include <stdexcept>
#include <string>
#include <thread>

#include "packets.h"

/**
 * @brief Maximum data size to be read or sent over the wire.
 */
#define BUFFER_SIZE 1024

/**
 * @brief Maximum number of clients that can be connected to the server.
 */
#define MAX_CLIENTS 100

// private methods start here
void WemosServer::handleClient(int client_fd, const struct sockaddr_in &client_address) {
    uint8_t buffer[BUFFER_SIZE] = {0};
    ssize_t bytes_received;

    std::cout << "Thread " << std::this_thread::get_id() << " : Connection accepted from "
              << inet_ntoa(client_address.sin_addr) << ':' << ntohs(client_address.sin_port)
              << std::endl;

    while ((bytes_received = recv(client_fd, buffer, BUFFER_SIZE, 0)) > 0) {
        printf("Received %zd bytes from %s:%d:\n", bytes_received,
               inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));

        for (int i = 0; i < bytes_received; i++) printf("%02X ", buffer[i]);
        printf("\n");

        size_t offset = 0;
        while (offset + 2 <= bytes_received) {
            uint8_t length = buffer[offset];
            uint8_t ptype = buffer[offset + 1];

            if (offset + length > bytes_received) {
                printf("Incomplete packet received, discarding\n");
                break;
            }

            if (ptype == (uint8_t)PacketType::DATA) {
                SensorType type = (SensorType)buffer[offset + 2];

                printf("Packet length: %u, type: %u\n", length, type);

                processSensorData(&buffer[offset + sizeof(struct sensor_header)], length, type);
            } else if (ptype == (uint8_t)PacketType::HEARTBEAT) {
                struct sensor_heartbeat *heartbeat =
                    (struct sensor_heartbeat *)&buffer[offset + sizeof(struct sensor_header)];
                printf("Heartbeat packet: ID=%u, type=%u\n", heartbeat->id, heartbeat->type);

                // Register the slave device
                slave_manager.registerSlave(heartbeat->id, client_fd);
            }

            offset += length + sizeof(struct sensor_header);
        }
    }

    if (bytes_received == 0) {
        printf("Connection closed by %s:%d\n", inet_ntoa(client_address.sin_addr),
               ntohs(client_address.sin_port));
    } else if (bytes_received < 0) {
        perror("recv failed");
    }

    close(client_fd);
}

void WemosServer::processSensorData(const uint8_t *data, size_t length, SensorType type) {
    switch (type) {
        case SensorType::BUTTON: {
            const struct sensor_packet_generic *btn = (const struct sensor_packet_generic *)data;
            printf("Processing button data: ID=%u\n", btn->id);

            // TODO: een tafel-knop ingedrukt
            /*
            if (btn->id == 0x69) {
                toggle_led(0x50);
                struct sensor_header header = {.length = sizeof(struct sensor_packet_light),
                                               .type = DATA};
                struct sensor_packet_light led_control = {
                    .type = LIGHT, .id = 0x50, .target_state = get_led_state(0x50)};

                uint8_t buffer[sizeof(struct sensor_header) + sizeof(struct sensor_packet_light)] =
                    {0};
                memcpy(buffer, &header, sizeof(header));
                memcpy(buffer + sizeof(header), &led_control, sizeof(led_control));

                send_to_slave(0x50, &buffer, sizeof(buffer));
            }

            if (btn->id == 0x70) {
                toggle_led(0x55);
                struct i2c_led_control led_control = {.led_number = 0x55,
                                                      .led_state = get_led_state(0x55)};
                send_to_rpi(&led_control, sizeof(led_control));
            }
            */

            break;
        }
        case SensorType::TEMPERATURE: {
            const struct sensor_packet_temperature *temp =
                (const struct sensor_packet_temperature *)data;
            printf("Processing temperature data: ID=%u, Value=%.2f\n", temp->id, temp->value);

            // TODO: do temperature things
            break;
        }
        case SensorType::CO2: {
            const struct sensor_packet_co2 *co2 = (const struct sensor_packet_co2 *)data;
            printf("Processing CO2 data: ID=%u, Value=%u\n", co2->id, co2->value);

            // TODO: do CO2 things
            break;
        }
        case SensorType::HUMIDITY: {
            const struct sensor_packet_humidity *hum = (const struct sensor_packet_humidity *)data;
            printf("Processing humidity data: ID=%u, Value=%.2f\n", hum->id, hum->value);

            // TODO: do humidity things
            break;
        }
        default:
            printf("No action defined for sensor type %u\n", type);
            break;
    }
}
// private methods end here

WemosServer::WemosServer(int port, const std::string &hub_ip, int hub_port)
    : server_fd(-1), hub_ip(hub_ip), hub_port(hub_port), i2c_client() {
    if (port <= 0 || port > 65535) throw std::invalid_argument("Invalid listen port number");

    if (INADDR_NONE == inet_addr(hub_ip.c_str()))
        throw std::invalid_argument("Invalid hub IP address passed");
    if (hub_port <= 0 || hub_port > 65535) throw std::invalid_argument("Invalid hub port passed");

    memset(&listen_address, 0, sizeof(listen_address));
    listen_address.sin_family = AF_INET;
    listen_address.sin_addr = {INADDR_ANY};
    listen_address.sin_port = htons(port);
}

WemosServer::~WemosServer() {
    tearDown();
    // other shit
}

void WemosServer::socketSetup() {
    if ((server_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)) < 0) {
        perror("socket() failed");
        throw std::runtime_error("socket() failed");
        exit(EXIT_FAILURE);
    }

    const int enable_opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &enable_opt,
                   sizeof(enable_opt)) < 0) {
        perror("setsockopt() failed");
        throw std::runtime_error("setsockopt() failed");
        exit(EXIT_FAILURE);
    }

    if (bind(server_fd, (struct sockaddr *)&listen_address, sizeof(listen_address)) < 0) {
        perror("bind() failed");
        throw std::runtime_error("bind() failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("listen() failed");
        throw std::runtime_error("listen() failed");
        exit(EXIT_FAILURE);
    }

    std::cout << "Listening on port " << ntohs(listen_address.sin_port) << " (max " << MAX_CLIENTS
              << " clients)" << std::endl;
}

void WemosServer::setupI2cClient() { i2c_client.setup(hub_ip, hub_port); }

void WemosServer::start() {
    setupI2cClient();
    i2c_client.openConnection();
    i2c_client.start();

    while (true) {
        struct sensor_packet pkt;
        try {
            pkt = i2c_client.retrievePacket();
        } catch (std::runtime_error &) {
            // this means there is no new I2C packet available
        }

        struct sockaddr_in client_address;
        socklen_t client_addr_len = sizeof(client_address);
        int client_fd = accept(server_fd, (struct sockaddr *)&client_address, NULL);

        if (-1 == client_fd) {
            // no one tried to connect
            continue;
        }

        handleClient(client_fd, client_address);
    }
}

void WemosServer::tearDown() {
    close(server_fd);
    i2c_client.closeConnection();
}
