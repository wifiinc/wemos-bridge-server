/**
 * @file test_math.cpp
 * @brief Unit tests for mathematical operations using Google Test framework.
 *
 * This file contains test cases for verifying the correctness of functions
 * defined in the "math.h" header. The tests ensure that the mathematical
 * operations behave as expected under various conditions.
 *
 * @test MathTest.Add
 * - Verifies that the `add` function correctly computes the sum of two integers.
 * - Example: `add(2, 3)` should return `5`.
 *
 * @test MathTest.Subtract
 * - Verifies that the `subtract` function correctly computes the difference
 *   between two integers.
 * - Examples:
 *   - `subtract(10, 3)` should return `7`.
 *   - `subtract(9, 3)` should return `6`.
 *
 * @test MathTest.SubtractNegative
 * - Verifies that the `subtract` function handles subtraction with negative
 *   integers correctly.
 * - Example: `subtract(10, -3)` should return `13`.
 */
#include <gtest/gtest.h>

#include "math.h"

TEST(MathTest, Add) { EXPECT_EQ(add(2, 3), 5); }

TEST(MathTest, Subtract) {
    EXPECT_EQ(subtract(10, 3), 7);
    EXPECT_EQ(subtract(9, 3), 6);
}

TEST(MathTest, SubtractNegative) { EXPECT_EQ(subtract(10, -3), 13); }
