#ifndef FF9A3C64_8D45_4C1F_9A1D_9E3277F7C3A1
#define FF9A3C64_8D45_4C1F_9A1D_9E3277F7C3A1

#include <ratio>
#include <type_traits>
#include <format>
#include <ostream>
#include <utility>
#include <stdexcept>

namespace afs::mem_units {
    // Helper trait to detect std::ratio specializations
    template<typename T>
    struct is_ratio : std::false_type {
    };

    template<std::intmax_t Num, std::intmax_t Den>
    struct is_ratio<std::ratio<Num, Den> > : std::true_type {
    };

    template<typename T>
    concept RatioType = is_ratio<T>::value;

    template<typename T>
    concept RepType = std::is_arithmetic_v<T>;

    template<RepType Rep, RatioType Ratio>
    class memory_unit {
        Rep _count = 0;

    public:
        using ratio = Ratio;
        using rep = Rep;
        using this_type = memory_unit<Rep, Ratio>;

        constexpr memory_unit() = default;

        /// Construct with explicit count.
        explicit constexpr memory_unit(const Rep count) : _count(count) {}

        /// Construct from other memory unit with same or greater ratio.
        template<RatioType OtherRatio>
            requires std::ratio_greater_equal_v<OtherRatio, ratio>
        explicit constexpr memory_unit(const memory_unit<Rep, OtherRatio> &other) {
            const auto other_converted = memory_unit_cast<this_type>(other);
            _count = other_converted.count();
        }

        /// Assign from other memory unit with same or greater ratio.
        template<RatioType OtherRatio>
            requires std::ratio_greater_equal_v<OtherRatio, ratio>
        constexpr this_type& operator=(const memory_unit<Rep, OtherRatio> &other) {
            const auto other_converted = memory_unit_cast<this_type>(other);
            _count = other_converted.count();
            return *this;
        }

        /// Return the current count.
        [[nodiscard]] constexpr Rep count() const { return _count; }

        [[nodiscard]] constexpr memory_unit operator+(const memory_unit &other) const {
            if (std::cmp_less(std::numeric_limits<Rep>::max() - count(), other.count())) {
                throw std::overflow_error("Addition would cause an overflow!");
            }
            return memory_unit{static_cast<Rep>(count() + other.count())};
        }

        /// `this` + \a other, where ratio of `this` is smaller than ratio of \a other.
        /// The returned type is then of `this`:
        /// ~~~~~.cpp
        /// assert(4_b + 2_kb == 2052_b)
        /// ~~~~~
        template<RatioType OtherRatio>
            requires std::ratio_greater_v<OtherRatio, ratio>
        [[nodiscard]] constexpr this_type operator+(const memory_unit<Rep, OtherRatio> &other) const {
            const auto other_converted = memory_unit_cast<this_type>(other);
            return operator+(other_converted);
        }

        /// `this` + \a other, where ratio of `this` is greater than ratio of \a other.
        /// The returned type is then of \a other:
        /// ~~~~~.cpp
        /// assert(1_kb + 2b == 1026_b)
        /// ~~~~~
        template<RatioType OtherRatio>
            requires std::ratio_less_v<OtherRatio, ratio>
        [[nodiscard]] constexpr memory_unit<Rep, OtherRatio> operator+(const memory_unit<Rep, OtherRatio> &other) const {
            using other_type = memory_unit<Rep, OtherRatio>;
            const other_type converted = memory_unit_cast<other_type>(*this);
            return other.operator+(converted);
        }

        [[nodiscard]] constexpr memory_unit operator-(const memory_unit &other) const {
            if (std::cmp_less(std::numeric_limits<Rep>::min() + count(), other.count())) {
                throw std::underflow_error("Subtraction would cause an underflow!");
            }
            return memory_unit{static_cast<Rep>(count() - other.count())};
        }
    };

    template<typename T>
    struct is_memory_unit : std::false_type {
    };

    template<RepType Rep, std::intmax_t Num, std::intmax_t Den>
    struct is_memory_unit<memory_unit<Rep, std::ratio<Num, Den> > > : std::true_type {
    };

