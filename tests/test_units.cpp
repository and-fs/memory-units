#include "mem_units.hpp"
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../inc/mem_units.hpp"

using ::testing::Eq;

namespace afs::mem_units {
    template<MemoryUnitType MemoryUnit>
    void PrintTo(const MemoryUnit& memunit, std::ostream* os) {
        *os << memunit.count() << memory_unit_suffix<MemoryUnit>();
    }
}

using namespace afs::mem_units;
using namespace afs::mem_units::literals;

TEST(AMemoryUnit, HasCountOf0WhenDefaultConstructed) {
    constexpr memory_unit<std::uint64_t, std::ratio<1>> unit{};
    ASSERT_THAT(unit.count(), Eq(0));
}

TEST(AMemoryUnit, HasCountOfConstruction) {
    constexpr memory_unit<std::uint64_t, std::ratio<1>> unit(42);
    ASSERT_THAT(unit.count(), Eq(42));
}

TEST(AMemoryUnit, IsComparableToSameType) {
    ASSERT_LT(bytes(8), bytes(42));
}

TEST(AMemoryUnit, CanAddAnotherUnitWithSameRatio) {
    ASSERT_THAT(8_b + 42_b, Eq(50_b));
}

TEST(AMemoryUnit, CanAddAnotherUnitWithGreaterRatio) {
    ASSERT_THAT(42_kb + 3_mb, Eq(3'114_kb));
}

TEST(AMemoryUnit, CanAddAnotherUnitWithSmallerRatio) {
    ASSERT_THAT(5_gb + 43_mb, Eq(5'163_mb));
}

TEST(AMemoryUnit, AddRaisesOverflowErrorIfSumWouldBeTooBig) {
    using test_unit = memory_unit<std::uint8_t, std::ratio<1>>;
    test_unit value_250(250), value_10(10);
    ASSERT_THROW(std::ignore = value_250 + value_10, std::overflow_error);
}

TEST(AMemoryUnit, CanSubtractAnotherUnitWithSameRatio) {
    ASSERT_THAT(1977_b - 176_b, Eq(1801_b));
}

TEST(AMemoryUnit, CanSubtractAnotherUnitWithGreaterRatio) {
    ASSERT_THAT(1028_kb - 1_mb, Eq(4_kb));
}

TEST(AMemoryUnit, CanSubtractAnotherUnitWithSmallerRatio) {
    ASSERT_THAT(2_mb - 2_kb, Eq(2046_kb));
}

TEST(AMemoryUnit, SubtractRaisesUnderflowErrorIfDifferenceIsBelowMinOfUnsigned) {
    using test_unit = memory_unit<std::uint8_t, std::ratio<1>>;
    test_unit value_10(10), value_12(12);
    ASSERT_THROW(std::ignore = value_10 - value_12, std::underflow_error);
}

TEST(AMemoryUnit, SubtractRaisesUnderflowErrorIfDifferenceIsBelowMinOfSigned) {
    using test_unit = memory_unit<std::int8_t, std::ratio<1>>;
    test_unit value_120(-120), value_42(42);
    ASSERT_THROW(std::ignore = value_120 - value_42, std::underflow_error);
}

TEST(AMemoryUnit, CanBeMultipliedByInteger) {
    EXPECT_THAT(2_mb * 4, Eq(8_mb));
    EXPECT_THAT(128_kb * 0, Eq(0_kb));
}

TEST(AMemoryUnit, CanMultiplyAnInteger) {
    EXPECT_THAT(4 * 2_mb, Eq(8_mb));
    EXPECT_THAT(0 * 128_kb, Eq(0_kb));
}

TEST(AMemoryUnit, CanBeMultipliedByFloat) {
    EXPECT_THAT(30_kb * 1.5f, Eq(45_kb));
    EXPECT_THAT(42_gb * 0.1f, Eq(4_gb));
}

TEST(AMemoryUnit, CanMultiplyAFloat) {
    EXPECT_THAT(1.5f * 30_kb, Eq(45_kb));
    EXPECT_THAT(0.1f * 42_gb, Eq(4_gb));
}

TEST(AMemoryUnit, SupportsBitLiteral) {
    ASSERT_THAT(3_bit, Eq(bits(3)));
}

TEST(AMemoryUnit, SupportsByteLiteral) {
    ASSERT_THAT(42_b, Eq(bytes(42)));
}

TEST(AMemoryUnit, SupportsKilobyteLiteral) {
    ASSERT_THAT(4_kb, Eq(kilobytes(4)));
}

TEST(AMemoryUnit, SupportsMegabyteLiteral) {
    ASSERT_THAT(9_mb, Eq(megabytes(9)));
}

TEST(AMemoryUnit, SupportsGigabyteLiteral) {
    ASSERT_THAT(123_gb, Eq(gigabytes(123)));
}

TEST(AMemoryUnit, SupportsTerabyteLiteral) {
    ASSERT_THAT(91_tb, Eq(terabytes(91)));
}

TEST(AMemoryUnit, SupportsPetabyteLiteral) {
    ASSERT_THAT(2_pb, Eq(petabytes(2)));
}

TEST(AMemoryUnit, SupportsExabyteLiteral) {
    ASSERT_THAT(76_eb, Eq(exabytes(76)));
}

TEST(AMemoryUnit, CanCompareEqualityWithSameUnitType) {
    ASSERT_EQ(1024_b, 1024_b);
}

TEST(AMemoryUnit, CanCompareEqualityWithBiggerUnitType) {
    ASSERT_EQ(1024_b, 1_kb);
}

TEST(AMemoryUnit, CanCompareEqualityWithSmallerUnitType) {
    ASSERT_EQ(1_mb, 1024_kb);
}

TEST(AMemoryUnit, CanCompareGreaterWithSameUnitType) {
    ASSERT_GT(234_b, 42_b);
}

TEST(AMemoryUnit, CanCompareGreaterWithBiggerUnitType) {
    ASSERT_GT(2_gb, 4000_kb);
}

TEST(AMemoryUnit, CanCompareGreaterWithSmallerUnitType) {
    ASSERT_GT(6000_kb, 2_mb);
}

TEST(AMemoryUnit, CanCastToSameUnitType) {
    ASSERT_THAT(memory_unit_cast<kilobytes>(42_kb), Eq(42_kb));
}

TEST(AMemoryUnit, CanCastToSmallerUnitType) {
    ASSERT_THAT(memory_unit_cast<kilobytes>(4_gb), Eq(kilobytes(4 * 1024 * 1024)));
}

TEST(AMemoryUnit, CanCastToBiggerUnitType) {
    ASSERT_THAT(memory_unit_cast<kilobytes>(4096_b), Eq(4_kb));
}

TEST(AMemoryUnit, CastToBiggerUnitTypeRoundsDown) {
    ASSERT_THAT(memory_unit_cast<kilobytes>(1234_b), Eq(1_kb));
    ASSERT_THAT(memory_unit_cast<kilobytes>(2047_b), Eq(1_kb));
}

TEST(AMemoryUnit, CastsFromBit) {
    ASSERT_THAT(memory_unit_cast<bytes>(8_bit), Eq(1_b));
    ASSERT_THAT(memory_unit_cast<bytes>(32_bit), Eq(4_b));
}

TEST(AMemoryUnit, IsCopyConstructibleFromSameUnit) {
    const bytes original(42_b);
    const bytes copied(original);
    ASSERT_EQ(original, copied);
}

TEST(AMemoryUnit, IsCopyConstructibleFromGreaterUnit) {
    const megabytes original = 2_mb;
    const bytes copied(original);
    ASSERT_EQ(original, copied);
}

TEST(AMemoryUnit, IsNotCopyConstructibleFromSmallerUnit) {
    // this expectedly fails to compile:
    // constexpr gigabytes gb(41_kb);
}

TEST(AMemoryUnit, IsAssignableFromSameUnit) {
    constexpr megabytes original(123);
    megabytes copied{};
    copied = original;
    ASSERT_EQ(original, copied);
}

TEST(AMemoryUnit, IsAssignableFromGreaterUnit) {
    constexpr gigabytes original(2);
    kilobytes copied{};
    copied = original;
    ASSERT_EQ(original, copied);
}

TEST(AMemoryUnit, IsAssignableFromSmallerUnit) {
    // this expectedly fails to compile:
    // constexpr gigabytes gb = 41_kb;
}


// TEST(AMemoryUnit, Of8BitsIsAByte) {
//     constexpr bits bit_value(8);
//     constexpr bytes byte_value(1);
//     ASSERT_THAT(byte_value, Eq(bit_value));
// }

TEST(MultiplicationOverflowCheck, ReturnsFalseIfOneOpIs0) {
    ASSERT_FALSE(wouldMultiplicationOverflow(std::numeric_limits<int>::max(), 0));
    ASSERT_FALSE(wouldMultiplicationOverflow(0, std::numeric_limits<unsigned int>::min()));
}

TEST(MultiplicationOverflowCheck, ReturnsFalseIfResultFitsInUnsignedType) {
    constexpr uint8_t op1 = 2, op2 = 42;
    ASSERT_FALSE(wouldMultiplicationOverflow(op1, op2));
    constexpr uint8_t op3 = 25, op4 = 10;
    ASSERT_FALSE(wouldMultiplicationOverflow(op3, op4));
    constexpr uint8_t op5 = std::numeric_limits<uint8_t>::max(), op6 = 1;
    ASSERT_FALSE(wouldMultiplicationOverflow(op5, op6));
}

TEST(MultiplicationOverflowCheck, ReturnsFalseIfResultFitsInSignedType) {
    constexpr int8_t op1 = -2, op2 = 42;
    ASSERT_FALSE(wouldMultiplicationOverflow(op1, op2));
    constexpr int8_t op3 = 12, op4 = 10;
    ASSERT_FALSE(wouldMultiplicationOverflow(op3, op4));
    constexpr uint8_t op5 = std::numeric_limits<int8_t>::max(), op6 = 1;
    ASSERT_FALSE(wouldMultiplicationOverflow(op5, op6));
    constexpr uint8_t op7 = 1, op8 = std::numeric_limits<int8_t>::min();
    ASSERT_FALSE(wouldMultiplicationOverflow(op7, op8));
}

TEST(MultiplicationOverflowCheck, ReturnsTrueIfResultDoesntFitInUnsignedType) {
    constexpr uint8_t op1 = 2, op2 = 128;
    ASSERT_TRUE(wouldMultiplicationOverflow(op1, op2));
    constexpr uint8_t op3 = 254, op4 = 10;
    ASSERT_TRUE(wouldMultiplicationOverflow(op3, op4));
}

TEST(MultiplicationOverflowCheck, ReturnsTrueIfResultDoesntFitInSignedType) {
    constexpr int8_t op1 = -2, op2 = 65;
    ASSERT_TRUE(wouldMultiplicationOverflow(op1, op2));
    constexpr int8_t op3 = 12, op4 = 11;
    ASSERT_TRUE(wouldMultiplicationOverflow(op3, op4));
}