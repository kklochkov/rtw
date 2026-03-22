#include "math/quaternion.h"
#include "math/quaternion_operations.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cmath>

namespace
{
constexpr auto EPSILON = 1e-7;

using QuaternionD = rtw::math::Quaternion<double>;
using Vector3D = rtw::math::Vector3<double>;
using Matrix3x3D = rtw::math::Matrix3x3<double>;
using AngleD = rtw::math::Angle<double>;

void expect_quaternion_near(const QuaternionD& actual, const double w, const double x, const double y, const double z,
                            const double epsilon = EPSILON)
{
  EXPECT_NEAR(actual.w(), w, epsilon);
  EXPECT_NEAR(actual.x(), x, epsilon);
  EXPECT_NEAR(actual.y(), y, epsilon);
  EXPECT_NEAR(actual.z(), z, epsilon);
}

void expect_quaternion_equivalent(const QuaternionD& q1, const QuaternionD& q2, const double epsilon = EPSILON)
{
  const bool same_sign = std::abs(q1.w() - q2.w()) < epsilon && std::abs(q1.x() - q2.x()) < epsilon
                      && std::abs(q1.y() - q2.y()) < epsilon && std::abs(q1.z() - q2.z()) < epsilon;
  const bool opposite_sign = std::abs(q1.w() + q2.w()) < epsilon && std::abs(q1.x() + q2.x()) < epsilon
                          && std::abs(q1.y() + q2.y()) < epsilon && std::abs(q1.z() + q2.z()) < epsilon;
  EXPECT_TRUE(same_sign || opposite_sign);
}

} // namespace

TEST(Quaternion, default_ctor)
{
  constexpr QuaternionD Q;
  expect_quaternion_near(Q, 0.0, 0.0, 0.0, 0.0);
}

TEST(Quaternion, identity_ctor)
{
  constexpr QuaternionD Q{rtw::math::IDENTITY};
  expect_quaternion_near(Q, 1.0, 0.0, 0.0, 0.0);
}

TEST(Quaternion, zero_ctor)
{
  constexpr QuaternionD Q{rtw::math::ZERO};
  expect_quaternion_near(Q, 0.0, 0.0, 0.0, 0.0);
}

TEST(Quaternion, value_ctor)
{
  constexpr QuaternionD Q{1.0, 2.0, 3.0, 4.0}; // w, x, y, z
  expect_quaternion_near(Q, 1.0, 2.0, 3.0, 4.0);
}

TEST(Quaternion, initialize_with_value_ctor)
{
  constexpr QuaternionD Q{rtw::math::INITIALIZE_WITH_VALUE, 5.0};
  expect_quaternion_near(Q, 5.0, 5.0, 5.0, 5.0);
}

TEST(Quaternion, conjugate_identity)
{
  constexpr QuaternionD Q{rtw::math::IDENTITY};
  const auto conj = Q.conjugate();
  expect_quaternion_near(conj, 1.0, 0.0, 0.0, 0.0);
}

TEST(Quaternion, conjugate_general)
{
  constexpr QuaternionD Q{1.0, 2.0, 3.0, 4.0};
  const auto conj = Q.conjugate();
  expect_quaternion_near(conj, 1.0, -2.0, -3.0, -4.0);
}

TEST(Quaternion, conjugate_double_returns_original)
{
  constexpr QuaternionD Q{1.0, 2.0, 3.0, 4.0};
  const auto double_conj = Q.conjugate().conjugate();
  expect_quaternion_near(double_conj, Q.w(), Q.x(), Q.y(), Q.z());
}

TEST(Quaternion, angle_identity)
{
  constexpr QuaternionD Q{rtw::math::IDENTITY};
  EXPECT_NEAR(Q.angle().rad(), 0.0, EPSILON);
}

TEST(Quaternion, angle_90_degrees_about_z)
{
  const auto q = QuaternionD::from_axis_angle(Vector3D{0.0, 0.0, 1.0}, rtw::math::PI_2<double>);
  EXPECT_NEAR(q.angle().rad(), rtw::math::PI_2<double>.rad(), EPSILON);
}

