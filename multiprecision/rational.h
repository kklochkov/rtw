#pragma once

#include "multiprecision/int128_math.h"

#include <cmath>
#include <type_traits>

namespace rtw::multiprecision
{

/// @brief An exact rational number represented as numerator/denominator.
///
/// Rational<T> stores a fraction in lowest terms with a positive denominator.
/// After every arithmetic operation, the result is normalized via GCD reduction.
///
/// Invariants (maintained after every operation):
///   - denominator > 0
///   - gcd(|numerator|, denominator) == 1
///   - If numerator == 0, denominator == 1
///
/// @note Arithmetic uses a double-width intermediate type (int64_t for T=int32_t, Int128 for T=int64_t)
/// to avoid overflow in cross-products. If the GCD-reduced result exceeds the representable range of T,
/// an assertion fires. Comparisons also use widened cross-multiplication.
///
/// @tparam T Underlying signed integer type (or signed big integer via Int<T>).
///
/// Complexity:
///   - Addition/Subtraction: O(log(max(a,b))) due to GCD normalization
///   - Multiplication/Division: O(log(max(a,b))) due to GCD normalization
///   - Comparison: O(1) (cross-multiply)
template <typename T>
class Rational
{
public:
  static_assert((std::is_integral_v<T> && std::is_signed_v<T> && !std::is_floating_point_v<T>)
                    || IS_SIGNED_BIG_INT_V<T>,
                "Template parameter T must be a signed integer type or a signed big integer type.");

  constexpr Rational(const T numerator, const T denominator) noexcept : numerator_{numerator}, denominator_{denominator}
  {
    normalize();
  }

  // NOLINTBEGIN(google-explicit-constructor, hicpp-explicit-conversions)
  template <typename I, std::enable_if_t<std::is_integral_v<I>, ArithmeticType> = ArithmeticType::INTEGRAL>
  constexpr Rational(const I numerator) noexcept : numerator_{numerator}
  {
  }

  /// @brief Constructs a Rational from a floating-point value.
  /// @note Not constexpr: uses std::frexp/std::ldexp/std::isnan/std::isinf which are not constexpr in C++17.
  /// @param[in] value The floating-point value to convert.
  /// @pre value must not be NaN or Inf.
  template <typename F, std::enable_if_t<std::is_floating_point_v<F>, ArithmeticType> = ArithmeticType::FLOATING_POINT>
  Rational(const F value) noexcept // NOLINT(google-explicit-constructor, hicpp-explicit-conversions)
  {
    if (std::isnan(value) || std::isinf(value))
    {
      assert(false && "Cannot convert NaN or Inf to Rational");
      numerator_ = T{0};
      denominator_ = T{1};
      return;
    }

    if (value == 0.0)
    {
      return;
    }

    std::int32_t exponent{};
    const auto mantissa = std::frexp(value, &exponent); // value = mantissa * 2^exponent, where 0.5 <= |mantissa| < 1

    // Cap mantissa digits to what T can represent without overflow.
    // T{1} << digits must fit in T, so digits <= numeric_limits<T>::digits - 1.
    const auto mantissa_digits = std::numeric_limits<F>::digits;
    const auto usable_digits = mantissa_digits <= (std::numeric_limits<T>::digits - 1)
                                 ? mantissa_digits
                                 : (std::numeric_limits<T>::digits - 1);

    numerator_ = static_cast<T>(std::ldexp(mantissa, usable_digits)); // round(mantissa * 2^usable_digits)
    denominator_ = static_cast<T>(T{1} << usable_digits);             // 2^usable_digits

    if (exponent > 0)
    {
      numerator_ <<= exponent;
    }
    else
    {
      denominator_ <<= -exponent;
    }

    normalize();
  }
  // NOLINTEND(google-explicit-constructor, hicpp-explicit-conversions)

  constexpr T numerator() const noexcept { return numerator_; }
  constexpr T denominator() const noexcept { return denominator_; }

  template <typename F, std::enable_if_t<std::is_floating_point_v<F>, ArithmeticType> = ArithmeticType::FLOATING_POINT>
  constexpr explicit operator F() const noexcept
  {
    assert(denominator_ != 0 && "Denominator must not be zero");
    return static_cast<F>(numerator_) / static_cast<F>(denominator_);
  }

  constexpr Rational& operator+=(const Rational other) noexcept
  {
    // a/b + c/d = (a*d + b*c) / (b*d)
    // Compute in WideT to avoid intermediate overflow.
    const auto wide_num = static_cast<WideT>(numerator_) * static_cast<WideT>(other.denominator_)
                        + static_cast<WideT>(other.numerator_) * static_cast<WideT>(denominator_);
    const auto wide_den = static_cast<WideT>(denominator_) * static_cast<WideT>(other.denominator_);
    normalize_wide(wide_num, wide_den);
    return *this;
  }

