#include "constants/time_constants.h"
#include "sw_renderer/camera.h"
#include "sw_renderer/fixed_pipeline/renderer.h"
#include "sw_renderer/obj_loader.h"
#include "sw_renderer/types.h"

#include "math/transform3.h"

#include <CLI/CLI.hpp>

#include <SDL.h>
#include <SDL_image.h>

#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>

#include <fmt/core.h>

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <numeric>
#include <thread>
#include <vector>

using namespace rtw::sw_renderer::angle_literals;

// Rolling window of recent draw-call timings (milliseconds) backing the on-screen pipeline-cost readout.
constexpr std::size_t DRAW_HISTORY_SIZE = 120U;

class Application
{
public:
  Application(const std::size_t width, const std::size_t height, const bool uncapped);
  Application(const Application&) = delete;
  Application(Application&&) = delete;
  Application& operator=(const Application&) = delete;
  Application& operator=(Application&&) = delete;
  ~Application();

  bool init();
  bool load_mesh(const std::filesystem::path& mesh_path);
  void run();

private:
  void init_imgui();
  static bool load_textures(const std::filesystem::path& resources_folder, rtw::sw_renderer::Mesh& mesh);
  void process_events(bool& is_running, const rtw::time_constants::Seconds& delta_time);
  void update(const rtw::time_constants::Seconds& delta_time);
  void render_imgui();
  void render();

private:
  SDL_Window* sdl_window_{nullptr};
  SDL_Renderer* sdl_renderer_{nullptr};
  SDL_Texture* sdl_texture_{nullptr};
  rtw::sw_renderer::Renderer sw_renderer_;
  rtw::sw_renderer::Mesh mesh_;
  rtw::sw_renderer::Matrix4x4F model_matrix_;
  rtw::sw_renderer::Matrix4x4F view_matrix_;
  rtw::sw_renderer::EulerAnglesF rotation_{0.0_degF, 45.0_degF, 0.0_degF};
  rtw::sw_renderer::Vector3F translation_{0.0F, 0.0F, -5.0F};
  rtw::sw_renderer::Vector3F scale_{1.0F, 1.0F, 1.0F};
  rtw::sw_renderer::Camera camera_{
      rtw::sw_renderer::Point3F{0.0F, 0.0F, 0.0F},
      rtw::sw_renderer::Vector3F{0.0F, 0.0F, -1.0F},
      rtw::sw_renderer::Vector3F{0.0F, 0.0F, 0.0F},
  };
  bool show_demo_window_{false};
  bool uncapped_{false};

  std::vector<float> draw_ms_history_;
  std::size_t draw_ms_cursor_{0U};
};

Application::Application(const std::size_t width, const std::size_t height, const bool uncapped)
    : sw_renderer_(width, height), uncapped_(uncapped)
{
  draw_ms_history_.assign(DRAW_HISTORY_SIZE, 0.0F);
}

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

bool Application::init()
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

  //  sdl_window_ = SDL_CreateWindow("Software Renderer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
  //                                 sw_renderer_.width(), sw_renderer_.height(), SDL_WINDOW_ALLOW_HIGHDPI);
  sdl_window_ = SDL_CreateWindow("Software Renderer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1'024, 768,
                                 SDL_WINDOW_ALLOW_HIGHDPI);

  if (sdl_window_ == nullptr)
  {
    fmt::print("Could not create window: {}\n", SDL_GetError());
    return false;
  }

  const std::uint32_t renderer_flags =
      uncapped_ ? static_cast<std::uint32_t>(SDL_RENDERER_ACCELERATED)
                : static_cast<std::uint32_t>(SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  sdl_renderer_ = SDL_CreateRenderer(sdl_window_, -1, renderer_flags);

  if (sdl_renderer_ == nullptr)
  {
    fmt::print("Could not create renderer: {}\n", SDL_GetError());
    return false;
  }

  sdl_texture_ = SDL_CreateTexture(sdl_renderer_, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING,
                                   static_cast<std::int32_t>(sw_renderer_.width()),
                                   static_cast<std::int32_t>(sw_renderer_.height()));

  if (sdl_texture_ == nullptr)
  {
    fmt::print("Could not create texture: {}\n", SDL_GetError());
    return false;
  }

  init_imgui();
  return true;
}

bool Application::load_mesh(const std::filesystem::path& mesh_path)
{
  const std::filesystem::path resources_folder = mesh_path.parent_path();
  auto maybe_mesh = rtw::sw_renderer::load_obj(mesh_path);
  if (!maybe_mesh.has_value())
  {
    // TODO: error handling
    //    fmt::print("Could not load mesh: {}\n", maybe_mesh.error());
    return false;
  }

  mesh_ = std::move(maybe_mesh.value());
  return load_textures(resources_folder, mesh_);
}

void Application::init_imgui()
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
  // ImGui::StyleColorsLight();

  // Setup Platform/Renderer backends
  ImGui_ImplSDL2_InitForSDLRenderer(sdl_window_, sdl_renderer_);
  ImGui_ImplSDLRenderer2_Init(sdl_renderer_);
}

