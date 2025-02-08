# this file is used to generate the coefficients for the Chebyshev and Taylor series approximations of sin(x) and cos(x) on the interval [-pi/2, pi/2]
# the coefficients are then printed to the console for use in the C++ implementation
# the Chebyshev and Taylor series approximations are plotted along with the absolute and relative errors
# NOTE: the file is not bazelized, so numpy and matplotlib must be installed to run the file

import numpy as np
import matplotlib.pyplot as plt
from numpy.polynomial.chebyshev import chebfit, chebval

def factorial(n):
    n_fact = 1
    for i in range(1, n + 1):
        n_fact *= i
    return n_fact

def taylor_sin_coeff(n):
    return [(-1) ** i / factorial(2 * i + 1) for i in range(n)]

def taylor_cos_coeff(n):
    return [(-1) ** i / factorial(2 * i) for i in range(n)]

# Define Taylor series expansions for sine and cosine
def taylor_sin(x, coeffs):
    result = np.zeros_like(x)
    for i in range(len(coeffs)):
        result += coeffs[i] * x ** (2 * i + 1)
    return result

def taylor_cos(x, coeffs):
    result = np.zeros_like(x)
    for i in range(len(coeffs)):
        result += coeffs[i] * x ** (2 * i)
    return result

# Define the interval [-pi/2, pi/2]
x_min, x_max = -np.pi / 2, np.pi / 2
x_vals = np.linspace(x_min, x_max, 1000)

# Normalize x-values to the Chebyshev domain [-1,1]
x_cheb = (2 * x_vals - (x_min + x_max)) / (x_max - x_min)

# Compute function values
sin_vals = np.sin(x_vals)
cos_vals = np.cos(x_vals)

# Define the degree of the Chebyshev approximation, the degree of 5 gives a good approximation
cheb_deg_sin = 5
cheb_deg_cos = 5

# Compute Chebyshev coefficients
cheb_coeffs_sin = chebfit(x_cheb, sin_vals, cheb_deg_sin)
cheb_coeffs_cos = chebfit(x_cheb, cos_vals, cheb_deg_cos)

# Define the degree of the Taylor series approximation
taylor_deg_sin = 5
taylor_deg_cos = 5

# Compute Taylor series approximations
taylor_coeff_sin = taylor_sin_coeff(taylor_deg_sin)
taylor_coeff_cos = taylor_cos_coeff(taylor_deg_cos)
sin_taylor_approx = taylor_sin(x_vals, taylor_coeff_sin)
cos_taylor_approx = taylor_cos(x_vals, taylor_coeff_cos)

np.set_printoptions(precision=8, floatmode="fixed")

# Print coefficients for use in C++
print("\nChebyshev Coefficients for sin(x) on [-pi/2, pi/2]:")
print([float(coeff) for coeff in cheb_coeffs_sin])

print("\nChebyshev Coefficients for cos(x) on [-pi/2, pi/2]:")
print([float(coeff) for coeff in cheb_coeffs_cos])

print("\nTaylor Coefficients for sin(x) on [-pi/2, pi/2]:")
print([float(coeff) for coeff in taylor_coeff_sin])

print("\nTaylor Coefficients for cos(x) on [-pi/2, pi/2]:")
print([float(coeff) for coeff in taylor_coeff_cos])

# Evaluate Chebyshev approximations
sin_cheb_approx = chebval(x_cheb, cheb_coeffs_sin)
cos_cheb_approx = chebval(x_cheb, cheb_coeffs_cos)

# Compute errors
abs_err_sin_cheb = np.abs(sin_cheb_approx - sin_vals)
rel_err_sin_cheb = 1 - np.abs(sin_cheb_approx / sin_vals)

abs_err_cos_cheb = np.abs(cos_cheb_approx - cos_vals)
rel_err_cos_cheb = 1 - np.abs(cos_cheb_approx / cos_vals)

abs_err_sin_taylor = np.abs(sin_taylor_approx - sin_vals)
rel_err_sin_taylor = 1 - np.abs(sin_taylor_approx / sin_vals)

abs_err_cos_taylor = np.abs(cos_taylor_approx - cos_vals)
rel_err_cos_taylor = 1 - np.abs(cos_taylor_approx / cos_vals)

abs_err_cheb_taylor_sin = np.abs(sin_cheb_approx - sin_taylor_approx)
rel_err_cheb_taylor_sin = 1 - np.abs(sin_cheb_approx / sin_taylor_approx)

abs_err_cheb_taylor_cos = np.abs(cos_cheb_approx - cos_taylor_approx)
rel_err_cheb_taylor_cos = 1 - np.abs(cos_cheb_approx / cos_taylor_approx)

# Print errors
print("\nChebyshev Approximation Errors for sin(x):")
print("Absolute Error: ", np.max(abs_err_sin_cheb))
print("Relative Error: ", np.max(rel_err_sin_cheb))

