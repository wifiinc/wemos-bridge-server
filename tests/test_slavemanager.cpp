/**
 * @file test_slavemanager.cpp
 * @brief Unit tests for SlaveManager class.
 * @author Daan Breur
 */
#include <gtest/gtest.h>

#include "slavemanager.h"

TEST(SlaveManagerTests, RegisterSlave_Success) {
    SlaveManager manager;
    int fd = 5;

    EXPECT_NO_THROW(manager.registerSlave(1, fd));
    EXPECT_EQ(manager.getSlaveFD(1), fd);
    EXPECT_EQ(manager.getSlaveDevice(1).fd, fd);
}
