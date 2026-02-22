#include "math/matrix.h"
#include "math/matrix_operations.h"
#include "math/transform3.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

// Type aliases for convenience
using RowMajor2x2 = rtw::math::Matrix<float, 2, 2, rtw::math::MemoryOrder::ROW_MAJOR>;
using ColMajor2x2 = rtw::math::Matrix<float, 2, 2, rtw::math::MemoryOrder::COLUMN_MAJOR>;
using RowMajor3x3 = rtw::math::Matrix<float, 3, 3, rtw::math::MemoryOrder::ROW_MAJOR>;
using ColMajor3x3 = rtw::math::Matrix<float, 3, 3, rtw::math::MemoryOrder::COLUMN_MAJOR>;
using RowMajor4x4 = rtw::math::Matrix<float, 4, 4, rtw::math::MemoryOrder::ROW_MAJOR>;
using ColMajor4x4 = rtw::math::Matrix<float, 4, 4, rtw::math::MemoryOrder::COLUMN_MAJOR>;
using RowMajor2x3 = rtw::math::Matrix<float, 2, 3, rtw::math::MemoryOrder::ROW_MAJOR>;
using ColMajor2x3 = rtw::math::Matrix<float, 2, 3, rtw::math::MemoryOrder::COLUMN_MAJOR>;
using RowMajor3x2 = rtw::math::Matrix<float, 3, 2, rtw::math::MemoryOrder::ROW_MAJOR>;
using ColMajor3x2 = rtw::math::Matrix<float, 3, 2, rtw::math::MemoryOrder::COLUMN_MAJOR>;

TEST(MatrixStorageOrder, row_major_memory_layout_2x2)
{
  // Row-major: elements stored row-by-row
  // Matrix:
  //   1  2
  //   3  4
  // Memory layout: [1, 2, 3, 4]
  constexpr RowMajor2x2 M{1.0F, 2.0F, 3.0F, 4.0F};

  // operator[] accesses raw memory order
  EXPECT_EQ(M[0], 1.0F);
  EXPECT_EQ(M[1], 2.0F);
  EXPECT_EQ(M[2], 3.0F);
  EXPECT_EQ(M[3], 4.0F);

  // operator() accesses logical (row, col)
  EXPECT_EQ(M(0, 0), 1.0F);
  EXPECT_EQ(M(0, 1), 2.0F);
  EXPECT_EQ(M(1, 0), 3.0F);
  EXPECT_EQ(M(1, 1), 4.0F);
}

TEST(MatrixStorageOrder, column_major_memory_layout_2x2)
{
  // Column-major: elements stored column-by-column
  // For the same logical matrix:
  //   1  2
  //   3  4
  // Memory layout: [1, 3, 2, 4]
  constexpr ColMajor2x2 M{1.0F, 3.0F, 2.0F, 4.0F};

  // operator[] accesses raw memory order
  EXPECT_EQ(M[0], 1.0F);
  EXPECT_EQ(M[1], 3.0F);
  EXPECT_EQ(M[2], 2.0F);
  EXPECT_EQ(M[3], 4.0F);

  // operator() accesses logical (row, col)
  EXPECT_EQ(M(0, 0), 1.0F);
  EXPECT_EQ(M(0, 1), 2.0F);
  EXPECT_EQ(M(1, 0), 3.0F);
  EXPECT_EQ(M(1, 1), 4.0F);
}

TEST(MatrixStorageOrder, row_major_memory_layout_2x3)
{
  // Row-major 2x3:
  //   1  2  3
  //   4  5  6
  // Memory layout: [1, 2, 3, 4, 5, 6]
  constexpr RowMajor2x3 M{1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F};

  EXPECT_THAT(M, ::testing::ElementsAre(1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F));

  EXPECT_EQ(M(0, 0), 1.0F);
  EXPECT_EQ(M(0, 1), 2.0F);
  EXPECT_EQ(M(0, 2), 3.0F);
  EXPECT_EQ(M(1, 0), 4.0F);
  EXPECT_EQ(M(1, 1), 5.0F);
  EXPECT_EQ(M(1, 2), 6.0F);
}