    template<typename T>
    concept MemoryUnitType = is_memory_unit<T>::value;

    template<RepType Rep, RatioType Ratio>
    constexpr bool operator==(const memory_unit<Rep, Ratio> &lhs, const memory_unit<Rep, Ratio> &rhs) {
        return lhs.count() == rhs.count();
    }

    template<RepType Rep, RatioType Ratio>
    constexpr bool operator<(const memory_unit<Rep, Ratio> &lhs, const memory_unit<Rep, Ratio> &rhs) {
        return lhs.count() < rhs.count();
    }

    template<RepType Rep, RatioType Ratio>
    constexpr bool operator>(const memory_unit<Rep, Ratio> &lhs, const memory_unit<Rep, Ratio> &rhs) {
        return lhs.count() > rhs.count();
    }

    /// Returns if \a lhs `==` \a rhs, where \a lhs has the greater or an equal ratio.
    template<MemoryUnitType LhsType, MemoryUnitType RhsType>
        requires std::ratio_greater_equal_v<typename LhsType::ratio, typename RhsType::ratio>
    constexpr bool operator==(const LhsType &lhs, const RhsType &rhs) {
        auto converted = memory_unit_cast<RhsType>(lhs);
        return std::cmp_equal(converted.count(), rhs.count());
    }

    /// Returns if \a lhs `==` \a rhs, where \a lhs has the smaller ratio.
    template<MemoryUnitType LhsType, MemoryUnitType RhsType>
        requires std::ratio_less_v<typename LhsType::ratio, typename RhsType::ratio>
    constexpr bool operator==(const LhsType &lhs, const RhsType &rhs) {
        auto converted = memory_unit_cast<LhsType>(rhs);
        return std::cmp_equal(converted.count(), lhs.count());
    }

    /// Returns if \a lhs `>` \a rhs, where \a lhs has the greater or an equal ratio.
    template<MemoryUnitType LhsType, MemoryUnitType RhsType>
        requires std::ratio_greater_equal_v<typename LhsType::ratio, typename RhsType::ratio>
    constexpr bool operator>(const LhsType &lhs, const RhsType &rhs) {
        const auto lhs_converted = memory_unit_cast<RhsType>(lhs);
        return std::cmp_greater(lhs_converted.count(), rhs.count());
    }

    /// Returns if \a lhs `>` \a rhs, where \a lhs has the smaller ratio.
    template<MemoryUnitType LhsType, MemoryUnitType RhsType>
        requires std::ratio_less_v<typename LhsType::ratio, typename RhsType::ratio>
    constexpr bool operator>(const LhsType &lhs, const RhsType &rhs) {
        const auto rhs_converted = memory_unit_cast<LhsType>(rhs);
        return std::cmp_greater(lhs.count(), rhs_converted.count());
    }

    /// Returns if \a op1 `*` \a op2 does NOT fit into unsigned  \t RepType.
    template <RepType Rep>
    [[nodiscard]] constexpr bool wouldMultiplicationOverflow(const Rep& op1, const auto& op2) {
        if (not std::cmp_equal(op2, 0)) {
            return std::cmp_less(std::numeric_limits<Rep>::max() / op2, op1);
        }
        return false;
    }

    /// Returns if \a op1 `*` \a op2 does NOT fit into signed  \t RepType.
    template <RepType Rep>
        requires std::is_signed_v<Rep>
    [[nodiscard]] constexpr bool wouldMultiplicationOverflow(const Rep& op1, const auto& op2) {
        if (not std::cmp_equal(op2, 0)) {
            return std::cmp_less(std::numeric_limits<Rep>::max() / op2, std::abs(op1));
        }
        return false;
    }

    template<MemoryUnitType ToType, MemoryUnitType FromType>
    [[nodiscard]] constexpr ToType memory_unit_cast(const FromType& from) {
        using Rep = typename ToType::rep;
        using conversion = std::ratio_divide<typename FromType::ratio, typename ToType::ratio>;
        if (wouldMultiplicationOverflow(from.count(), conversion::num)) {
            throw std::overflow_error("Conversion would cause an overflow!");
        }
        const auto temp = from.count() * conversion::num;
        const auto converted_count = static_cast<Rep>(temp / conversion::den);
        return ToType{converted_count};
    }