  constexpr Rational& operator-=(const Rational other) noexcept
  {
    // a/b - c/d = (a*d - b*c) / (b*d)
    const auto wide_num = static_cast<WideT>(numerator_) * static_cast<WideT>(other.denominator_)
                        - static_cast<WideT>(other.numerator_) * static_cast<WideT>(denominator_);
    const auto wide_den = static_cast<WideT>(denominator_) * static_cast<WideT>(other.denominator_);
    normalize_wide(wide_num, wide_den);
    return *this;
  }

  constexpr Rational& operator*=(const Rational other) noexcept
  {
    // a/b * c/d = (a*c) / (b*d)
    const auto wide_num = static_cast<WideT>(numerator_) * static_cast<WideT>(other.numerator_);
    const auto wide_den = static_cast<WideT>(denominator_) * static_cast<WideT>(other.denominator_);
    normalize_wide(wide_num, wide_den);
    return *this;
  }

  constexpr Rational& operator/=(const Rational other) noexcept
  {
    assert(other.numerator_ != 0 && "Division by zero: other.numerator must not be zero");
    // a/b / c/d = (a*d) / (b*c)
    const auto wide_num = static_cast<WideT>(numerator_) * static_cast<WideT>(other.denominator_);
    const auto wide_den = static_cast<WideT>(denominator_) * static_cast<WideT>(other.numerator_);
    normalize_wide(wide_num, wide_den);
    return *this;
  }

  constexpr Rational& operator++() noexcept
  {
    // (n + d) / d — GCD unchanged (gcd(n+d, d) = gcd(n, d) = 1), just check bounds.
    const auto wide_num = static_cast<WideT>(numerator_) + static_cast<WideT>(denominator_);
    assert(wide_num >= static_cast<WideT>(std::numeric_limits<T>::min())
           && wide_num <= static_cast<WideT>(std::numeric_limits<T>::max())
           && "Rational overflow: increment result exceeds representable range");
    numerator_ = static_cast<T>(wide_num);
    return *this;
  }

  constexpr Rational operator++(int) noexcept
  {
    const auto result = *this;
    ++(*this);
    return result;
  }

  constexpr Rational& operator--() noexcept
  {
    // (n - d) / d — GCD unchanged, just check bounds.
    const auto wide_num = static_cast<WideT>(numerator_) - static_cast<WideT>(denominator_);
    assert(wide_num >= static_cast<WideT>(std::numeric_limits<T>::min())
           && wide_num <= static_cast<WideT>(std::numeric_limits<T>::max())
           && "Rational overflow: decrement result exceeds representable range");
    numerator_ = static_cast<T>(wide_num);
    return *this;
  }

  constexpr Rational operator--(int) noexcept
  {
    const auto result = *this;
    --(*this);
    return result;
  }

  constexpr Rational operator+() const noexcept { return *this; }
  /// @pre Result must be representable (asserts if numerator==MIN and denominator is odd).
  constexpr Rational operator-() const noexcept
  {
    // Negate via denominator to avoid -MIN overflow UB on numerator.
    // normalize() handles the negative denominator safely.
    return Rational{numerator_, -denominator_};
  }

  /// Barton-Nackman trick to generate operators.
  /// @{
  friend constexpr Rational operator+(Rational lhs, const Rational rhs) noexcept { return lhs += rhs; }
  friend constexpr Rational operator-(Rational lhs, const Rational rhs) noexcept { return lhs -= rhs; }
  friend constexpr Rational operator*(Rational lhs, const Rational rhs) noexcept { return lhs *= rhs; }
  friend constexpr Rational operator/(Rational lhs, const Rational rhs) noexcept { return lhs /= rhs; }
  /// @}

  /// Comparison operators.
  /// Cross-multiplication uses a wider type to avoid overflow.
  /// @{
  friend constexpr bool operator==(const Rational lhs, const Rational rhs) noexcept
  {
    return (lhs.numerator_ == rhs.numerator_) && (lhs.denominator_ == rhs.denominator_);
  }
  friend constexpr bool operator!=(const Rational lhs, const Rational rhs) noexcept { return !(lhs == rhs); }
  friend constexpr bool operator<(const Rational lhs, const Rational rhs) noexcept
  {
    // Use a wider type for cross-multiplication to avoid overflow.
    return (static_cast<WideT>(lhs.numerator_) * static_cast<WideT>(rhs.denominator_))
         < (static_cast<WideT>(rhs.numerator_) * static_cast<WideT>(lhs.denominator_));
  }
  friend constexpr bool operator>(const Rational lhs, const Rational rhs) noexcept { return rhs < lhs; }
  friend constexpr bool operator<=(const Rational lhs, const Rational rhs) noexcept { return !(lhs > rhs); }
  friend constexpr bool operator>=(const Rational lhs, const Rational rhs) noexcept { return !(lhs < rhs); }
  /// @}

private:
  /// Wider integer type for intermediate arithmetic (avoids overflow in cross-products).
  using WideT = std::conditional_t<(sizeof(T) <= 4), std::int64_t, Int<std::int64_t>>;

