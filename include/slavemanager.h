/**
 * @file slavemanager.h
 * @brief Header file for slavemanager.cpp.
 * @details This file contains declarations for the SlaveManager class and the SlaveDevice struct.
 *          The SlaveManager class is responsible for managing slave devices and their file
 *          descriptors.
 * @author Daan Breur
 */

#ifndef SLAVEMANAGER_H
#define SLAVEMANAGER_H

/**
 * @brief Biggest possible slave ID.
 */
#define MAX_SLAVE_ID 0xFF

#include <stddef.h>
#include <stdint.h>

#include "packets.h"

/**
 * @brief Structure representing a slave device.
 * @details This structure contains the file descriptor associated with the slave device, and also
 * its current state in the form of a packet.
 */
struct SlaveDevice {
    int fd = -1;
    struct sensor_packet sensor_data = {0};

    bool isConnected() const;
    void setSensorData(const struct sensor_packet &);
};

class SlaveManager {
   private:
    SlaveDevice slave_devices[MAX_SLAVE_ID + 1];

   public:
    SlaveManager();
    ~SlaveManager();

    SlaveManager(const SlaveManager &) = delete;
    SlaveManager &operator=(const SlaveManager &) = delete;
    SlaveManager(SlaveManager &&) = delete;
    SlaveManager &operator=(SlaveManager &&) = delete;

    /**
     * @brief Registers a slave device with the given ID and file descriptor.
     * @param slave_id The ID of the slave device to register.
     * @param fd The file descriptor associated with the slave device.
     * @throws std::invalid_argument if the slave ID is invalid.
     */
    void registerSlave(uint8_t slave_id, int fd);

    /**
     * @brief Unregisters a slave device with the given ID.
     * @param slave_id The ID of the slave device to unregister.
     * @throws std::invalid_argument if the slave ID is invalid.
     * @warning This method closes the file descriptor associated with the slave device.
     */
    void unregisterSlave(uint8_t slave_id);

    /**
     * @brief Sends data to the slave device with the given ID.
     * @param slave_id The ID of the slave device to send data to.
     * @param data The data to send to the slave device.
     * @param length The length of the data to send.
     * @return 0 on success, -1 on failure.
     */
    int sendToSlave(uint8_t slave_id, const void *data, size_t length);

    /**
     * @brief Gets the file descriptor associated with the given slave ID.
     * @param slave_id The ID of the slave device.
     * @return The file descriptor associated with the slave device.
     */
    int getSlaveFD(uint8_t slave_id) const;

    /**
     * @brief Updates the internal sensor_packet structure that contains the current state of the
     * device
     * @param slave_id The ID of the slave device.
     * @param packet A sensor_packet structure that represents the updated internal state of the
     * device.
     */
    void updateSlaveState(uint8_t slave_id, const struct sensor_packet &packet);

    /**
     * @brief Retrieves the internal sensor_packet structure that contains the current state of the
     * device
     * @param slave_id The ID of the slave device.
     * @return The internal state of the device as a sensor_packet struct
     */
    struct sensor_packet getSlaveState(uint8_t slave_id);
};

#endif
