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

TEST(AMemoryUnit, HasCountOf0WhenDefaultConstructed) {
    constexpr memory_unit<std::uint64_t, std::ratio<1>> unit{};
    ASSERT_THAT(unit.count(), Eq(0));
}

TEST(AMemoryUnit, HasCountOfConstruction) {
    constexpr memory_unit<std::uint64_t, std::ratio<1>> unit(42);
    ASSERT_THAT(unit.count(), Eq(42));
}

TEST(AMemoryUnit, IsComparableToSameType) {
    constexpr bytes byte_8(8);
    constexpr bytes byte_42(42);
    ASSERT_THAT(byte_8, ::testing::Lt(byte_42));
}

TEST(AMemoryUnit, CanAddAnotherOfSameType) {
    constexpr bytes byte_8(8);
    constexpr bytes byte_42(42);
    ASSERT_THAT((byte_8 + byte_42).count(), Eq(bytes(50).count()));
}

TEST(AMemoryUnit, AddRaisesOverflowErrorIfSumWouldBeTooBig) {
    using test_unit = memory_unit<std::uint8_t, std::ratio<1>>;
    test_unit value_250(250), value_10(10);
    ASSERT_THROW(std::ignore = value_250 + value_10, std::overflow_error);
}

TEST(AMemoryUnit, CanSubtractAnotherOfSameType) {
    constexpr bytes byte_1977(1977);
    constexpr bytes byte_176(176);
    ASSERT_THAT((byte_1977 - byte_176).count(), Eq(bytes(1801).count()));
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

TEST(AMemoryUnit, CanCompareEqualityWithSameUnitType) {
    constexpr bytes byte_a(1024), byte_b(1024);
    ASSERT_EQ(byte_a, byte_b);
}

TEST(AMemoryUnit, CanCompareEqualityWithBiggerUnitType) {
    constexpr bytes byte_1024(1024);
    constexpr kilobytes kb_1(1);
    ASSERT_EQ(byte_1024, kb_1);
}

TEST(AMemoryUnit, CanCompareEqualityWithSmallerUnitType) {
    constexpr megabytes mb(1);
    constexpr kilobytes kb(1024);
    ASSERT_EQ(mb, kb);
}

TEST(AMemoryUnit, CanCompareGreaterWithSameUnitType) {
    constexpr bytes byte_a(234), byte_b(42);
    ASSERT_GT(byte_a, byte_b);
}

TEST(AMemoryUnit, CanCompareGreaterWithBiggerUnitType) {
    constexpr kilobytes kb(4000);
    constexpr gigabytes gb(2);
    ASSERT_GT(gb, kb);
}

TEST(AMemoryUnit, CanCompareGreaterWithSmallerUnitType) {
    ASSERT_GT(kilobytes(6000), megabytes(2));
}

TEST(AMemoryUnit, CanCastToSameUnitType) {
    ASSERT_THAT(memory_unit_cast<kilobytes>(kilobytes(42)), Eq(kilobytes(42)));
}

TEST(AMemoryUnit, CanCastToSmallerUnitType) {
    ASSERT_THAT(memory_unit_cast<kilobytes>(gigabytes(4)), Eq(kilobytes(4 * 1024 * 1024)));
}

TEST(AMemoryUnit, CanCastToBiggerUnitType) {
    ASSERT_THAT(memory_unit_cast<kilobytes>(bytes(4096)), Eq(kilobytes(4)));
}

TEST(AMemoryUnit, CastToBiggerUnitTypeRoundsDown) {
    ASSERT_THAT(memory_unit_cast<kilobytes>(bytes(1234)), Eq(kilobytes(1)));
    ASSERT_THAT(memory_unit_cast<kilobytes>(bytes(2047)), Eq(kilobytes(1)));
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