    /// Returns if \a lhs `==` \a rhs, where \a lhs has the greater or an equal ratio.
    template<MemoryUnitType LhsType, MemoryUnitType RhsType>
        requires std::ratio_greater_equal_v<typename LhsType::ratio, typename RhsType::ratio>
    constexpr RhsType operator+(const LhsType &lhs, const RhsType &rhs) {
        auto converted = memory_unit_cast<RhsType>(lhs);
        return {converted.count(), rhs.count()};
    }

    using bits = memory_unit<std::uint64_t, std::ratio<1, 8> >;
    using bytes = memory_unit<std::uint64_t, std::ratio<1> >;
    using kilobytes = memory_unit<std::uint64_t, std::ratio<bytes::ratio::num * 1'024> >;
    using megabytes = memory_unit<std::uint64_t, std::ratio<kilobytes::ratio::num * 1'024> >;
    using gigabytes = memory_unit<std::uint64_t, std::ratio<megabytes::ratio::num * 1'024> >;
    using terabytes = memory_unit<std::uint64_t, std::ratio<gigabytes::ratio::num * 1'024> >;
    using petabytes = memory_unit<std::uint64_t, std::ratio<terabytes::ratio::num * 1'024> >;
    using exabytes = memory_unit<std::uint64_t, std::ratio<petabytes::ratio::num * 1'024> >;

    template<MemoryUnitType MemoryUnit>
    [[nodiscard]] constexpr std::string_view memory_unit_suffix() {
        if constexpr (std::is_same_v<MemoryUnit, bits>) return "bit";      // bits
        else if constexpr (std::is_same_v<MemoryUnit, bytes>) return "b";     // bytes
        else if constexpr (std::is_same_v<MemoryUnit, kilobytes>) return "kb";
        else if constexpr (std::is_same_v<MemoryUnit, megabytes>) return "mb";
        else if constexpr (std::is_same_v<MemoryUnit, gigabytes>) return "gb";
        else if constexpr (std::is_same_v<MemoryUnit, terabytes>) return "tb";
        else if constexpr (std::is_same_v<MemoryUnit, petabytes>) return "pb";
        else if constexpr (std::is_same_v<MemoryUnit, exabytes>)  return "eb";
        else return "?";
    }

    namespace literals {
        constexpr bits operator"" _bit(unsigned long long count) {
            return bits{static_cast<bits::rep>(count)};
        }

        constexpr bytes operator"" _b(unsigned long long count) {
            return bytes{static_cast<bytes::rep>(count)};
        }

        constexpr kilobytes operator"" _kb(unsigned long long count) {
            return kilobytes{static_cast<kilobytes::rep>(count)};
        }

        constexpr megabytes operator"" _mb(unsigned long long count) {
            return megabytes{static_cast<megabytes::rep>(count)};
        }

        constexpr gigabytes operator"" _gb(unsigned long long count) {
            return gigabytes{static_cast<gigabytes::rep>(count)};
        }

        constexpr terabytes operator"" _tb(unsigned long long count) {
            return terabytes{static_cast<terabytes::rep>(count)};
        }

        constexpr petabytes operator"" _pb(unsigned long long count) {
            return petabytes{static_cast<petabytes::rep>(count)};
        }

        constexpr exabytes operator"" _eb(unsigned long long count) {
            return exabytes{static_cast<exabytes::rep>(count)};
        }

    }
}

namespace std {
    template<>
    struct formatter<afs::mem_units::bits> : formatter<afs::mem_units::bits::rep> {
        auto format(const afs::mem_units::bits& value, std::format_context& context) const {
            return std::format("{}{}", value.count(), afs::mem_units::memory_unit_suffix<afs::mem_units::bits>());
        }
    };
}

#endif // FF9A3C64_8D45_4C1F_9A1D_9E3277F7C3A1