TEST(Quaternion, angle_180_degrees)
{
  const auto q = QuaternionD::from_axis_angle(Vector3D{1.0, 0.0, 0.0}, rtw::math::PI<double>);
  EXPECT_NEAR(q.angle().rad(), rtw::math::PI<double>.rad(), EPSILON);
}

TEST(Quaternion, from_axis_angle_zero_angle)
{
  const auto q = QuaternionD::from_axis_angle(Vector3D{1.0, 0.0, 0.0}, AngleD{0.0});
  expect_quaternion_near(q, 1.0, 0.0, 0.0, 0.0);
}

TEST(Quaternion, from_axis_angle_90_degrees_x)
{
  const auto q = QuaternionD::from_axis_angle(Vector3D{1.0, 0.0, 0.0}, rtw::math::PI_2<double>);
  const double half = rtw::math::PI_2<double>.rad() / 2.0;
  expect_quaternion_near(q, std::cos(half), std::sin(half), 0.0, 0.0);
}

TEST(Quaternion, from_axis_angle_90_degrees_y)
{
  const auto q = QuaternionD::from_axis_angle(Vector3D{0.0, 1.0, 0.0}, rtw::math::PI_2<double>);
  const double half = rtw::math::PI_2<double>.rad() / 2.0;
  expect_quaternion_near(q, std::cos(half), 0.0, std::sin(half), 0.0);
}

TEST(Quaternion, from_axis_angle_90_degrees_z)
{
  const auto q = QuaternionD::from_axis_angle(Vector3D{0.0, 0.0, 1.0}, rtw::math::PI_2<double>);
  const double half = rtw::math::PI_2<double>.rad() / 2.0;
  expect_quaternion_near(q, std::cos(half), 0.0, 0.0, std::sin(half));
}

TEST(Quaternion, from_axis_angle_180_degrees)
{
  const auto q = QuaternionD::from_axis_angle(Vector3D{1.0, 0.0, 0.0}, rtw::math::PI<double>);
  expect_quaternion_near(q, 0.0, 1.0, 0.0, 0.0);
}

TEST(Quaternion, axis_recovers_original_axis)
{
  const Vector3D original_axis{0.0, 0.0, 1.0};
  const auto q = QuaternionD::from_axis_angle(original_axis, AngleD{1.0}); // ~57 degrees
  const auto recovered_axis = rtw::math::axis(q);

  EXPECT_NEAR(recovered_axis.x(), original_axis.x(), EPSILON);
  EXPECT_NEAR(recovered_axis.y(), original_axis.y(), EPSILON);
  EXPECT_NEAR(recovered_axis.z(), original_axis.z(), EPSILON);
}

TEST(Quaternion, from_euler_angles_zero)
{
  const auto q = QuaternionD::from_euler_angles(AngleD{0.0}, AngleD{0.0}, AngleD{0.0}); // roll, pitch, yaw
  expect_quaternion_near(q, 1.0, 0.0, 0.0, 0.0);
}

TEST(Quaternion, from_euler_angles_roll_only)
{
  const auto q = QuaternionD::from_euler_angles(rtw::math::PI_2<double>, AngleD{0.0}, AngleD{0.0});
  // Roll about X axis
  const auto expected = QuaternionD::from_axis_angle(Vector3D{1.0, 0.0, 0.0}, rtw::math::PI_2<double>);
  expect_quaternion_equivalent(q, expected);
}

TEST(Quaternion, from_euler_angles_pitch_only)
{
  const auto q = QuaternionD::from_euler_angles(AngleD{0.0}, rtw::math::PI_2<double>, AngleD{0.0});
  // Pitch about Y axis
  const auto expected = QuaternionD::from_axis_angle(Vector3D{0.0, 1.0, 0.0}, rtw::math::PI_2<double>);
  expect_quaternion_equivalent(q, expected);
}

