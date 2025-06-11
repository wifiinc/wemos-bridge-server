/**
 * @file slavemanager.cpp
 * @brief Implementation of SlaveManager class.
 * @author Daan Breur
 */

#include "slavemanager.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstdio>
#include <cstring>
#include <stdexcept>

#include "packets.h"

bool SlaveDevice::isConnected() const { return (-1 != fd); }
void SlaveDevice::setSensorData(const struct sensor_packet& pkt) {
    memcpy(&sensor_data, &pkt, sizeof(sensor_data));
}

SlaveManager::SlaveManager() {
    for (int i = 0; i <= MAX_SLAVE_ID; ++i) {
        slave_devices[i].fd = -1;
    }
}

SlaveManager::~SlaveManager() {
    for (int i = 0; i <= MAX_SLAVE_ID; ++i) {
        if (slave_devices[i].fd >= 0) {
            close(slave_devices[i].fd);
            slave_devices[i].fd = -1;
        }
    }
}

void SlaveManager::registerSlave(uint8_t slave_id, int fd) {
    if (slave_id > MAX_SLAVE_ID || slave_id < 0) {
        printf("Invalid slave ID=%u\n", slave_id);
        throw std::invalid_argument("Invalid slave ID");
    }

    printf("Registering new slave ID=%u\n", slave_id);

    slave_devices[slave_id].fd = fd;
    memset(&slave_devices[slave_id].sensor_data, 0, sizeof(slave_devices[slave_id].sensor_data));
}

void SlaveManager::unregisterSlave(uint8_t slave_id) {
    if (slave_id > MAX_SLAVE_ID || slave_id < 0) {
        printf("Invalid slave ID=%u\n", slave_id);
        throw std::invalid_argument("Invalid slave ID");
    }

    printf("Unregistering slave ID=%u\n", slave_id);
    close(slave_devices[slave_id].fd);
    slave_devices[slave_id].fd = -1;
}

int SlaveManager::sendToSlave(uint8_t slave_id, const void* data, size_t length) {
    if (slave_id > MAX_SLAVE_ID || slave_id < 0) {
        printf("Invalid slave ID=%u\n", slave_id);
        throw std::invalid_argument("Invalid slave ID");
    }

    printf("Sending %zu bytes to slave ID=%u\n", length, slave_id);
    if (slave_devices[slave_id].fd < 0) {
        printf("Slave ID=%u not registered\n", slave_id);
        return -1;
    }

    ssize_t bytes_sent = send(slave_devices[slave_id].fd, data, length, 0);
    if (bytes_sent < 0) {
        perror("send to slave failed");
        return -1;
    }

    return 0;
}

int SlaveManager::getSlaveFD(uint8_t slave_id) const {
    if (slave_id > MAX_SLAVE_ID || slave_id < 0) {
        printf("Invalid slave ID=%u\n", slave_id);
        throw std::invalid_argument("Invalid slave ID");
    }

    return slave_devices[slave_id].fd;
}

void SlaveManager::updateSlaveState(uint8_t slave_id, const struct sensor_packet& packet) {
    slave_devices[slave_id].setSensorData(packet);
}

struct sensor_packet SlaveManager::getSlaveState(uint8_t slave_id) {
    return slave_devices[slave_id].sensor_data;
}
