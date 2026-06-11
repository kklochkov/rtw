#pragma once

namespace rtw::math_constants
{

/// Pi (ratio of circumference to diameter). Precision: 75 significant digits.
template <typename T>
constexpr inline T PI{3.141592653589793238462643383279502884197169399375105820974944592307816406286};

/// Tau (2 * Pi). Full turn in radians.
template <typename T>
constexpr inline T TAU = T{2} * PI<T>;

/// Alias for TAU (2 * Pi).
template <typename T>
constexpr inline T TWO_PI = TAU<T>;

/// Pi / 2. Quarter turn in radians.
template <typename T>
constexpr inline T PI_2 = PI<T> / T{2};

/// Pi / 4. Eighth turn in radians.
template <typename T>
constexpr inline T PI_4 = PI<T> / T{4};

/// Degrees-to-radians conversion factor (Pi / 180).
template <typename T>
constexpr inline T DEG_TO_RAD = PI<T> / T{180};

/// Radians-to-degrees conversion factor (180 / Pi).
template <typename T>
constexpr inline T RAD_TO_DEG = T{180} / PI<T>;

/// Euler's number (base of natural logarithm). Precision: 75 significant digits.
template <typename T>
constexpr inline T E = T{2.718281828459045235360287471352662497757247093699959574966967627724076630353};

/// Square root of 2.
template <typename T>
constexpr inline T SQRT_2 = T{1.414213562373095048801688724209698078569671875376948073176679737990732478462};

/// Square root of 3.
template <typename T>
constexpr inline T SQRT_3 = T{1.732050807568877293527446341505872366942805253810380628055806979451933016908};

/// Common logarithm of 2 (log10(2)).
template <typename T>
constexpr inline T LOG10_2 = T{0.301029995663981195213738894724493026768189881462108541310};

/// Natural logarithm of 2 (ln(2)).
template <typename T>
constexpr inline T LN2 = T{0.693147180559945309417232121458176568075500134360255254120};

/// Natural logarithm of 10 (ln(10)).
template <typename T>
constexpr inline T LN10 = T{2.302585092994045684017991454684364207601101488628772976033};

/// Base-2 logarithm of e (log2(e) = 1/ln(2)).
template <typename T>
constexpr inline T LOG2_E = T{1.442695040888963407359924681001892137426645954152985934135};

/// Common logarithm of e (log10(e) = 1/ln(10)).
template <typename T>
constexpr inline T LOG10_E = T{0.434294481903251827651128918916605082294397005803666566114};

// -- Compile-time relationship proofs --

static_assert(TAU<double> == 2.0 * PI<double>, "TAU must equal 2*PI");
static_assert(TWO_PI<double> == TAU<double>, "TWO_PI must equal TAU");
static_assert(PI_2<double> == PI<double> / 2.0, "PI_2 must equal PI/2");
static_assert(PI_4<double> == PI<double> / 4.0, "PI_4 must equal PI/4");
static_assert(DEG_TO_RAD<double> == PI<double> / 180.0, "DEG_TO_RAD must equal PI/180");
static_assert(RAD_TO_DEG<double> == 180.0 / PI<double>, "RAD_TO_DEG must equal 180/PI");

// Verify inverse relationships hold within floating-point representation.
static_assert(DEG_TO_RAD<double> * RAD_TO_DEG<double> == 1.0,
              "DEG_TO_RAD * RAD_TO_DEG must equal 1 (double precision)");
static_assert(DEG_TO_RAD<float> * RAD_TO_DEG<float> > 0.999999F && DEG_TO_RAD<float> * RAD_TO_DEG<float> < 1.000001F,
              "DEG_TO_RAD * RAD_TO_DEG must be approximately 1 (single precision)");

// Verify logarithmic inverses.
static_assert(LOG2_E<double> * LN2<double> > 0.9999999999 && LOG2_E<double> * LN2<double> < 1.0000000001,
              "LOG2_E * LN2 must be approximately 1");
static_assert(LOG10_E<double> * LN10<double> > 0.9999999999 && LOG10_E<double> * LN10<double> < 1.0000000001,
              "LOG10_E * LN10 must be approximately 1");

} // namespace rtw::math_constants