TEST(Quaternion, from_euler_angles_yaw_only)
{
  const auto q = QuaternionD::from_euler_angles(AngleD{0.0}, AngleD{0.0}, rtw::math::PI_2<double>);
  // Yaw about Z axis
  const auto expected = QuaternionD::from_axis_angle(Vector3D{0.0, 0.0, 1.0}, rtw::math::PI_2<double>);
  expect_quaternion_equivalent(q, expected);
}

TEST(Quaternion, euler_angles_roundtrip)
{
  const AngleD roll{0.3};
  const AngleD pitch{0.2};
  const AngleD yaw{0.5};

  const auto q = QuaternionD::from_euler_angles(roll, pitch, yaw);
  const auto euler = q.to_euler_angles();

  EXPECT_NEAR(euler.roll.rad(), roll.rad(), EPSILON);
  EXPECT_NEAR(euler.pitch.rad(), pitch.rad(), EPSILON);
  EXPECT_NEAR(euler.yaw.rad(), yaw.rad(), EPSILON);
}

TEST(Quaternion, euler_angles_from_struct)
{
  const rtw::math::EulerAngles<double> euler{AngleD{0.5}, AngleD{0.2}, AngleD{0.3}}; // yaw, pitch, roll
  const auto q = QuaternionD::from_euler_angles(euler);
  const auto recovered = q.to_euler_angles();

  EXPECT_NEAR(recovered.roll.rad(), euler.roll.rad(), EPSILON);
  EXPECT_NEAR(recovered.pitch.rad(), euler.pitch.rad(), EPSILON);
  EXPECT_NEAR(recovered.yaw.rad(), euler.yaw.rad(), EPSILON);
}

TEST(Quaternion, to_rotation_matrix_identity)
{
  constexpr QuaternionD Q{rtw::math::IDENTITY};
  const auto mat = Q.to_rotation_matrix();

  EXPECT_NEAR(mat(0, 0), 1.0, EPSILON);
  EXPECT_NEAR(mat(0, 1), 0.0, EPSILON);
  EXPECT_NEAR(mat(0, 2), 0.0, EPSILON);
  EXPECT_NEAR(mat(1, 0), 0.0, EPSILON);
  EXPECT_NEAR(mat(1, 1), 1.0, EPSILON);
  EXPECT_NEAR(mat(1, 2), 0.0, EPSILON);
  EXPECT_NEAR(mat(2, 0), 0.0, EPSILON);
  EXPECT_NEAR(mat(2, 1), 0.0, EPSILON);
  EXPECT_NEAR(mat(2, 2), 1.0, EPSILON);
}

TEST(Quaternion, to_rotation_matrix_90_about_z)
{
  const auto q = QuaternionD::from_axis_angle(Vector3D{0.0, 0.0, 1.0}, rtw::math::PI_2<double>);
  const auto mat = q.to_rotation_matrix();

  // Rotation 90 degrees about Z: x -> y, y -> -x
  EXPECT_NEAR(mat(0, 0), 0.0, EPSILON);
  EXPECT_NEAR(mat(0, 1), -1.0, EPSILON);
  EXPECT_NEAR(mat(1, 0), 1.0, EPSILON);
  EXPECT_NEAR(mat(1, 1), 0.0, EPSILON);
  EXPECT_NEAR(mat(2, 2), 1.0, EPSILON);
}

TEST(Quaternion, rotation_matrix_roundtrip)
{
  const auto original =
      QuaternionD::from_axis_angle(rtw::math::normalize(Vector3D{1.0, 1.0, 1.0}), AngleD{1.0}); // ~57 degrees
  const auto normalized_original = rtw::math::normalize(original);
  const auto mat = normalized_original.to_rotation_matrix();
  const auto recovered = QuaternionD::from_rotation_matrix(mat);

  expect_quaternion_equivalent(normalized_original, recovered);
}

TEST(Quaternion, from_rotation_matrix_identity)
{
  const Matrix3x3D identity{rtw::math::IDENTITY};
  const auto q = QuaternionD::from_rotation_matrix(identity);
  expect_quaternion_near(q, 1.0, 0.0, 0.0, 0.0);
}