TEST(MatrixStorageOrder, column_major_memory_layout_2x3)
{
  // Column-major 2x3:
  //   1  2  3
  //   4  5  6
  // Memory layout: [1, 4, 2, 5, 3, 6]
  constexpr ColMajor2x3 M{1.0F, 4.0F, 2.0F, 5.0F, 3.0F, 6.0F};

  EXPECT_THAT(M, ::testing::ElementsAre(1.0F, 4.0F, 2.0F, 5.0F, 3.0F, 6.0F));

  EXPECT_EQ(M(0, 0), 1.0F);
  EXPECT_EQ(M(0, 1), 2.0F);
  EXPECT_EQ(M(0, 2), 3.0F);
  EXPECT_EQ(M(1, 0), 4.0F);
  EXPECT_EQ(M(1, 1), 5.0F);
  EXPECT_EQ(M(1, 2), 6.0F);
}

TEST(MatrixStorageOrder, from_row_major_tag_with_row_major_matrix)
{
  // FROM_ROW_MAJOR into ROW_MAJOR: no conversion needed
  constexpr RowMajor2x2 M{rtw::math::FROM_ROW_MAJOR, 1.0F, 2.0F, 3.0F, 4.0F};

  // Memory layout should be [1, 2, 3, 4]
  EXPECT_THAT(M, ::testing::ElementsAre(1.0F, 2.0F, 3.0F, 4.0F));

  // Logical access
  EXPECT_EQ(M(0, 0), 1.0F);
  EXPECT_EQ(M(0, 1), 2.0F);
  EXPECT_EQ(M(1, 0), 3.0F);
  EXPECT_EQ(M(1, 1), 4.0F);
}

TEST(MatrixStorageOrder, from_row_major_tag_with_column_major_matrix)
{
  // FROM_ROW_MAJOR into COLUMN_MAJOR: converts data
  // Input (row-major order): 1, 2, 3, 4
  // Logical matrix:
  //   1  2
  //   3  4
  // Output (column-major memory): [1, 3, 2, 4]
  constexpr ColMajor2x2 M{rtw::math::FROM_ROW_MAJOR, 1.0F, 2.0F, 3.0F, 4.0F};

  // Memory layout should be column-major
  EXPECT_THAT(M, ::testing::ElementsAre(1.0F, 3.0F, 2.0F, 4.0F));

  // Logical access should match the row-major input
  EXPECT_EQ(M(0, 0), 1.0F);
  EXPECT_EQ(M(0, 1), 2.0F);
  EXPECT_EQ(M(1, 0), 3.0F);
  EXPECT_EQ(M(1, 1), 4.0F);
}

TEST(MatrixStorageOrder, from_row_major_tag_with_column_major_2x3)
{
  // 2x3 matrix specified in row-major order:
  //   1  2  3
  //   4  5  6
  constexpr ColMajor2x3 M{rtw::math::FROM_ROW_MAJOR, 1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F};

  // Memory should be column-major: [1, 4, 2, 5, 3, 6]
  EXPECT_THAT(M, ::testing::ElementsAre(1.0F, 4.0F, 2.0F, 5.0F, 3.0F, 6.0F));

  // Logical access
  EXPECT_EQ(M(0, 0), 1.0F);
  EXPECT_EQ(M(0, 1), 2.0F);
  EXPECT_EQ(M(0, 2), 3.0F);
  EXPECT_EQ(M(1, 0), 4.0F);
  EXPECT_EQ(M(1, 1), 5.0F);
  EXPECT_EQ(M(1, 2), 6.0F);
}

TEST(MatrixStorageOrder, from_column_major_tag_with_column_major_matrix)
{
  // FROM_COLUMN_MAJOR into COLUMN_MAJOR: no conversion needed
  constexpr ColMajor2x2 M{rtw::math::FROM_COLUMN_MAJOR, 1.0F, 3.0F, 2.0F, 4.0F};

  // Memory layout should be [1, 3, 2, 4]
  EXPECT_THAT(M, ::testing::ElementsAre(1.0F, 3.0F, 2.0F, 4.0F));

  // Logical matrix:
  //   1  2
  //   3  4
  EXPECT_EQ(M(0, 0), 1.0F);
  EXPECT_EQ(M(0, 1), 2.0F);
  EXPECT_EQ(M(1, 0), 3.0F);
  EXPECT_EQ(M(1, 1), 4.0F);
}

