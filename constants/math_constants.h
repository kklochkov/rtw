#pragma once

namespace rtw::math_constants
{

template <typename T>
constexpr T PI{3.141592653589793238462643383279502884197169399375105820974944592307816406286};

template <typename T>
constexpr T TAU = T{2} * PI<T>;

template <typename T>
constexpr T TWO_PI = TAU<T>;

template <typename T>
constexpr T PI_2 = PI<T> / T{2};

template <typename T>
constexpr T PI_4 = PI<T> / T{4};

template <typename T>
constexpr T DEG_TO_RAD = PI<T> / T{180};

template <typename T>
constexpr T RAD_TO_DEG = T{180} / PI<T>;

template <typename T>
constexpr T E = T{2.718281828459045235360287471352662497757247093699959574966967627724076630353};

template <typename T>
constexpr T SQRT_2 = T{1.414213562373095048801688724209698078569671875376948073176679737990732478462};

template <typename T>
constexpr T SQRT_3 = T{1.732050807568877293527446341505872366942805253810380628055806979451933016908};

template <typename T>
constexpr T LOG10_2 = T{0.301029995663981195213738894724493026768189881462108541310};

} // namespace rtw::math_constants