TEST(Quaternion, multiply_identity_left)
{
  constexpr QuaternionD IDENTITY{rtw::math::IDENTITY};
  constexpr QuaternionD Q{1.0, 2.0, 3.0, 4.0};
  const auto result = IDENTITY * Q;
  expect_quaternion_near(result, Q.w(), Q.x(), Q.y(), Q.z());
}

TEST(Quaternion, multiply_identity_right)
{
  constexpr QuaternionD IDENTITY{rtw::math::IDENTITY};
  constexpr QuaternionD Q{1.0, 2.0, 3.0, 4.0};
  const auto result = Q * IDENTITY;
  expect_quaternion_near(result, Q.w(), Q.x(), Q.y(), Q.z());
}

TEST(Quaternion, multiply_inverse_gives_identity)
{
  const auto q = rtw::math::normalize(QuaternionD{1.0, 2.0, 3.0, 4.0});
  const auto q_inv = rtw::math::inverse(q);
  const auto result = q * q_inv;
  expect_quaternion_near(result, 1.0, 0.0, 0.0, 0.0);
}

TEST(Quaternion, multiply_two_90_degree_rotations)
{
  // Two 90-degree rotations about Z should give 180-degree rotation
  const auto q_90 = QuaternionD::from_axis_angle(Vector3D{0.0, 0.0, 1.0}, rtw::math::PI_2<double>);
  const auto result = q_90 * q_90;
  const auto expected = QuaternionD::from_axis_angle(Vector3D{0.0, 0.0, 1.0}, rtw::math::PI<double>);
  expect_quaternion_equivalent(result, expected);
}

TEST(Quaternion, multiply_equals_modifies_in_place)
{
  auto q1 = QuaternionD::from_axis_angle(Vector3D{1.0, 0.0, 0.0}, AngleD{0.5});
  const auto q2 = QuaternionD::from_axis_angle(Vector3D{0.0, 1.0, 0.0}, AngleD{0.5});
  const auto expected = q1 * q2;
  q1 *= q2;
  expect_quaternion_near(q1, expected.w(), expected.x(), expected.y(), expected.z());
}

TEST(Quaternion, multiply_scalar)
{
  constexpr QuaternionD Q{1.0, 2.0, 3.0, 4.0};
  const auto result = Q * 2.0;
  expect_quaternion_near(result, 2.0, 4.0, 6.0, 8.0);
}

TEST(Quaternion, multiply_scalar_left)
{
  constexpr QuaternionD Q{1.0, 2.0, 3.0, 4.0};
  const auto result = 2.0 * Q;
  expect_quaternion_near(result, 2.0, 4.0, 6.0, 8.0);
}

TEST(Quaternion, rotate_vector_identity)
{
  constexpr QuaternionD Q{rtw::math::IDENTITY};
  constexpr Vector3D V{1.0, 2.0, 3.0};
  const auto result = Q * V;
  EXPECT_NEAR(result.x(), V.x(), EPSILON);
  EXPECT_NEAR(result.y(), V.y(), EPSILON);
  EXPECT_NEAR(result.z(), V.z(), EPSILON);
}

TEST(Quaternion, rotate_vector_90_about_z)
{
  const auto q = QuaternionD::from_axis_angle(Vector3D{0.0, 0.0, 1.0}, rtw::math::PI_2<double>);
  const Vector3D v{1.0, 0.0, 0.0}; // X unit vector
  const auto result = q * v;

  // 90 degrees about Z: X -> Y
  EXPECT_NEAR(result.x(), 0.0, EPSILON);
  EXPECT_NEAR(result.y(), 1.0, EPSILON);
  EXPECT_NEAR(result.z(), 0.0, EPSILON);
}

TEST(Quaternion, rotate_vector_180_about_x)
{
  const auto q = QuaternionD::from_axis_angle(Vector3D{1.0, 0.0, 0.0}, rtw::math::PI<double>);
  const Vector3D v{0.0, 1.0, 0.0}; // Y unit vector
  const auto result = q * v;

  // 180 degrees about X: Y -> -Y
  EXPECT_NEAR(result.x(), 0.0, EPSILON);
  EXPECT_NEAR(result.y(), -1.0, EPSILON);
  EXPECT_NEAR(result.z(), 0.0, EPSILON);
}