bool Application::load_textures(const std::filesystem::path& resources_folder, rtw::sw_renderer::Mesh& mesh)
{
  for (auto& [name, texture] : mesh.textures)
  {
    const auto texture_path = resources_folder / name;
    SDL_Surface* surface = IMG_Load(texture_path.c_str());
    if (surface == nullptr)
    {
      fmt::print("Could not load texture {}: {}\n", name, IMG_GetError());
      return false;
    }

    SDL_Surface* converted_surface = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA8888, 0);
    SDL_FreeSurface(surface);

    if (converted_surface == nullptr)
    {
      fmt::print("Could not convert texture {}: {}\n", name, SDL_GetError());
      return false;
    }

    fmt::print("Texture {}: {}x{}, loaded.\n", name, converted_surface->w, converted_surface->h);

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    texture = rtw::sw_renderer::Texture{reinterpret_cast<std::uint32_t*>(converted_surface->pixels),
                                        static_cast<std::size_t>(converted_surface->w),
                                        static_cast<std::size_t>(converted_surface->h)};

    SDL_FreeSurface(converted_surface);
  }
  return true;
}

void Application::process_events(bool& is_running, const rtw::time_constants::Seconds& delta_time)
{
  const auto speed = 2.0F * delta_time.count();              // m/s
  const auto angular_speed = 15.0_degF * delta_time.count(); // deg/s -> rad/s

  SDL_Event event;
  while (SDL_PollEvent(&event))
  {
    ImGui_ImplSDL2_ProcessEvent(&event);
    switch (event.type)
    {
    case SDL_QUIT:
      is_running = false;
      break;
    case SDL_KEYDOWN:
      switch (event.key.keysym.sym)
      {
      case SDLK_ESCAPE:
        is_running = false;
        break;
      case SDLK_w:
      {
        if (event.key.keysym.mod & KMOD_SHIFT)
        {
          camera_.rotation.x() -= angular_speed;
        }
        else
        {
          camera_.velocity = camera_.direction * speed;
          camera_.position = camera_.position + camera_.velocity;
        }
      }
      break;
      case SDLK_s:
      {
        if (event.key.keysym.mod & KMOD_SHIFT)
        {
          camera_.rotation.x() += angular_speed;
        }
        else
        {
          camera_.velocity = camera_.direction * speed;
          camera_.position = camera_.position - camera_.velocity;
        }
      }
      break;
      case SDLK_a:
        camera_.rotation.y() -= angular_speed;
        break;
      case SDLK_d:
        camera_.rotation.y() += angular_speed;
        break;
      case SDLK_UP:
        camera_.position.y() += speed;
        break;
      case SDLK_DOWN:
        camera_.position.y() -= speed;
        break;
      case SDLK_LEFT:
        camera_.position.x() -= speed;
        break;
      case SDLK_RIGHT:
        camera_.position.x() += speed;
        break;
      default:
        break;
      }
      break;
    default:
      break;
    }
  }
}

void Application::update(const rtw::time_constants::Seconds& delta_time)
{
  std::ignore = delta_time;

  rtw::sw_renderer::Point3F target{0.0F, 0.0F, -1.0F};

  camera_.direction =
      rtw::math::transform3::make_rotation(camera_.rotation) * static_cast<rtw::sw_renderer::Vector3F>(target);

  target = camera_.position + camera_.direction;

  rotation_.z() += 0.5_degF;

  model_matrix_ = rtw::math::transform3::make_transform(scale_, rotation_, translation_);
  view_matrix_ = rtw::sw_renderer::make_look_at(camera_.position, target);
}

