#pragma once

#include "event_bus/event_bus.h"

namespace ecs::demo
{

struct QuitEvent : rtw::event_bus::Event
{};

struct KeyPressEvent : rtw::event_bus::Event
{
  explicit KeyPressEvent(const std::int32_t key_code) : key_code{key_code} {}

  std::int32_t key_code{0};
};

} // namespace ecs::demo