TEST(Quaternion, rotate_vector_preserves_magnitude)
{
  const auto q = QuaternionD::from_axis_angle(rtw::math::normalize(Vector3D{1.0, 1.0, 1.0}), AngleD{1.234});
  const Vector3D v{3.0, 4.0, 5.0};
  const auto result = q * v;

  const auto original_mag = rtw::math::norm(v);
  const auto result_mag = rtw::math::norm(result);
  EXPECT_NEAR(result_mag, original_mag, EPSILON);
}

TEST(Quaternion, dot_same_quaternion)
{
  constexpr QuaternionD Q{1.0, 2.0, 3.0, 4.0};
  const auto result = rtw::math::dot(Q, Q);
  EXPECT_NEAR(result, 30.0, EPSILON); // 1 + 4 + 9 + 16
}

TEST(Quaternion, dot_identity)
{
  constexpr QuaternionD Q{rtw::math::IDENTITY};
  EXPECT_NEAR(rtw::math::dot(Q, Q), 1.0, EPSILON);
}

TEST(Quaternion, norm2)
{
  constexpr QuaternionD Q{1.0, 2.0, 3.0, 4.0};
  EXPECT_NEAR(rtw::math::norm2(Q), 30.0, EPSILON);
}

TEST(Quaternion, norm)
{
  constexpr QuaternionD Q{1.0, 2.0, 3.0, 4.0};
  EXPECT_NEAR(rtw::math::norm(Q), std::sqrt(30.0), EPSILON);
}

TEST(Quaternion, norm_identity)
{
  constexpr QuaternionD Q{rtw::math::IDENTITY};
  EXPECT_NEAR(rtw::math::norm(Q), 1.0, EPSILON);
}

TEST(Quaternion, normalize)
{
  constexpr QuaternionD Q{1.0, 2.0, 3.0, 4.0};
  const auto normalized = rtw::math::normalize(Q);
  EXPECT_NEAR(rtw::math::norm(normalized), 1.0, EPSILON);

  // Check direction is preserved
  const auto scale = std::sqrt(30.0);
  expect_quaternion_near(normalized, 1.0 / scale, 2.0 / scale, 3.0 / scale, 4.0 / scale);
}

TEST(Quaternion, normalize_zero_death)
{
  constexpr QuaternionD Q{rtw::math::ZERO};
  EXPECT_DEATH(rtw::math::normalize(Q), "");
}

TEST(Quaternion, inverse_identity)
{
  constexpr QuaternionD Q{rtw::math::IDENTITY};
  const auto inv = rtw::math::inverse(Q);
  expect_quaternion_near(inv, 1.0, 0.0, 0.0, 0.0);
}

TEST(Quaternion, inverse_unit_quaternion)
{
  const auto q = rtw::math::normalize(QuaternionD{1.0, 2.0, 3.0, 4.0});
  const auto inv = rtw::math::inverse(q);
  // For unit quaternion, inverse == conjugate
  expect_quaternion_near(inv, q.w(), -q.x(), -q.y(), -q.z());
}

TEST(Quaternion, inverse_non_unit_quaternion)
{
  constexpr QuaternionD Q{1.0, 2.0, 3.0, 4.0};
  const auto inv = rtw::math::inverse(Q);
  const auto result = Q * inv;
  expect_quaternion_near(result, 1.0, 0.0, 0.0, 0.0);
}

TEST(Quaternion, inverse_zero_death)
{
  constexpr QuaternionD Q{rtw::math::ZERO};
  EXPECT_DEATH(rtw::math::inverse(Q), "");
}

TEST(Quaternion, log_identity)
{
  constexpr QuaternionD Q{rtw::math::IDENTITY};
  const auto result = rtw::math::log(Q);
  expect_quaternion_near(result, 0.0, 0.0, 0.0, 0.0);
}