TEST(MatrixStorageOrder, from_column_major_tag_with_row_major_matrix)
{
  // FROM_COLUMN_MAJOR into ROW_MAJOR: converts data
  // Input (column-major order): 1, 3, 2, 4
  // Logical matrix:
  //   1  2
  //   3  4
  // Output (row-major memory): [1, 2, 3, 4]
  constexpr RowMajor2x2 M{rtw::math::FROM_COLUMN_MAJOR, 1.0F, 3.0F, 2.0F, 4.0F};

  // Memory layout should be row-major
  EXPECT_THAT(M, ::testing::ElementsAre(1.0F, 2.0F, 3.0F, 4.0F));

  // Logical access
  EXPECT_EQ(M(0, 0), 1.0F);
  EXPECT_EQ(M(0, 1), 2.0F);
  EXPECT_EQ(M(1, 0), 3.0F);
  EXPECT_EQ(M(1, 1), 4.0F);
}

TEST(MatrixStorageOrder, identity_row_major)
{
  constexpr RowMajor3x3 M{rtw::math::IDENTITY};

  // Row-major identity: [1, 0, 0, 0, 1, 0, 0, 0, 1]
  EXPECT_THAT(M, ::testing::ElementsAre(1, 0, 0, 0, 1, 0, 0, 0, 1));

  EXPECT_EQ(M(0, 0), 1.0F);
  EXPECT_EQ(M(1, 1), 1.0F);
  EXPECT_EQ(M(2, 2), 1.0F);
  EXPECT_EQ(M(0, 1), 0.0F);
  EXPECT_EQ(M(1, 0), 0.0F);
}

TEST(MatrixStorageOrder, identity_column_major)
{
  constexpr ColMajor3x3 M{rtw::math::IDENTITY};

  // Column-major identity: [1, 0, 0, 0, 1, 0, 0, 0, 1]
  // Note: identity is the same in both layouts!
  EXPECT_THAT(M, ::testing::ElementsAre(1, 0, 0, 0, 1, 0, 0, 0, 1));

  EXPECT_EQ(M(0, 0), 1.0F);
  EXPECT_EQ(M(1, 1), 1.0F);
  EXPECT_EQ(M(2, 2), 1.0F);
  EXPECT_EQ(M(0, 1), 0.0F);
  EXPECT_EQ(M(1, 0), 0.0F);
}

TEST(MatrixStorageOrder, identity_non_square_row_major)
{
  constexpr RowMajor2x3 M{rtw::math::IDENTITY};

  // 2x3 identity (diagonal 1s where row == col):
  //   1  0  0
  //   0  1  0
  // Row-major: [1, 0, 0, 0, 1, 0]
  EXPECT_THAT(M, ::testing::ElementsAre(1, 0, 0, 0, 1, 0));
}

TEST(MatrixStorageOrder, identity_non_square_column_major)
{
  constexpr ColMajor2x3 M{rtw::math::IDENTITY};

  // 2x3 identity (diagonal 1s where row == col):
  //   1  0  0
  //   0  1  0
  // Column-major: [1, 0, 0, 1, 0, 0]
  EXPECT_THAT(M, ::testing::ElementsAre(1, 0, 0, 1, 0, 0));
}

TEST(MatrixStorageOrder, cast_row_major_to_column_major)
{
  constexpr RowMajor2x2 RM{rtw::math::FROM_ROW_MAJOR, 1.0F, 2.0F, 3.0F, 4.0F};
  const auto cm = RM.cast<rtw::math::MemoryOrder::COLUMN_MAJOR>();

  static_assert(std::is_same_v<decltype(cm), const ColMajor2x2>);

  // Memory layouts differ
  EXPECT_THAT(RM, ::testing::ElementsAre(1.0F, 2.0F, 3.0F, 4.0F));
  EXPECT_THAT(cm, ::testing::ElementsAre(1.0F, 3.0F, 2.0F, 4.0F));

  // Logical access is identical
  EXPECT_EQ(RM(0, 0), cm(0, 0));
  EXPECT_EQ(RM(0, 1), cm(0, 1));
  EXPECT_EQ(RM(1, 0), cm(1, 0));
  EXPECT_EQ(RM(1, 1), cm(1, 1));
}