print("\nChebyshev Approximation Errors for cos(x):")
print("Absolute Error: ", np.max(abs_err_cos_cheb))
print("Relative Error: ", np.max(rel_err_cos_cheb))

print("\nTaylor Approximation Errors for sin(x):")
print("Absolute Error: ", np.max(abs_err_sin_taylor))
print("Relative Error: ", np.max(rel_err_sin_taylor))

print("\nTaylor Approximation Errors for cos(x):")
print("Absolute Error: ", np.max(abs_err_cos_taylor))
print("Relative Error: ", np.max(rel_err_cos_taylor))

print("\nChebyshev vs. Taylor Approximation Errors for sin(x):")
print("Absolute Error: ", np.max(abs_err_cheb_taylor_sin))
print("Relative Error: ", np.max(rel_err_cheb_taylor_sin))

# Plot results
plt.figure(figsize=(10, 5))

# Plot sin(x)
plt.subplot(1, 2, 1)
plt.plot(x_vals, sin_vals, label="sin(x)", color="blue")
plt.plot(x_vals, sin_cheb_approx, label="Chebyshev Approx", linestyle="dashed", color="red")
plt.plot(x_vals, sin_taylor_approx, label="Taylor Approx", linestyle="dotted", color="purple")
plt.xlabel("x (radians)")
plt.ylabel("sin(x)")
plt.legend()
plt.title("Chebyshev/Taylor Approximation of sin(x)")

# Plot cos(x)
plt.subplot(1, 2, 2)
plt.plot(x_vals, cos_vals, label="cos(x)",  color="green")
plt.plot(x_vals, cos_cheb_approx, label="Chebyshev Approx", linestyle="dashed",color="orange")
plt.plot(x_vals, cos_taylor_approx, label="Taylor Approx", linestyle="dotted", color="purple")
plt.xlabel("x (radians)")
plt.ylabel("cos(x)")
plt.legend()
plt.title("Chebyshev/Taylor Approximation of cos(x)")

plt.tight_layout()

# Plot absolute errors
plt.figure(figsize=(10, 5))

# Plot sin(x) errors
plt.subplot(1, 2, 1)
plt.plot(x_vals, abs_err_sin_cheb, label="Chebyshev Approx vs. sin(x)", linestyle="dashed", color="red")
plt.plot(x_vals, abs_err_sin_taylor, label="Taylor Approx vs. sine(x)", linestyle="dotted", color="purple")
plt.plot(x_vals, abs_err_cheb_taylor_sin, label="Chebyshev vs. Taylor", linestyle="dashdot", color="green")
plt.xlabel("x (radians)")
plt.ylabel("Absolute Error")
plt.legend()
plt.title("Absolute Error of sin(x) Approximations")

# Plot cos(x) errors
plt.subplot(1, 2, 2)
plt.plot(x_vals, abs_err_cos_cheb, label="Chebyshev Approx vs. cos(x)", linestyle="dashed", color="orange")
plt.plot(x_vals, abs_err_cos_taylor, label="Taylor Approx vs. sine(x)", linestyle="dotted", color="purple")
plt.plot(x_vals, abs_err_cheb_taylor_cos, label="Chebyshev vs. Taylor", linestyle="dashdot", color="green")
plt.xlabel("x (radians)")
plt.ylabel("Absolute Error")
plt.legend()
plt.title("Absolute Error of cos(x) Approximations")

plt.tight_layout()

# Plot relative errors
plt.figure(figsize=(10, 5))

# Plot sin(x) Relative Errors
plt.subplot(1, 2, 1)
plt.plot(x_vals, rel_err_sin_cheb, label="Chebyshev Approx vs. sin(x)", linestyle="dashed", color="red")
plt.plot(x_vals, rel_err_sin_taylor, label="Taylor Approx vs. sin(x)", linestyle="dotted", color="purple")
plt.plot(x_vals, rel_err_cheb_taylor_sin, label="Chebyshev vs. Taylor", linestyle="dashdot", color="green")
plt.xlabel("x (radians)")
plt.ylabel("Relative Error")
plt.legend()
plt.title("Relative Error of sin(x) Approximations")

# Plot cos(x) Relative Errors
plt.subplot(1, 2, 2)
plt.plot(x_vals, rel_err_cos_cheb, label="Chebyshev Approx vs. cos(x)", linestyle="dashed", color="orange")
plt.plot(x_vals, rel_err_cos_taylor, label="Taylor Approx vs. cos(x)", linestyle="dotted", color="purple")
plt.plot(x_vals, rel_err_cheb_taylor_cos, label="Chebyshev vs. Taylor", linestyle="dashdot", color="green")
plt.xlabel("x (radians)")
plt.ylabel("Relative Error")
plt.legend()
plt.title("Relative Error of cos(x) Approximations")

plt.tight_layout()
plt.show()
