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

#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>

#include "packets.h"
#include "slavemanager.h"

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
            struct sensor_packet *pkt_ptr = (struct sensor_packet *)&buffer[offset];
            uint8_t data_length = pkt_ptr->header.length;
            PacketType ptype = pkt_ptr->header.ptype;
            SensorType s_type = pkt_ptr->data.generic.metadata.sensor_type;
            uint8_t s_id = pkt_ptr->data.generic.metadata.sensor_id;

            if (offset + data_length + sizeof(struct sensor_header) > bytes_received) {
                printf("Incomplete packet received, discarding\n");
                break;
            }

            switch (ptype) {
                case PacketType::DATA:
                    printf("Packet length: %u, type: %u\n", data_length, s_type);

                    processSensorData((const struct sensor_packet *)&buffer[offset]);
                    break;

                case PacketType::HEARTBEAT:
                    printf("Heartbeat packet: ID=%u, type=%u\n",
                           pkt_ptr->data.heartbeat.metadata.sensor_id,
                           pkt_ptr->data.heartbeat.metadata.sensor_type);

                    // Register the slave device
                    slave_manager.registerSlave(pkt_ptr->data.heartbeat.metadata.sensor_id,
                                                client_fd);
                    break;

                case PacketType::DASHBOARD_GET:
                    printf("Dashboard requested data on sensor: ID=%u, type=%u\n",
                           pkt_ptr->data.generic.metadata.sensor_id,
                           pkt_ptr->data.generic.metadata.sensor_type);

                    if (s_id < 128) {
                        // YIPEE
                        const struct SlaveDevice &s_device =
                            slave_manager.getSlaveDevice(pkt_ptr->data.generic.metadata.sensor_id);
                        struct sensor_packet s_packet(s_device.sensor_data);
                        sendToDashboard(client_fd, &s_packet,
                                        sizeof(s_packet.header) + s_packet.header.length);
                    } else {
                        i2c_client.sendRawData((uint8_t *)pkt_ptr,
                                               sizeof(struct sensor_header) + data_length);

                        printf("incoming data: ");
                        for (int i = 0; i < sizeof(struct sensor_header) + pkt_ptr->header.length;
                             ++i) {
                            printf("%02X ", ((uint8_t *)(pkt_ptr))[i]);
                        }
                        printf("\n");
                        struct sensor_packet ret_pkt;
                        do {
                            ret_pkt = i2c_client.retrievePacket(true);
                        } while (ret_pkt.data.generic.metadata.sensor_id !=
                                 pkt_ptr->data.generic.metadata.sensor_id);

                        printf("sending back to dashboard :D\n");
                        sendToDashboard(client_fd, pkt_ptr,
                                        sizeof(struct sensor_header) + data_length);
                    }
                    break;

                case PacketType::DASHBOARD_POST:
                    printf("Dashboard posting data on sensor: ID=%u, type=%u\n",
                           pkt_ptr->data.generic.metadata.sensor_id,
                           pkt_ptr->data.generic.metadata.sensor_type);
                    // the dashboard is trying to update something
                    if (s_id < 128) {
                        // blabla
                        slave_manager.sendToSlave(
                            pkt_ptr->data.generic.metadata.sensor_id, (uint8_t *)pkt_ptr,
                            sizeof(struct sensor_header) + pkt_ptr->header.length);
                    } else {
                        i2c_client.sendRawData((uint8_t *)pkt_ptr,
                                               sizeof(struct sensor_header) + data_length);
                    }
                    break;

                default:
                    // unknown packet type
                    break;
            }

            offset += data_length + sizeof(struct sensor_header);
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

void WemosServer::processSensorData(const struct sensor_packet *packet) {
    SlaveDevice &target_slave =
        slave_manager.getSlaveDevice(packet->data.generic.metadata.sensor_id);
    target_slave.setSensorData(*packet);

    switch (packet->data.generic.metadata.sensor_type) {
        case SensorType::BUTTON: {
            printf("Processing button data: ID=%u\n", packet->data.generic.metadata.sensor_id);

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
            printf("Processing temperature data: ID=%u, Value=%.2f\n",
                   packet->data.temperature.metadata.sensor_id, packet->data.temperature.value);

            // TODO: do temperature things
            break;
        }
        case SensorType::CO2: {
            printf("Processing CO2 data: ID=%u, Value=%u\n", packet->data.co2.metadata.sensor_id,
                   packet->data.co2.value);

            // TODO: do CO2 things
            break;
        }
        case SensorType::HUMIDITY: {
            printf("Processing humidity data: ID=%u, Value=%.2f\n",
                   packet->data.humidity.metadata.sensor_id, packet->data.humidity.value);

            // TODO: do humidity things
            break;
        }
        default:
            printf("No action defined for sensor type %u\n",
                   packet->data.generic.metadata.sensor_type);
            break;
    }
}

void WemosServer::sendToDashboard(int dashboard_fd, struct sensor_packet *pkt_ptr, size_t len) {
    struct sensor_packet sensor_data;

    send(dashboard_fd, pkt_ptr, len, 0);
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
    socketSetup();

    setupI2cClient();
    i2c_client.openConnection();
    i2c_client.start();

    while (true) {
        struct sensor_packet pkt;
        try {
            pkt = i2c_client.retrievePacket();
            // std::cout << "packet received from the I2C hub!" << std::endl;

            // now do things with the I2C packet if necessary

        } catch (std::runtime_error &exc) {
            /* this means there is no new I2C packet available */
            // std::cerr << exc.what() << std::endl;
        }

        struct sockaddr_in client_address;
        socklen_t client_addr_len = sizeof(client_address);
        int client_fd = accept(server_fd, (struct sockaddr *)&client_address, &client_addr_len);

        if (-1 == client_fd) {
            // no one tried to connect
            continue;
        }

        std::cout << "Connection accepted from " << inet_ntoa(client_address.sin_addr) << ":"
                  << ntohs(client_address.sin_port) << std::endl;

        handleClient(client_fd, client_address);
    }
}

void WemosServer::tearDown() {
    close(server_fd);
    i2c_client.closeConnection();
}