TEST(MatrixStorageOrder, cast_column_major_to_row_major)
{
  constexpr ColMajor2x2 CM{rtw::math::FROM_ROW_MAJOR, 1.0F, 2.0F, 3.0F, 4.0F};
  const auto rm = CM.cast<rtw::math::MemoryOrder::ROW_MAJOR>();

  static_assert(std::is_same_v<decltype(rm), const RowMajor2x2>);

  // Memory layouts differ
  EXPECT_THAT(CM, ::testing::ElementsAre(1.0F, 3.0F, 2.0F, 4.0F));
  EXPECT_THAT(rm, ::testing::ElementsAre(1.0F, 2.0F, 3.0F, 4.0F));

  // Logical access is identical
  EXPECT_EQ(CM(0, 0), rm(0, 0));
  EXPECT_EQ(CM(0, 1), rm(0, 1));
  EXPECT_EQ(CM(1, 0), rm(1, 0));
  EXPECT_EQ(CM(1, 1), rm(1, 1));
}

TEST(MatrixStorageOrder, cast_roundtrip)
{
  constexpr RowMajor3x3 ORIGINAL{rtw::math::FROM_ROW_MAJOR, 1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F, 7.0F, 8.0F, 9.0F};

  const auto to_col = ORIGINAL.cast<rtw::math::MemoryOrder::COLUMN_MAJOR>();
  const auto back_to_row = to_col.cast<rtw::math::MemoryOrder::ROW_MAJOR>();

  EXPECT_EQ(ORIGINAL, back_to_row);
}

TEST(MatrixStorageOrder, multiplication_row_major)
{
  // A = [1, 2; 3, 4]
  // B = [5, 6; 7, 8]
  // A * B = [19, 22; 43, 50]
  constexpr RowMajor2x2 A{rtw::math::FROM_ROW_MAJOR, 1.0F, 2.0F, 3.0F, 4.0F};
  constexpr RowMajor2x2 B{rtw::math::FROM_ROW_MAJOR, 5.0F, 6.0F, 7.0F, 8.0F};

  constexpr auto C = A * B;

  EXPECT_EQ(C(0, 0), 19.0F);
  EXPECT_EQ(C(0, 1), 22.0F);
  EXPECT_EQ(C(1, 0), 43.0F);
  EXPECT_EQ(C(1, 1), 50.0F);
}

TEST(MatrixStorageOrder, multiplication_column_major)
{
  // Same logical matrices, column-major storage
  constexpr ColMajor2x2 A{rtw::math::FROM_ROW_MAJOR, 1.0F, 2.0F, 3.0F, 4.0F};
  constexpr ColMajor2x2 B{rtw::math::FROM_ROW_MAJOR, 5.0F, 6.0F, 7.0F, 8.0F};

  constexpr auto C = A * B;

  // Same result regardless of storage order
  EXPECT_EQ(C(0, 0), 19.0F);
  EXPECT_EQ(C(0, 1), 22.0F);
  EXPECT_EQ(C(1, 0), 43.0F);
  EXPECT_EQ(C(1, 1), 50.0F);
}

TEST(MatrixStorageOrder, multiplication_result_matches_between_orders)
{
  constexpr RowMajor3x3 A_RM{rtw::math::FROM_ROW_MAJOR, 1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F, 7.0F, 8.0F, 9.0F};
  constexpr RowMajor3x3 B_RM{rtw::math::FROM_ROW_MAJOR, 9.0F, 8.0F, 7.0F, 6.0F, 5.0F, 4.0F, 3.0F, 2.0F, 1.0F};

  constexpr ColMajor3x3 A_CM{rtw::math::FROM_ROW_MAJOR, 1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F, 7.0F, 8.0F, 9.0F};
  constexpr ColMajor3x3 B_CM{rtw::math::FROM_ROW_MAJOR, 9.0F, 8.0F, 7.0F, 6.0F, 5.0F, 4.0F, 3.0F, 2.0F, 1.0F};

  constexpr auto C_RM = A_RM * B_RM;
  constexpr auto C_CM = A_CM * B_CM;

  // Results should be logically identical
  for (std::uint16_t row = 0; row < 3; ++row)
  {
    for (std::uint16_t col = 0; col < 3; ++col)
    {
      EXPECT_EQ(C_RM(row, col), C_CM(row, col)) << "Mismatch at (" << row << ", " << col << ")";
    }
  }
}

