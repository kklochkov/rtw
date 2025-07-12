#pragma once

#include "constants/time_constants.h"
#include "event_bus/event_bus.h"
#include "sandbox/ecs/events.h"

#include <cstddef>
#include <string_view>

class SDL_Window;
class SDL_Renderer;
class SDL_Texture;

namespace ecs::demo
{

class Application
{
public:
  Application() = default;
  Application(const Application&) = delete;
  Application(Application&&) = delete;
  Application& operator=(const Application&) = delete;
  Application& operator=(Application&&) = delete;
  ~Application();

  bool init(const std::string_view window_title, const std::int32_t window_width, const std::int32_t window_height);
  void run();

private:
  void process_events();
  void update(const rtw::time_constants::Seconds& delta_time);
  void render();

  void handle_quit_event(const QuitEvent& event);
  void handle_key_press_event(const KeyPressEvent& event);

private:
  SDL_Window* sdl_window_{nullptr};
  SDL_Renderer* sdl_renderer_{nullptr};
  SDL_Texture* sdl_texture_{nullptr};
  rtw::event_bus::EventBus event_bus_;
  bool is_running_{true};
};

} // namespace ecs::demo