TEST(Quaternion, exp_zero)
{
  constexpr QuaternionD Q{rtw::math::ZERO};
  const auto result = rtw::math::exp(Q);
  expect_quaternion_near(result, 1.0, 0.0, 0.0, 0.0);
}

TEST(Quaternion, log_exp_roundtrip)
{
  const auto q = rtw::math::normalize(QuaternionD{1.0, 0.2, 0.3, 0.4});
  const auto log_q = rtw::math::log(q);
  const auto result = rtw::math::exp(log_q);
  expect_quaternion_equivalent(result, q);
}

TEST(Quaternion, exp_log_roundtrip)
{
  // Pure quaternion (w = 0, small vector part)
  constexpr QuaternionD Q{0.0, 0.1, 0.2, 0.3};
  const auto exp_q = rtw::math::exp(Q);
  const auto result = rtw::math::log(exp_q);
  expect_quaternion_near(result, Q.w(), Q.x(), Q.y(), Q.z());
}

TEST(Quaternion, pow_zero)
{
  const auto q = rtw::math::normalize(QuaternionD{1.0, 2.0, 3.0, 4.0});
  const auto result = rtw::math::pow(q, 0.0);
  expect_quaternion_near(result, 1.0, 0.0, 0.0, 0.0);
}

TEST(Quaternion, pow_one)
{
  const auto q = rtw::math::normalize(QuaternionD{1.0, 2.0, 3.0, 4.0});
  const auto result = rtw::math::pow(q, 1.0);
  expect_quaternion_equivalent(result, q);
}

TEST(Quaternion, pow_two_equals_squared)
{
  const auto q = rtw::math::normalize(QuaternionD::from_axis_angle(Vector3D{0.0, 0.0, 1.0}, AngleD{0.5}));
  const auto pow_result = rtw::math::pow(q, 2.0);
  const auto squared = q * q;
  expect_quaternion_equivalent(pow_result, squared);
}

TEST(Quaternion, pow_half_is_sqrt)
{
  const auto q = QuaternionD::from_axis_angle(Vector3D{0.0, 0.0, 1.0}, rtw::math::PI_2<double>);
  const auto sqrt_q = rtw::math::pow(q, 0.5);
  const auto result = sqrt_q * sqrt_q;
  expect_quaternion_equivalent(result, q);
}

TEST(Quaternion, nlerp_t0)
{
  const auto q1 = rtw::math::normalize(QuaternionD{1.0, 0.0, 0.0, 0.0});
  const auto q2 = QuaternionD::from_axis_angle(Vector3D{0.0, 0.0, 1.0}, rtw::math::PI_2<double>);
  const auto result = rtw::math::nlerp(q1, q2, 0.0);
  expect_quaternion_equivalent(result, q1);
}

TEST(Quaternion, nlerp_t1)
{
  const auto q1 = rtw::math::normalize(QuaternionD{1.0, 0.0, 0.0, 0.0});
  const auto q2 = QuaternionD::from_axis_angle(Vector3D{0.0, 0.0, 1.0}, rtw::math::PI_2<double>);
  const auto result = rtw::math::nlerp(q1, q2, 1.0);
  expect_quaternion_equivalent(result, q2);
}

TEST(Quaternion, nlerp_midpoint)
{
  const auto q1 = QuaternionD{rtw::math::IDENTITY};
  const auto q2 = QuaternionD::from_axis_angle(Vector3D{0.0, 0.0, 1.0}, rtw::math::PI_2<double>);
  const auto result = rtw::math::nlerp(q1, q2, 0.5);

  // Result should be unit quaternion
  EXPECT_NEAR(rtw::math::norm(result), 1.0, EPSILON);
}

TEST(Quaternion, nlerp_shortest_path)
{
  // Test that nlerp takes shortest path when quaternions are opposite
  const auto q1 = QuaternionD{rtw::math::IDENTITY};
  const auto q2 = QuaternionD{-1.0, 0.0, 0.0, 0.0}; // Same rotation as identity but negated

  const auto result = rtw::math::nlerp(q1, q2, 0.5);
  // Should stay near identity since q1 and -q2 represent the same rotation
  EXPECT_NEAR(rtw::math::norm(result), 1.0, EPSILON);
}

