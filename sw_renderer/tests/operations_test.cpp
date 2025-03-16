#include "sw_renderer/operations.h"

#include <gtest/gtest.h>

#include <array>

TEST(Opertions, make_baricentric)
{
  const rtw::math::Point2F v0{0.0F, 0.0F};
  const rtw::math::Point2F v1{1.0F, 0.0F};
  const rtw::math::Point2F v2{0.0F, 1.0F};

  const std::array<std::pair<rtw::math::Point2F, rtw::sw_renderer::Barycentric3F>, 9> data = {
      std::pair{rtw::math::Point2F{0.0F, 0.0F}, rtw::sw_renderer::Barycentric3F{1.0F, 0.0F, 0.0F}},
      std::pair{rtw::math::Point2F{0.5F, 0.0F}, rtw::sw_renderer::Barycentric3F{0.5F, 0.5F, 0.0F}},
      std::pair{rtw::math::Point2F{0.0F, 0.5F}, rtw::sw_renderer::Barycentric3F{0.5F, 0.0F, 0.5F}},
      std::pair{rtw::math::Point2F{0.5F, 0.5F}, rtw::sw_renderer::Barycentric3F{0.0F, 0.5F, 0.5F}},
      std::pair{rtw::math::Point2F{0.4F, 0.4F}, rtw::sw_renderer::Barycentric3F{0.2F, 0.4F, 0.4F}},
      std::pair{rtw::math::Point2F{-0.5F, 0.5F}, rtw::sw_renderer::Barycentric3F{1.0F, -0.5F, 0.5F}},
      std::pair{rtw::math::Point2F{-0.5F, -0.5F}, rtw::sw_renderer::Barycentric3F{2.0F, -0.5F, -0.5F}},
      std::pair{rtw::math::Point2F{0.5F, -0.5F}, rtw::sw_renderer::Barycentric3F{1.0F, 0.5F, -0.5F}},
      std::pair{rtw::math::Point2F{1.0F, 1.0F}, rtw::sw_renderer::Barycentric3F{-1.0F, 1.0F, 1.0F}},
  };

  for (const auto& [p, b] : data)
  {
    const auto result = rtw::sw_renderer::make_barycentric(v0, v1, v2, p);
    ASSERT_FLOAT_EQ(result.alpha(), b.alpha());
    ASSERT_FLOAT_EQ(result.beta(), b.beta());
    ASSERT_FLOAT_EQ(result.gamma(), b.gamma());
  }
}

TEST(Opertions, contains)
{
  const rtw::math::Point2F v0{0.0F, 0.0F};
  const rtw::math::Point2F v1{1.0F, 0.0F};
  const rtw::math::Point2F v2{0.0F, 1.0F};

  const std::array<std::pair<rtw::math::Point2F, bool>, 9> data = {
      std::pair{rtw::math::Point2F{0.0F, 0.0F}, true},    std::pair{rtw::math::Point2F{0.5F, 0.0F}, true},
      std::pair{rtw::math::Point2F{0.0F, 0.5F}, true},    std::pair{rtw::math::Point2F{0.5F, 0.5F}, true},
      std::pair{rtw::math::Point2F{0.4F, 0.4F}, true},    std::pair{rtw::math::Point2F{-0.5F, 0.5F}, false},
      std::pair{rtw::math::Point2F{-0.5F, -0.5F}, false}, std::pair{rtw::math::Point2F{0.5F, -0.5F}, false},
      std::pair{rtw::math::Point2F{1.0F, 1.0F}, false},
  };

  for (const auto& [p, b] : data)
  {
    const auto result = rtw::sw_renderer::contains(v0, v1, v2, p);
    ASSERT_EQ(result, b);
  }
}

TEST(Opertions, contains2)
{
  const rtw::math::Point2I a{80, 100};
  const rtw::math::Point2I b{200, 100};
  const rtw::math::Point2I c{150, 200};
  const rtw::math::Point2I p{150, 150};

  const auto r = rtw::sw_renderer::make_barycentric(a.cast<float>(), b.cast<float>(), c.cast<float>(), p.cast<float>());
  ASSERT_FLOAT_EQ(r.alpha(), 5.0F / 24.0F);
  ASSERT_FLOAT_EQ(r.beta(), 7.0F / 24.0F);
  ASSERT_FLOAT_EQ(r.gamma(), 1.0F / 2.0F);
}

TEST(Opertions, contains3)
{
  const rtw::math::Point2I a{50, 50};
  const rtw::math::Point2I b{100, 150};
  const rtw::math::Point2I c{250, 250};
  const rtw::math::Point2I p{106, 154};

  const auto r = rtw::sw_renderer::make_barycentric(a.cast<float>(), b.cast<float>(), c.cast<float>(), p.cast<float>());
  ASSERT_FLOAT_EQ(r.alpha(), 0.0F);
  ASSERT_NEAR(r.beta(), 24.F / 25.0F, 1e-6F);
  ASSERT_NEAR(r.gamma(), 1.0F / 25.0F, 1e-6F);
}

TEST(Opertions, winding_order)
{
  const rtw::math::Point2F v0{0.0F, 0.0F};
  const rtw::math::Point2F v1{1.0F, 0.0F};
  const rtw::math::Point2F v2{0.0F, 1.0F};

  ASSERT_EQ(rtw::sw_renderer::winding_order(v0, v1, v2), rtw::sw_renderer::WindingOrder::COUNTER_CLOCKWISE);
  ASSERT_EQ(rtw::sw_renderer::winding_order(v0, v2, v1), rtw::sw_renderer::WindingOrder::CLOCKWISE);
  ASSERT_EQ(rtw::sw_renderer::winding_order(v1, v0, v2), rtw::sw_renderer::WindingOrder::CLOCKWISE);
  ASSERT_EQ(rtw::sw_renderer::winding_order(v1, v2, v0), rtw::sw_renderer::WindingOrder::COUNTER_CLOCKWISE);
  ASSERT_EQ(rtw::sw_renderer::winding_order(v2, v0, v1), rtw::sw_renderer::WindingOrder::COUNTER_CLOCKWISE);
  ASSERT_EQ(rtw::sw_renderer::winding_order(v2, v1, v0), rtw::sw_renderer::WindingOrder::CLOCKWISE);
}