  /// @brief Normalizes a wide numerator/denominator and stores back into T members.
  /// Performs GCD reduction in WideT (no MIN edge cases since values are far from WideT limits),
  /// then asserts the result fits in T's representable range.
  constexpr void normalize_wide(WideT wide_num, WideT wide_den) noexcept
  {
    assert(wide_den != WideT{0} && "Denominator must not be zero");

    // Make denominator positive.
    {
      const auto sign = math::sign(wide_den);
      wide_num *= sign;
      wide_den *= sign;
    }

    // Zero numerator: result is 0/1.
    if (wide_num == WideT{0})
    {
      numerator_ = T{0};
      denominator_ = T{1};
      return;
    }

    // GCD reduction (simple Euclid — no MIN concerns in WideT).
    auto a = math::sign(wide_num) * wide_num;
    auto b = wide_den;
    while (b != WideT{0})
    {
      const auto rem = a % b;
      a = b;
      b = rem;
    }
    // a is now gcd(|wide_num|, wide_den)
    wide_num /= a;
    wide_den /= a;

    // Assert the normalized result fits in T.
    assert(wide_num >= static_cast<WideT>(std::numeric_limits<T>::min())
           && wide_num <= static_cast<WideT>(std::numeric_limits<T>::max())
           && "Rational overflow: numerator exceeds representable range after normalization");
    assert(wide_den >= WideT{1} && wide_den <= static_cast<WideT>(std::numeric_limits<T>::max())
           && "Rational overflow: denominator exceeds representable range after normalization");

    numerator_ = static_cast<T>(wide_num);
    denominator_ = static_cast<T>(wide_den);
  }

  constexpr void normalize() noexcept
  {
    assert(denominator_ != 0 && "Denominator must not be zero");

    // Make denominator positive (canonical form).
    if (denominator_ < 0)
    {
      reduce_min_for_negation();
      numerator_ = -numerator_;
      denominator_ = -denominator_;
    }

    if (numerator_ == 0)
    {
      denominator_ = 1;
      return;
    }

    // Handle MIN numerator: math::abs(MIN) saturates to MAX (off by 1), making gcd incorrect.
    // Since MIN = -2^(N-1) is a power of 2, reduce by dividing out common factors of 2.
    if (numerator_ == std::numeric_limits<T>::min())
    {
      reduce_min_numerator();
      if (numerator_ == std::numeric_limits<T>::min())
      {
        return; // Denominator is odd: gcd(2^(N-1), odd) = 1, already normalized.
      }
      // Fall through to regular GCD.
    }

    const T gcd_value = gcd(math::abs(numerator_), denominator_);
    numerator_ /= gcd_value;
    denominator_ /= gcd_value;
  }

  /// @brief Divides out common factors of 2 when denominator is negative and either value is MIN.
  /// After this call, neither numerator nor denominator is MIN (or asserts).
  constexpr void reduce_min_for_negation() noexcept
  {
    if (numerator_ != std::numeric_limits<T>::min() && denominator_ != std::numeric_limits<T>::min())
    {
      return;
    }

    // MIN is always even, so at least one factor of 2 can be removed per iteration.
    while ((numerator_ == std::numeric_limits<T>::min() || denominator_ == std::numeric_limits<T>::min())
           && ((numerator_ % T{2}) == T{0}) && ((denominator_ % T{2}) == T{0}))
    {
      numerator_ /= T{2};
      denominator_ /= T{2};
    }

    assert(numerator_ != std::numeric_limits<T>::min()
           && "Rational overflow: cannot negate MIN numerator (result unrepresentable)");
    assert(denominator_ != std::numeric_limits<T>::min()
           && "Rational overflow: cannot negate MIN denominator (result unrepresentable)");
  }

  /// @brief Divides out common factors of 2 from a MIN numerator.
  /// Since MIN = -2^(N-1), this reduces numerator by halving while denominator is even.
  constexpr void reduce_min_numerator() noexcept
  {
    while ((denominator_ % T{2}) == T{0})
    {
      numerator_ /= T{2};
      denominator_ /= T{2};
    }
  }

  /// @brief Computes the greatest common divisor using Euclid's algorithm.
  /// @param[in] a First non-negative value.
  /// @param[in] b Second non-negative value.
  /// @return gcd(a, b).
  constexpr static T gcd(T a, T b) noexcept
  {
    while (b != 0)
    {
      const T rem = a % b;
      a = b;
      b = rem;
    }
    return a;
  }

  T numerator_{0};
  T denominator_{1};
};

using Rational32 = Rational<std::int32_t>;
using Rational64 = Rational<std::int64_t>;

} // namespace rtw::multiprecision