void Application::render_imgui()
{
  // NOLINTBEGIN(hicpp-signed-bitwise)
  ImGui::Begin("Settings", nullptr,
               ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoNavInputs);
  // NOLINTEND(hicpp-signed-bitwise)
  ImGui::SetWindowPos(ImVec2(0, 0));
  ImGui::SetWindowCollapsed(true, ImGuiCond_FirstUseEver);
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg, hicpp-vararg)
  ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0F / ImGui::GetIO().Framerate,
              ImGui::GetIO().Framerate);

  const auto draw_ms_minmax = std::minmax_element(draw_ms_history_.begin(), draw_ms_history_.end());
  const auto draw_ms_avg =
      std::accumulate(draw_ms_history_.begin(), draw_ms_history_.end(), 0.0F) / static_cast<float>(DRAW_HISTORY_SIZE);
  const auto draw_ms_last = draw_ms_history_[(draw_ms_cursor_ + DRAW_HISTORY_SIZE - 1U) % DRAW_HISTORY_SIZE];
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg, hicpp-vararg)
  ImGui::Text("draw %.3f ms  avg %.3f  min %.3f  max %.3f", draw_ms_last, draw_ms_avg, *draw_ms_minmax.first,
              *draw_ms_minmax.second);
  ImGui::PlotLines("##draw_ms", draw_ms_history_.data(), static_cast<int>(DRAW_HISTORY_SIZE),
                   static_cast<int>(draw_ms_cursor_), nullptr, 0.0F, *draw_ms_minmax.second, ImVec2(0.0F, 40.0F));

  bool enabled = sw_renderer_.face_culling_enabled();
  if (ImGui::Checkbox("Face Culling", &enabled))
  {
    sw_renderer_.set_face_culling_enabled(enabled);
  }

  enabled = sw_renderer_.wireframe_enabled();
  if (ImGui::Checkbox("Wireframe", &enabled))
  {
    sw_renderer_.set_wireframe_enabled(enabled);
  }

  enabled = sw_renderer_.vertex_drawing_enabled();
  if (ImGui::Checkbox("Vertex Drawing", &enabled))
  {
    sw_renderer_.set_vertex_drawing_enabled(enabled);
  }

  enabled = sw_renderer_.light_enabled();
  if (ImGui::Checkbox("Light", &enabled))
  {
    sw_renderer_.set_light_enabled(enabled);
  }

  enabled = sw_renderer_.normal_draw_enabled();
  if (ImGui::Checkbox("Normal Draw", &enabled))
  {
    sw_renderer_.set_normal_draw_enabled(enabled);
  }

  if (ImGui::RadioButton("Shading (flat)", sw_renderer_.shading_enabled()))
  {
    sw_renderer_.set_shading_enabled(true);
    sw_renderer_.set_texture_enabled(false);
  }

  if (ImGui::RadioButton("Texture", sw_renderer_.texture_enabled()))
  {
    sw_renderer_.set_shading_enabled(false);
    sw_renderer_.set_texture_enabled(true);
  }

  ImGui::Separator();

  ImGui::Checkbox("Demo Window", &show_demo_window_);
  if (show_demo_window_)
  {
    ImGui::ShowDemoWindow(&show_demo_window_);
  }

  ImGui::End();
}

void Application::render()
{
  {
    // Start the Dear ImGui frame
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    render_imgui();

    ImGui::Render();

    ImGuiIO& io = ImGui::GetIO();
    SDL_RenderSetScale(sdl_renderer_, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
  }

  SDL_SetRenderDrawColor(sdl_renderer_, 0, 0xFF, 0, 0xFF);
  SDL_RenderClear(sdl_renderer_);

  constexpr auto GREY = rtw::sw_renderer::Color{0x80'80'80'FF};
  sw_renderer_.clear(GREY);
  const auto draw_start = std::chrono::steady_clock::now();
  sw_renderer_.draw_mesh(mesh_, view_matrix_ * model_matrix_);
  const rtw::time_constants::Milliseconds draw_ms = std::chrono::steady_clock::now() - draw_start;
  draw_ms_history_[draw_ms_cursor_] = draw_ms.count();
  draw_ms_cursor_ = (draw_ms_cursor_ + 1U) % DRAW_HISTORY_SIZE;

  SDL_UpdateTexture(sdl_texture_, nullptr, sw_renderer_.data(), static_cast<std::int32_t>(sw_renderer_.pitch()));
  SDL_RenderCopy(sdl_renderer_, sdl_texture_, nullptr, nullptr);

  {
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
  }

  SDL_RenderPresent(sdl_renderer_);
}

void Application::run()
{
  constexpr auto TARGET_FRAME_RATE = 60.0F;
  constexpr rtw::time_constants::Milliseconds TARGET_FRAME_TIME{1000.0F / TARGET_FRAME_RATE};
  auto last_frame_time = std::chrono::system_clock::now();

  bool is_running = true;
  while (is_running)
  {
    const auto current_frame_time = std::chrono::system_clock::now();
    const rtw::time_constants::Seconds frame_time = current_frame_time - last_frame_time;

    process_events(is_running, frame_time);
    update(frame_time);
    render();

    last_frame_time = current_frame_time;
    if (!uncapped_)
    {
      const rtw::time_constants::Milliseconds sleep_time = TARGET_FRAME_TIME - frame_time;
      if (sleep_time > rtw::time_constants::Milliseconds::zero() && sleep_time < TARGET_FRAME_TIME)
      {
        std::this_thread::sleep_for(sleep_time);
      }
    }
  }
}

int main(int argc, char* argv[]) // NOLINT(bugprone-exception-escape)
{
  CLI::App cli_app{"Software Renderer"};

  std::filesystem::path mesh_path;
  cli_app.add_option("-m,--mesh", mesh_path, "Mesh file path")
      ->check(CLI::ExistingFile)
      ->default_val("sw_renderer/resources/textured_cube.obj");

  bool uncapped = false;
  cli_app.add_flag("--uncapped", uncapped, "Disable vsync and the frame-rate limiter (uncapped FPS for benchmarking)");

  CLI11_PARSE(cli_app, argc, argv);

  //  Application app(800, 600);
  Application app(320, 240, uncapped);
  //  Application app(1024, 768);

  if (!app.init())
  {
    return EXIT_FAILURE;
  }

  if (!app.load_mesh(mesh_path))
  {
    return EXIT_FAILURE;
  }

  app.run();

  return EXIT_SUCCESS;
}
