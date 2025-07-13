#include "sandbox/ecs/application.h"
#include "sandbox/ecs/events.h"
#include "sw_renderer/color.h"

#include "constants/render_constants.h"
#include "constants/time_constants.h"

#include <fmt/core.h>

#include <SDL.h>
#include <SDL_image.h>

#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>

#include <thread>

namespace ecs::demo
{

Application::~Application()
{
  {
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
  }

  SDL_DestroyTexture(sdl_texture_);
  SDL_DestroyRenderer(sdl_renderer_);
  SDL_DestroyWindow(sdl_window_);
  IMG_Quit();
  SDL_Quit();
}

bool Application::init(const std::string_view window_title, const std::int32_t window_width,
                       const std::int32_t window_height)
{
  if (SDL_Init(SDL_INIT_VIDEO) != 0)
  {
    fmt::print("Could not initialize SDL: {}\n", SDL_GetError());
    return false;
  }

  if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG)
  {
    fmt::print("Could not initialize SDL_image: {}\n", IMG_GetError());
    return false;
  }

  {
    // From 2.0.18: Enable native IME.
#ifdef SDL_HINT_IME_SHOW_UI
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif
  }

  sdl_window_ = SDL_CreateWindow(window_title.data(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, window_width,
                                 window_height, SDL_WINDOW_ALLOW_HIGHDPI);

  if (sdl_window_ == nullptr)
  {
    fmt::print("Could not create window: {}\n", SDL_GetError());
    return false;
  }

  sdl_renderer_ = SDL_CreateRenderer(sdl_window_, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

  if (sdl_renderer_ == nullptr)
  {
    fmt::print("Could not create renderer: {}\n", SDL_GetError());
    return false;
  }

  sdl_texture_ = SDL_CreateTexture(sdl_renderer_, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, window_width,
                                   window_height);

  if (sdl_texture_ == nullptr)
  {
    fmt::print("Could not create texture: {}\n", SDL_GetError());
    return false;
  }

  {
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    // NOLINTBEGIN(hicpp-signed-bitwise)
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
    // NOLINTEND(hicpp-signed-bitwise)

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForSDLRenderer(sdl_window_, sdl_renderer_);
    ImGui_ImplSDLRenderer2_Init(sdl_renderer_);
  }

  {
    event_bus_.subscribe<QuitEvent>(*this, &Application::handle_quit_event);
    event_bus_.subscribe<KeyPressEvent>(*this, &Application::handle_key_press_event);
  }

  return true;
}

void Application::run()
{
  auto last_frame_time = std::chrono::system_clock::now();

  while (is_running_)
  {
    const auto current_frame_time = std::chrono::system_clock::now();
    const rtw::time_constants::Seconds frame_time = current_frame_time - last_frame_time;

    process_events();
    update(frame_time);
    render();

    const rtw::time_constants::Milliseconds sleep_time{rtw::render_constants::TARGET_FRAME_TIME - frame_time};
    last_frame_time = current_frame_time;
    if ((sleep_time.count() > 0) && (sleep_time < rtw::render_constants::TARGET_FRAME_TIME))
    {
      std::this_thread::sleep_for(sleep_time);
    }
  }
}

void Application::process_events()
{
  SDL_Event event;
  while (SDL_PollEvent(&event))
  {
    ImGui_ImplSDL2_ProcessEvent(&event);

    if (event.type == SDL_QUIT)
    {
      event_bus_.publish(QuitEvent{});
    }
    else if (event.type == SDL_KEYDOWN)
    {
      event_bus_.publish(KeyPressEvent{event.key.keysym.sym});
    }
  }
}

void Application::update(const rtw::time_constants::Seconds& /*delta_time*/) {}

void Application::render()
{
  constexpr auto GREY = rtw::sw_renderer::Color{0x80'80'80'FF};
  SDL_SetRenderDrawColor(sdl_renderer_, GREY.r(), GREY.g(), GREY.b(), GREY.a());
  SDL_RenderClear(sdl_renderer_);

  SDL_RenderPresent(sdl_renderer_);
}

void Application::handle_quit_event(const QuitEvent& /*event*/)
{
  fmt::print("Exiting on quit event.\n");
  is_running_ = false;
}

void Application::handle_key_press_event(const KeyPressEvent& event)
{
  fmt::print("Key pressed: {}\n", event.key_code);

  if (event.key_code == SDLK_ESCAPE)
  {
    is_running_ = false;
    fmt::print("Exiting on ESC key press.\n");
  }
}

} // namespace ecs::demo
