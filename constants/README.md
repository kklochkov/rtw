# rtw/constants -- Compile-Time Constants

Header-only module providing mathematical, time, and rendering constants for the rtw project.

## Headers

| Header | Bazel Target | Purpose |
|--------|--------------|---------|
| `math_constants.h` | `//constants:math_constants` | Universal mathematical constants as `constexpr inline` variable templates |
| `time_constants.h` | `//constants:time_constants` | `std::chrono::duration` type aliases for float-based time |
| `render_constants.h` | `//constants:render_constants` | Frame rate, frame time, and spatial conversion factors |

## Math Constants

All math constants are variable templates parameterized on the scalar type `T`, allowing use with `float`, `double`, `FixedPoint`, or any type constructible from a `double` literal:

```cpp
#include "constants/math_constants.h"

constexpr float pi_f = rtw::math_constants::PI<float>;
constexpr double pi_d = rtw::math_constants::PI<double>;
constexpr auto deg = rtw::math_constants::RAD_TO_DEG<double> * 1.5708; // ~90
```

Available constants:

| Constant | Value | Description |
|----------|-------|-------------|
| `PI<T>` | 3.14159... | Ratio of circumference to diameter |
| `TAU<T>` | 6.28318... | Full turn (2 * Pi) |
| `TWO_PI<T>` | 6.28318... | Alias for TAU |
| `PI_2<T>` | 1.57079... | Half Pi (quarter turn) |
| `PI_4<T>` | 0.78539... | Quarter Pi (eighth turn) |
| `DEG_TO_RAD<T>` | 0.01745... | Multiply degrees to get radians |
| `RAD_TO_DEG<T>` | 57.2957... | Multiply radians to get degrees |
| `E<T>` | 2.71828... | Euler's number |
| `SQRT_2<T>` | 1.41421... | Square root of 2 |
| `SQRT_3<T>` | 1.73205... | Square root of 3 |
| `LN2<T>` | 0.69314... | Natural logarithm of 2 |
| `LN10<T>` | 2.30258... | Natural logarithm of 10 |
| `LOG2_E<T>` | 1.44269... | Base-2 logarithm of e (= 1/ln(2)) |
| `LOG10_2<T>` | 0.30102... | Common logarithm of 2 |
| `LOG10_E<T>` | 0.43429... | Common logarithm of e (= 1/ln(10)) |

## Compile-Time Proofs

`math_constants.h` includes `static_assert` proofs verifying:
- `TAU == 2*PI`, `TWO_PI == TAU`, `PI_2 == PI/2`, `PI_4 == PI/4`
- `DEG_TO_RAD * RAD_TO_DEG == 1` (both float and double)
- `LOG2_E * LN2 ~ 1`, `LOG10_E * LN10 ~ 1`

`render_constants.h` asserts all values are positive.

## Usage (Bazel)

```python
cc_binary(
    name = "my_target",
    deps = [
        "//constants:math_constants",
        "//constants:time_constants",
        "//constants:render_constants",
    ],
)
```