TEST(MatrixStorageOrder, multiplication_non_square)
{
  // A: 2x3, B: 3x2 -> C: 2x2
  constexpr RowMajor2x3 A_RM{rtw::math::FROM_ROW_MAJOR, 1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F};
  constexpr RowMajor3x2 B_RM{rtw::math::FROM_ROW_MAJOR, 7.0F, 8.0F, 9.0F, 10.0F, 11.0F, 12.0F};

  constexpr ColMajor2x3 A_CM{rtw::math::FROM_ROW_MAJOR, 1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F};
  constexpr ColMajor3x2 B_CM{rtw::math::FROM_ROW_MAJOR, 7.0F, 8.0F, 9.0F, 10.0F, 11.0F, 12.0F};

  constexpr auto C_RM = A_RM * B_RM;
  constexpr auto C_CM = A_CM * B_CM;

  // Expected: [58, 64; 139, 154]
  EXPECT_EQ(C_RM(0, 0), 58.0F);
  EXPECT_EQ(C_RM(0, 1), 64.0F);
  EXPECT_EQ(C_RM(1, 0), 139.0F);
  EXPECT_EQ(C_RM(1, 1), 154.0F);

  EXPECT_EQ(C_CM(0, 0), 58.0F);
  EXPECT_EQ(C_CM(0, 1), 64.0F);
  EXPECT_EQ(C_CM(1, 0), 139.0F);
  EXPECT_EQ(C_CM(1, 1), 154.0F);
}

TEST(MatrixStorageOrder, transpose_row_major)
{
  constexpr RowMajor2x3 M{rtw::math::FROM_ROW_MAJOR, 1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F};
  const auto mt = rtw::math::transpose(M);

  static_assert(std::is_same_v<std::remove_cv_t<decltype(mt)>,
                               rtw::math::Matrix<float, 3, 2, rtw::math::MemoryOrder::ROW_MAJOR>>);

  EXPECT_EQ(mt(0, 0), 1.0F);
  EXPECT_EQ(mt(0, 1), 4.0F);
  EXPECT_EQ(mt(1, 0), 2.0F);
  EXPECT_EQ(mt(1, 1), 5.0F);
  EXPECT_EQ(mt(2, 0), 3.0F);
  EXPECT_EQ(mt(2, 1), 6.0F);
}

TEST(MatrixStorageOrder, transpose_column_major)
{
  constexpr ColMajor2x3 M{rtw::math::FROM_ROW_MAJOR, 1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F};
  const auto mt = rtw::math::transpose(M);

  static_assert(std::is_same_v<std::remove_cv_t<decltype(mt)>,
                               rtw::math::Matrix<float, 3, 2, rtw::math::MemoryOrder::COLUMN_MAJOR>>);

  EXPECT_EQ(mt(0, 0), 1.0F);
  EXPECT_EQ(mt(0, 1), 4.0F);
  EXPECT_EQ(mt(1, 0), 2.0F);
  EXPECT_EQ(mt(1, 1), 5.0F);
  EXPECT_EQ(mt(2, 0), 3.0F);
  EXPECT_EQ(mt(2, 1), 6.0F);
}

TEST(MatrixStorageOrder, determinant_row_major)
{
  constexpr RowMajor2x2 M{rtw::math::FROM_ROW_MAJOR, 1.0F, 2.0F, 3.0F, 4.0F};
  constexpr auto DET = rtw::math::determinant(M);
  EXPECT_FLOAT_EQ(DET, -2.0F);
}

TEST(MatrixStorageOrder, determinant_column_major)
{
  constexpr ColMajor2x2 M{rtw::math::FROM_ROW_MAJOR, 1.0F, 2.0F, 3.0F, 4.0F};
  constexpr auto DET = rtw::math::determinant(M);
  EXPECT_FLOAT_EQ(DET, -2.0F);
}

TEST(MatrixStorageOrder, inverse_row_major)
{
  constexpr RowMajor2x2 M{rtw::math::FROM_ROW_MAJOR, 4.0F, 7.0F, 2.0F, 6.0F};
  const auto inv = rtw::math::inverse(M);
  const auto identity = M * inv;

  EXPECT_NEAR(identity(0, 0), 1.0F, 1e-5F);
  EXPECT_NEAR(identity(0, 1), 0.0F, 1e-5F);
  EXPECT_NEAR(identity(1, 0), 0.0F, 1e-5F);
  EXPECT_NEAR(identity(1, 1), 1.0F, 1e-5F);
}