TEST(Quaternion, slerp_t0)
{
  const auto q1 = QuaternionD{rtw::math::IDENTITY};
  const auto q2 = QuaternionD::from_axis_angle(Vector3D{0.0, 0.0, 1.0}, rtw::math::PI_2<double>);
  const auto result = rtw::math::slerp(q1, q2, 0.0);
  expect_quaternion_equivalent(result, q1);
}

TEST(Quaternion, slerp_t1)
{
  const auto q1 = QuaternionD{rtw::math::IDENTITY};
  const auto q2 = QuaternionD::from_axis_angle(Vector3D{0.0, 0.0, 1.0}, rtw::math::PI_2<double>);
  const auto result = rtw::math::slerp(q1, q2, 1.0);
  expect_quaternion_equivalent(result, q2);
}

TEST(Quaternion, slerp_midpoint)
{
  const auto q1 = QuaternionD{rtw::math::IDENTITY};
  const auto q2 = QuaternionD::from_axis_angle(Vector3D{0.0, 0.0, 1.0}, rtw::math::PI_2<double>);
  const auto result = rtw::math::slerp(q1, q2, 0.5);

  // Midpoint should be 45-degree rotation (PI/4)
  const auto expected =
      QuaternionD::from_axis_angle(Vector3D{0.0, 0.0, 1.0}, AngleD{rtw::math::PI_2<double>.rad() / 2.0});
  expect_quaternion_equivalent(result, expected);
}

TEST(Quaternion, slerp_constant_speed)
{
  const auto q1 = QuaternionD{rtw::math::IDENTITY};
  const auto q2 = QuaternionD::from_axis_angle(Vector3D{0.0, 0.0, 1.0}, rtw::math::PI<double>);

  // Quarter, half, and three-quarter points
  const auto r25 = rtw::math::slerp(q1, q2, 0.25);
  const auto r50 = rtw::math::slerp(q1, q2, 0.5);
  const auto r75 = rtw::math::slerp(q1, q2, 0.75);

  // Angles should be evenly spaced
  EXPECT_NEAR(r25.angle().rad(), rtw::math::PI<double>.rad() * 0.25, EPSILON);
  EXPECT_NEAR(r50.angle().rad(), rtw::math::PI<double>.rad() * 0.5, EPSILON);
  EXPECT_NEAR(r75.angle().rad(), rtw::math::PI<double>.rad() * 0.75, EPSILON);
}

TEST(Quaternion, slerp_falls_back_to_nlerp_for_close_quaternions)
{
  const auto q1 = QuaternionD{rtw::math::IDENTITY};
  // Very small rotation - slerp should use nlerp fallback
  const auto q2 = QuaternionD::from_axis_angle(Vector3D{0.0, 0.0, 1.0}, AngleD{1e-10});
  const auto result = rtw::math::slerp(q1, q2, 0.5);

  // Should still be valid unit quaternion
  EXPECT_NEAR(rtw::math::norm(result), 1.0, EPSILON);
}

TEST(Quaternion, slerp_shortest_path)
{
  const auto q1 = QuaternionD{rtw::math::IDENTITY};
  // Create quaternion that's "the long way around" (270 degrees = 3*PI/2)
  const auto q2 = QuaternionD::from_axis_angle(Vector3D{0.0, 0.0, 1.0}, AngleD{rtw::math::PI<double>.rad() * 1.5});

  // Slerp should take the short path (90 degrees the other way)
  const auto result = rtw::math::slerp(q1, q2, 0.5);
  // The angle should be less than pi/2 (the short path is 90 degrees)
  EXPECT_LT(result.angle().rad(), rtw::math::PI<double>.rad());
}

TEST(Quaternion, add)
{
  constexpr QuaternionD Q1{1.0, 2.0, 3.0, 4.0};
  constexpr QuaternionD Q2{0.5, 1.5, 2.5, 3.5};
  const auto result = Q1 + Q2;
  expect_quaternion_near(result, 1.5, 3.5, 5.5, 7.5);
}

