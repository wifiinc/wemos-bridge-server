/**
 * @file test_i2cclient.cpp
 * @brief Unit tests for I2CClient class.
 * @author Daan Breur
 */
#include <gtest/gtest.h>

#include "i2cclient.h"

/**
 * @test I2CClientTests.Constructor_ValidPort
 * @brief Test the constructor with valid port numbers.
 * @details
 * - Test the constructor of WemosServer with valid port numbers.
 * - Expect no exceptions to be thrown.
 * @ingroup I2CClientTests
 */
TEST(I2CClientTests, Constructor_ValidPort) {
    EXPECT_NO_THROW(I2CClient server("10.0.0.1", 5000));
    EXPECT_NO_THROW(I2CClient server("10.0.0.1", 6969));
    EXPECT_NO_THROW(I2CClient server("10.0.0.1", 65535));
}

/**
 * @test I2CClientTests.Constructor_InvalidPort_Negative
 * @details
 * - Verify that the constructor throws an exception when a negative port number is provided.
 * - Expects std::invalid_argument to be thrown.
 * @ingroup I2CClientTests
 */
TEST(I2CClientTests, Constructor_InvalidPort_Negative) {
    EXPECT_THROW(I2CClient server("10.0.0.1", -1), std::invalid_argument);
}

/**
 * @test I2CClientTests.Constructor_InvalidPort_Zero
 * @details
 * - Verify that the constructor throws an exception when a port number of zero is provided.
 * - Expects std::invalid_argument to be thrown.
 * @ingroup I2CClientTests
 */
TEST(I2CClientTests, Constructor_InvalidPort_Zero) {
    EXPECT_THROW(I2CClient server("10.0.0.1", 0), std::invalid_argument);
}

/**
 * @test I2CClientTests.Constructor_InvalidPort_High
 * @details
 * - Verify that the constructor throws an exception when a port number greater than 65535 is
 * provided.
 * - Expects std::invalid_argument to be thrown.
 * @ingroup I2CClientTests
 */
TEST(I2CClientTests, Constructor_InvalidPort_High) {
    EXPECT_THROW(I2CClient server("10.0.0.1", 65536), std::invalid_argument);
    EXPECT_THROW(I2CClient server("10.0.0.1", 69696), std::invalid_argument);
}