TEST(MatrixStorageOrder, inverse_column_major)
{
  constexpr ColMajor2x2 M{rtw::math::FROM_ROW_MAJOR, 4.0F, 7.0F, 2.0F, 6.0F};
  const auto inv = rtw::math::inverse(M);
  const auto identity = M * inv;

  EXPECT_NEAR(identity(0, 0), 1.0F, 1e-5F);
  EXPECT_NEAR(identity(0, 1), 0.0F, 1e-5F);
  EXPECT_NEAR(identity(1, 0), 0.0F, 1e-5F);
  EXPECT_NEAR(identity(1, 1), 1.0F, 1e-5F);
}

TEST(MatrixStorageOrder, transform_scale_row_major)
{
  constexpr auto SCALE = rtw::math::transform3::make_scale<float, rtw::math::MemoryOrder::ROW_MAJOR>(2.0F, 3.0F, 4.0F);

  EXPECT_EQ(SCALE(0, 0), 2.0F);
  EXPECT_EQ(SCALE(1, 1), 3.0F);
  EXPECT_EQ(SCALE(2, 2), 4.0F);
}

TEST(MatrixStorageOrder, transform_scale_column_major)
{
  constexpr auto SCALE =
      rtw::math::transform3::make_scale<float, rtw::math::MemoryOrder::COLUMN_MAJOR>(2.0F, 3.0F, 4.0F);

  EXPECT_EQ(SCALE(0, 0), 2.0F);
  EXPECT_EQ(SCALE(1, 1), 3.0F);
  EXPECT_EQ(SCALE(2, 2), 4.0F);
}

TEST(MatrixStorageOrder, transform_translation_row_major)
{
  constexpr auto TRANS =
      rtw::math::transform3::make_translation<float, rtw::math::MemoryOrder::ROW_MAJOR>(10.0F, 20.0F, 30.0F);

  // Translation in last column for column-vector convention
  EXPECT_EQ(TRANS(0, 3), 10.0F);
  EXPECT_EQ(TRANS(1, 3), 20.0F);
  EXPECT_EQ(TRANS(2, 3), 30.0F);
  EXPECT_EQ(TRANS(3, 3), 1.0F);
}

TEST(MatrixStorageOrder, transform_translation_column_major)
{
  constexpr auto TRANS =
      rtw::math::transform3::make_translation<float, rtw::math::MemoryOrder::COLUMN_MAJOR>(10.0F, 20.0F, 30.0F);

  // Translation in last column for column-vector convention
  EXPECT_EQ(TRANS(0, 3), 10.0F);
  EXPECT_EQ(TRANS(1, 3), 20.0F);
  EXPECT_EQ(TRANS(2, 3), 30.0F);
  EXPECT_EQ(TRANS(3, 3), 1.0F);
}

TEST(MatrixStorageOrder, static_properties)
{
  static_assert(RowMajor2x2::IS_ROW_MAJOR);
  static_assert(!RowMajor2x2::IS_COLUMN_MAJOR);
  static_assert(!ColMajor2x2::IS_ROW_MAJOR);
  static_assert(ColMajor2x2::IS_COLUMN_MAJOR);

  static_assert(RowMajor2x2::NUM_ROWS == 2);
  static_assert(RowMajor2x2::NUM_COLS == 2);
  static_assert(ColMajor2x2::NUM_ROWS == 2);
  static_assert(ColMajor2x2::NUM_COLS == 2);

  // Default memory order is row-major
  static_assert(rtw::math::Matrix2x2F::IS_ROW_MAJOR);
  static_assert(rtw::math::DEFAULT_MEMORY_ORDER == rtw::math::MemoryOrder::ROW_MAJOR);
}

