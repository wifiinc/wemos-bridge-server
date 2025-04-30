/**
 * @file test_slavemanager.cpp
 * @brief Unit tests for SlaveManager class.
 * @author Daan Breur
 */
#include <gtest/gtest.h>

#include "slavemanager.h"

TEST(SlaveManagerTests, RegisterSlave_ValidID) {
    SlaveManager manager;
    int fd = 5;

    EXPECT_NO_THROW(manager.registerSlave(1, fd));
    EXPECT_NO_THROW(manager.registerSlave(255, fd));
}

TEST(SlaveManagerTests, RegisterSlave_InvalidID) {
    SlaveManager manager;
    int fd = 5;

    EXPECT_THROW(manager.registerSlave(-1, fd), std::invalid_argument);
    EXPECT_THROW(manager.registerSlave(256, fd), std::invalid_argument);
}
