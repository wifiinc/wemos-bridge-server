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

#include "packets.h"

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

void WemosServer::processSensorData(const uint8_t *data, size_t length, enum sensor_type type) {
    switch (type) {
        case BUTTON: {
            const struct sensor_packet_generic *btn = (const struct sensor_packet_generic *)data;
            printf("Processing button data: ID=%u\n", btn->id);

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

            break;
        }
        case TEMPERATURE: {
            const struct sensor_packet_temperature *temp =
                (const struct sensor_packet_temperature *)data;
            printf("Processing temperature data: ID=%u, Value=%.2f\n", temp->id, temp->value);
            break;
        }
        case CO2: {
            const struct sensor_packet_co2 *co2 = (const struct sensor_packet_co2 *)data;
            printf("Processing CO2 data: ID=%u, Value=%u\n", co2->id, co2->value);
            break;
        }
        case HUMIDITY: {
            const struct sensor_packet_humidity *hum = (const struct sensor_packet_humidity *)data;
            printf("Processing humidity data: ID=%u, Value=%.2f\n", hum->id, hum->value);
            break;
        }
        default:
            printf("No action defined for sensor type %u\n", type);
            break;
    }
}

void WemosServer::handleClient(int client_fd, const struct sockaddr_in &client_address) {
    uint8_t buffer[BUFFER_SIZE] = {0};
    ssize_t bytes_received;

    printf("Thread %d : Connection accepted from %s:%d\n", std::this_thread::get_id(),
           inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));

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

            if (ptype == DATA) {
                enum sensor_type type = (enum sensor_type)buffer[offset + 2];

                printf("Packet length: %u, type: %u\n", length, type);

                processSensorData(&buffer[offset + sizeof(struct sensor_header)], length, type);
            } else if (ptype == HEARTBEAT) {
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