TEST(Quaternion, subtract)
{
  constexpr QuaternionD Q1{1.0, 2.0, 3.0, 4.0};
  constexpr QuaternionD Q2{0.5, 1.5, 2.5, 3.5};
  const auto result = Q1 - Q2;
  expect_quaternion_near(result, 0.5, 0.5, 0.5, 0.5);
}

TEST(Quaternion, divide_scalar)
{
  constexpr QuaternionD Q{2.0, 4.0, 6.0, 8.0};
  const auto result = Q / 2.0;
  expect_quaternion_near(result, 1.0, 2.0, 3.0, 4.0);
}

TEST(Quaternion, negate)
{
  constexpr QuaternionD Q{1.0, 2.0, 3.0, 4.0};
  const auto result = -Q;
  expect_quaternion_near(result, -1.0, -2.0, -3.0, -4.0);
}

TEST(Quaternion, equality)
{
  constexpr QuaternionD Q1{1.0, 2.0, 3.0, 4.0};
  constexpr QuaternionD Q2{1.0, 2.0, 3.0, 4.0};
  EXPECT_TRUE(Q1 == Q2);
  EXPECT_FALSE(Q1 != Q2);
}

TEST(Quaternion, inequality)
{
  constexpr QuaternionD Q1{1.0, 2.0, 3.0, 4.0};
  constexpr QuaternionD Q2{1.0, 2.0, 3.0, 5.0};
  EXPECT_FALSE(Q1 == Q2);
  EXPECT_TRUE(Q1 != Q2);
}

TEST(Quaternion, rotation_composition_matches_matrix)
{
  // Create two rotations
  const auto q1 = QuaternionD::from_axis_angle(Vector3D{1.0, 0.0, 0.0}, AngleD{0.5});
  const auto q2 = QuaternionD::from_axis_angle(Vector3D{0.0, 1.0, 0.0}, AngleD{0.8});

  // Compose via quaternion multiplication
  const auto q_composed = q1 * q2;

  // Compose via rotation matrices
  const auto m1 = q1.to_rotation_matrix();
  const auto m2 = q2.to_rotation_matrix();
  const auto m_composed = m1 * m2;

  // Convert composed quaternion to matrix
  const auto q_composed_matrix = q_composed.to_rotation_matrix();

  // Matrices should match
  for (int i = 0; i < 3; ++i)
  {
    for (int j = 0; j < 3; ++j)
    {
      EXPECT_NEAR(q_composed_matrix(i, j), m_composed(i, j), EPSILON);
    }
  }
}

TEST(Quaternion, vector_rotation_matches_matrix)
{
  const auto q = QuaternionD::from_euler_angles(AngleD{0.3}, AngleD{0.4}, AngleD{0.5});
  const Vector3D v{1.0, 2.0, 3.0};

  // Rotate via quaternion
  const auto v_quat = q * v;

  // Rotate via matrix
  const auto mat = q.to_rotation_matrix();
  const auto v_mat = mat * v.as_matrix();

  EXPECT_NEAR(v_quat.x(), v_mat(0, 0), EPSILON);
  EXPECT_NEAR(v_quat.y(), v_mat(1, 0), EPSILON);
  EXPECT_NEAR(v_quat.z(), v_mat(2, 0), EPSILON);
}

TEST(Quaternion, slerp_vs_pow_equivalence)
{
  const auto q1 = QuaternionD{rtw::math::IDENTITY};
  const auto q2 =
      rtw::math::normalize(QuaternionD::from_axis_angle(rtw::math::normalize(Vector3D{1.0, 1.0, 1.0}), AngleD{1.0}));

  // slerp(identity, q, t) should equal pow(q, t) for unit quaternions
  for (double t = 0.0; t <= 1.0; t += 0.25)
  {
    const auto slerp_result = rtw::math::slerp(q1, q2, t);
    const auto pow_result = rtw::math::pow(q2, t);
    expect_quaternion_equivalent(slerp_result, pow_result);
  }
}