TEST(MatrixStorageOrder, runtime_operations)
{
  constexpr RowMajor2x2 A{rtw::math::FROM_ROW_MAJOR, 1.0F, 2.0F, 3.0F, 4.0F};
  constexpr RowMajor2x2 B{rtw::math::FROM_ROW_MAJOR, 5.0F, 6.0F, 7.0F, 8.0F};

  const auto sum = A + B;
  const auto diff = A - B;
  const auto prod = A * B;
  const auto scaled = A * 2.0F;
  const auto transposed = rtw::math::transpose(A);
  constexpr auto DET = rtw::math::determinant(A);
  const auto converted = A.cast<rtw::math::MemoryOrder::COLUMN_MAJOR>();

  EXPECT_THAT(sum, ::testing::ElementsAre(6.0F, 8.0F, 10.0F, 12.0F));
  EXPECT_THAT(diff, ::testing::ElementsAre(-4.0F, -4.0F, -4.0F, -4.0F));
  EXPECT_THAT(prod, ::testing::ElementsAre(19.0F, 22.0F, 43.0F, 50.0F));
  EXPECT_THAT(scaled, ::testing::ElementsAre(2.0F, 4.0F, 6.0F, 8.0F));
  EXPECT_EQ(transposed(0, 1), 3.0F);
  EXPECT_EQ(DET, -2.0F);
  EXPECT_EQ(converted(0, 1), 2.0F);
}

TEST(MatrixStorageOrder, row_extraction_row_major)
{
  constexpr RowMajor2x3 M{rtw::math::FROM_ROW_MAJOR, 1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F};

  const auto row0 = M.row(0);
  const auto row1 = M.row(1);

  EXPECT_EQ(row0[0], 1.0F);
  EXPECT_EQ(row0[1], 2.0F);
  EXPECT_EQ(row0[2], 3.0F);

  EXPECT_EQ(row1[0], 4.0F);
  EXPECT_EQ(row1[1], 5.0F);
  EXPECT_EQ(row1[2], 6.0F);
}

TEST(MatrixStorageOrder, row_extraction_column_major)
{
  constexpr ColMajor2x3 M{rtw::math::FROM_ROW_MAJOR, 1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F};

  const auto row0 = M.row(0);
  const auto row1 = M.row(1);

  EXPECT_EQ(row0[0], 1.0F);
  EXPECT_EQ(row0[1], 2.0F);
  EXPECT_EQ(row0[2], 3.0F);

  EXPECT_EQ(row1[0], 4.0F);
  EXPECT_EQ(row1[1], 5.0F);
  EXPECT_EQ(row1[2], 6.0F);
}

TEST(MatrixStorageOrder, column_extraction_row_major)
{
  constexpr RowMajor2x3 M{rtw::math::FROM_ROW_MAJOR, 1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F};

  const auto col0 = M.column(0);
  const auto col1 = M.column(1);
  const auto col2 = M.column(2);

  EXPECT_EQ(col0[0], 1.0F);
  EXPECT_EQ(col0[1], 4.0F);

  EXPECT_EQ(col1[0], 2.0F);
  EXPECT_EQ(col1[1], 5.0F);

  EXPECT_EQ(col2[0], 3.0F);
  EXPECT_EQ(col2[1], 6.0F);
}

TEST(MatrixStorageOrder, column_extraction_column_major)
{
  constexpr ColMajor2x3 M{rtw::math::FROM_ROW_MAJOR, 1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F};

  const auto col0 = M.column(0);
  const auto col1 = M.column(1);
  const auto col2 = M.column(2);

  EXPECT_EQ(col0[0], 1.0F);
  EXPECT_EQ(col0[1], 4.0F);

  EXPECT_EQ(col1[0], 2.0F);
  EXPECT_EQ(col1[1], 5.0F);

  EXPECT_EQ(col2[0], 3.0F);
  EXPECT_EQ(col2[1], 6.0F);
}

TEST(MatrixStorageOrder, direct_memory_access_row_major)
{
  RowMajor2x2 m{rtw::math::FROM_ROW_MAJOR, 1.0F, 2.0F, 3.0F, 4.0F};

  // Direct memory access should be row-major
  EXPECT_EQ(m[0], 1.0F);
  EXPECT_EQ(m[1], 2.0F);
  EXPECT_EQ(m[2], 3.0F);
  EXPECT_EQ(m[3], 4.0F);
}

TEST(MatrixStorageOrder, direct_memory_access_column_major)
{
  ColMajor2x2 m{rtw::math::FROM_ROW_MAJOR, 1.0F, 2.0F, 3.0F, 4.0F};

  // Direct memory access should be column-major
  EXPECT_EQ(m[0], 1.0F);
  EXPECT_EQ(m[1], 3.0F);
  EXPECT_EQ(m[2], 2.0F);
  EXPECT_EQ(m[3], 4.0F);
}
