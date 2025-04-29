/**
 * @file test_wemosserver.cpp
 * @brief Unit tests for WemosServer class.
 * @details This file contains unit tests for the WemosServer class, which
 *          handles the server functionality for the Wemos device. The tests
 *          cover various aspects of the class, including socket setup,
 *          client handling, and communication with the I2C hub.
 * @author Daan Breur
 */
#include <gtest/gtest.h>

#include "wemosserver.h"

/**
 * @test WemosServerTest.Constructor_ValidPort
 * @brief Test the constructor with valid port numbers.
 * @details
 * - Test the constructor of WemosServer with valid port numbers.
 * - Expect no exceptions to be thrown.
 * @ingroup WemosServerTest
 */
TEST(WemosServerTest, Constructor_ValidPort) {
    EXPECT_NO_THROW(WemosServer server(5000, "10.0.0.1", 5000));
    EXPECT_NO_THROW(WemosServer server(6969, "10.0.0.1", 5000));
    EXPECT_NO_THROW(WemosServer server(65535, "10.0.0.1", 5000));
}

/**
 * @test WemosServerTest.Constructor_InvalidPort_Negative
 * @details
 * - Verify that the constructor throws an exception when a negative port number is provided.
 * - Expects std::invalid_argument to be thrown.
 * @ingroup WemosServerTest
 */
TEST(WemosServerTest, Constructor_InvalidPort_Negative) {
    EXPECT_THROW(WemosServer server(-1, "10.0.0.1", 5000), std::invalid_argument);
}

/**
 * @test WemosServerTest.Constructor_InvalidPort_Zero
 * @details
 * - Verify that the constructor throws an exception when a port number of zero is provided.
 * - Expects std::invalid_argument to be thrown.
 * @ingroup WemosServerTest
 */
TEST(WemosServerTest, Constructor_InvalidPort_Zero) {
    EXPECT_THROW(WemosServer server(0, "10.0.0.1", 5000), std::invalid_argument);
}

/**
 * @test WemosServerTest.Constructor_InvalidPort_High
 * @details
 * - Verify that the constructor throws an exception when a port number greater than 65535 is
 * provided.
 * - Expects std::invalid_argument to be thrown.
 * @ingroup WemosServerTest
 */
TEST(WemosServerTest, Constructor_InvalidPort_High) {
    EXPECT_THROW(WemosServer server(65536, "10.0.0.1", 5000), std::invalid_argument);
    EXPECT_THROW(WemosServer server(69696, "10.0.0.1", 5000), std::invalid_argument);
}

/**
 * @test WemosServerTest.Constructor_ValidHubIPAddress
 * @details
 * - Test the constructor of WemosServer with valid hub IP addresses.
 * - Expect no exceptions to be thrown.
 * @ingroup WemosServerTest
 */
TEST(WemosServerTest, Constructor_ValidHubIPAddress) {
    EXPECT_NO_THROW(WemosServer server(5000, "10.0.0.1", 5000));
    EXPECT_NO_THROW(WemosServer server(5000, "192.168.10.10", 5000));
}

/**
 * @test WemosServerTest.Constructor_InvalidHubIPAddress
 * @brief Test the constructor with invalid hub IP addresses.
 * @details
 * - Verify that the constructor throws an exception when an invalid hub IP address is provided.
 * - Testcases include:
 *   - Empty string
 *   - Invalid IP format (e.g., "192.168.0")
 *   - Non-numeric characters (e.g., "invalid_ip")
 *   - Out of range IP address (e.g., "256.256.256.256")
 * - Expects std::invalid_argument to be thrown.
 * @ingroup WemosServerTest
 */
TEST(WemosServerTest, Constructor_InvalidHubIPAddress) {
    EXPECT_THROW(WemosServer server(5000, "", 5000), std::invalid_argument);
    EXPECT_THROW(WemosServer server(5000, "192.168.0", 5000), std::invalid_argument);
    EXPECT_THROW(WemosServer server(5000, "invalid_ip", 5000), std::invalid_argument);
    EXPECT_THROW(WemosServer server(5000, "256.256.256.256", 5000), std::invalid_argument);
}

/**
 * @test WemosServerTest.Constructor_ValidHubPort
 * @brief Test the constructor with valid hub port numbers.
 * @ingroup WemosServerTest
 */
TEST(WemosServerTest, Constructor_ValidHubPort) {
    EXPECT_NO_THROW(WemosServer server(5000, "10.0.0.1", 5000));
    EXPECT_NO_THROW(WemosServer server(5000, "10.0.0.1", 6969));
    EXPECT_NO_THROW(WemosServer server(5000, "10.0.0.1", 65535));
}

/**
 * @test WemosServerTest.Constructor_InvalidHubPort_Negative
 * @brief Test the constructor with invalid negative hub port numbers.
 * @details
 * - Test the constructor of WemosServer with invalid hub port numbers that are negative.
 * - Expect std::invalid_argument to be thrown.
 * @ingroup WemosServerTest
 */
TEST(WemosServerTest, Constructor_InvalidHubPort_Negative) {
    EXPECT_THROW(WemosServer server(5000, "10.0.0.1", -1), std::invalid_argument);
}

/**
 * @test WemosServerTest.Constructor_InvalidHubPort_High
 * @brief Test the constructor with invalid hub port numbers.
 * @details
 * - Test the constructor of WemosServer with invalid hub port numbers that are to high.
 * - Expect std::invalid_argument to be thrown.
 * @ingroup WemosServerTest
 */
TEST(WemosServerTest, Constructor_InvalidHubPort_High) {
    EXPECT_THROW(WemosServer server(5000, "10.0.0.1", 65536), std::invalid_argument);
    EXPECT_THROW(WemosServer server(5000, "10.0.0.1", 69696), std::invalid_argument);
}

/**
 * @test WemosServerTest.Constructror_InvalidHubPort_Zero
 * @brief Test the constructor with invalid hub port numbers.
 * @details
 * - Test the constructor of WemosServer with invalid hub port numbers that are zero.
 * - Expect std::invalid_argument to be thrown.
 * @ingroup WemosServerTest
 */
TEST(WemosServerTest, Constructor_InvalidHubPort_Zero) {
    EXPECT_THROW(WemosServer server(5000, "10.0.0.1", 0), std::invalid_argument);
}
