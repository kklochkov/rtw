#include "constants/time_constants.h"
#include "stl/span.h"
#include "sw_renderer/camera.h"
#include "sw_renderer/color.h"
#include "sw_renderer/mesh.h"
#include "sw_renderer/obj_loader.h"
#include "sw_renderer/programmable_pipeline/builtin_shaders.h"
#include "sw_renderer/programmable_pipeline/frame_buffer.h"
#include "sw_renderer/programmable_pipeline/pipeline.h"
#include "sw_renderer/programmable_pipeline/pipeline_state.h"
#include "sw_renderer/programmable_pipeline/sampler.h"
#include "sw_renderer/programmable_pipeline/shader.h"
#include "sw_renderer/programmable_pipeline/vertex_layout.h"
#include "sw_renderer/programmable_pipeline/vertex_stream.h"
#include "sw_renderer/render_stats.h"
#include "sw_renderer/texture.h"
#include "sw_renderer/types.h"

#include "math/matrix_operations.h"
#include "math/transform3.h"
#include "math/vector_operations.h"

#include <CLI/CLI.hpp>

#include <SDL.h>
#include <SDL_image.h>

#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>

#include <fmt/core.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <numeric>
#include <thread>
#include <tuple>
#include <utility>
#include <vector>

using namespace rtw::sw_renderer::angle_literals;

namespace
{

/// Rolling window of recent draw-call timings (milliseconds) backing the on-screen pipeline-cost readout.
constexpr std::size_t DRAW_HISTORY_SIZE = 120U;

/// Which built-in shader program the demo currently feeds to the pipeline.
enum class ShaderKind : std::uint8_t
{
  FLAT,
  VERTEX_COLOR,
  TEXTURED,
  LIT,
};

/// Interleaved, GPU-style vertex consumed by the programmable pipeline.
///
/// The pipeline reads attributes from raw bytes through a `VertexLayout`, so the members are plain
/// `std::array<float, N>` (a standard-layout type) addressed by the hard-coded byte offsets below.
/// Keeping the storage as `float`/`FLOAT32` is deliberate: the vertex buffer stays the same in the
/// fixed-point build, where the decoder converts each `FLOAT32` component to `single_precision`.
struct DemoVertex
{
  std::array<float, 4> position;
  std::array<float, 4> normal;
  std::array<float, 2> uv;
  std::array<float, 4> color;
};

static_assert(sizeof(DemoVertex) == 56U, "DemoVertex must be tightly packed for the byte-offset layout");
static_assert(offsetof(DemoVertex, position) == 0U, "position attribute offset mismatch");
static_assert(offsetof(DemoVertex, normal) == 16U, "normal attribute offset mismatch");
static_assert(offsetof(DemoVertex, uv) == 32U, "uv attribute offset mismatch");
static_assert(offsetof(DemoVertex, color) == 40U, "color attribute offset mismatch");

/// The layout describing how `DemoVertex` maps onto the built-in shader attribute locations.
rtw::sw_renderer::VertexLayout make_demo_layout()
{
  return rtw::sw_renderer::VertexLayout{
      {
          rtw::sw_renderer::VertexAttribute{rtw::sw_renderer::attribute_location::POSITION, 0U,
                                            rtw::sw_renderer::ComponentType::FLOAT32, 4U},
          rtw::sw_renderer::VertexAttribute{rtw::sw_renderer::attribute_location::NORMAL, 16U,
                                            rtw::sw_renderer::ComponentType::FLOAT32, 4U},
          rtw::sw_renderer::VertexAttribute{rtw::sw_renderer::attribute_location::UV, 32U,
                                            rtw::sw_renderer::ComponentType::FLOAT32, 2U},
          rtw::sw_renderer::VertexAttribute{rtw::sw_renderer::attribute_location::COLOR, 40U,
                                            rtw::sw_renderer::ComponentType::FLOAT32, 4U},
      },
      sizeof(DemoVertex)};
}

/// Expand the index-based `Mesh` into a flat, three-vertices-per-triangle stream.
///
/// The OBJ loader stores separate vertex / texture / normal index arrays; the programmable pipeline
/// wants one interleaved record per corner. The per-vertex colour is derived from object-space
/// position (`p * 0.5 + 0.5`) so the `VertexColorShader` shows a recognisable RGB cube, and the V
/// texture coordinate is flipped because OBJ places the origin bottom-left while textures are top-left.
std::vector<DemoVertex> build_interleaved(const rtw::sw_renderer::Mesh& mesh)
{
  const auto to_float = [](const rtw::sw_renderer::single_precision value) { return static_cast<float>(value); };

  std::vector<DemoVertex> vertices;
  vertices.reserve(mesh.faces.size() * 3U);

  for (const auto& face : mesh.faces)
  {
    const auto& a = mesh.vertices[face.vertex_indices[0U]];
    const auto& b = mesh.vertices[face.vertex_indices[1U]];
    const auto& c = mesh.vertices[face.vertex_indices[2U]];
    const rtw::sw_renderer::Vector3F edge_ab{b.x() - a.x(), b.y() - a.y(), b.z() - a.z()};
    const rtw::sw_renderer::Vector3F edge_ac{c.x() - a.x(), c.y() - a.y(), c.z() - a.z()};
    const auto face_normal = rtw::math::normalize(rtw::math::cross(edge_ab, edge_ac));

    const bool has_normals = face.normal_indices.has_value() && !mesh.normals.empty();
    const bool has_uvs = face.texture_indices.has_value() && !mesh.tex_coords.empty();

    for (std::size_t i = 0U; i < 3U; ++i)
    {
      const auto& position = mesh.vertices[face.vertex_indices[i]];

      auto normal = face_normal;
      if (has_normals)
      {
        normal = mesh.normals[(*face.normal_indices)[i]];
      }

      float u = 0.0F;
      float v = 0.0F;
      if (has_uvs)
      {
        const auto& tex_coord = mesh.tex_coords[(*face.texture_indices)[i]];
        u = to_float(tex_coord.u());
        v = 1.0F - to_float(tex_coord.v());
      }

      DemoVertex vertex{};
      vertex.position = {to_float(position.x()), to_float(position.y()), to_float(position.z()), 1.0F};
      vertex.normal = {to_float(normal.x()), to_float(normal.y()), to_float(normal.z()), 0.0F};
      vertex.uv = {u, v};
      vertex.color = {(to_float(position.x()) * 0.5F) + 0.5F, (to_float(position.y()) * 0.5F) + 0.5F,
                      (to_float(position.z()) * 0.5F) + 0.5F, 1.0F};
      vertices.push_back(vertex);
    }
  }

  return vertices;
}

} // namespace

/// Interactive showcase for the programmable software pipeline.
///
/// Mirrors the SDL2 + Dear ImGui scaffolding of the fixed-function demo (`main.cpp`) but drives a
/// `Pipeline` with selectable built-in shaders and live render-state toggles instead of the
/// monolithic `Renderer`.
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
  void shader_radio(const char* label, ShaderKind kind);
  void cull_radio(const char* label, rtw::sw_renderer::CullMode mode);
  rtw::sw_renderer::Vector4F object_color() const;
  rtw::sw_renderer::Vector3F light_direction() const;
  void configure_state();
  rtw::sw_renderer::IShaderProgram* configure_program();
  void render();

private:
  SDL_Window* sdl_window_{nullptr};
  SDL_Renderer* sdl_renderer_{nullptr};
  SDL_Texture* sdl_texture_{nullptr};

  rtw::sw_renderer::Pipeline pipeline_;
  rtw::sw_renderer::FrameBuffer framebuffer_;
  rtw::sw_renderer::PipelineState state_;
  rtw::sw_renderer::RenderStats stats_;

  rtw::sw_renderer::FlatColorShader flat_shader_;
  rtw::sw_renderer::VertexColorShader vertex_color_shader_;
  rtw::sw_renderer::TexturedShader textured_shader_;
  rtw::sw_renderer::LitShader lit_shader_;

  rtw::sw_renderer::Mesh mesh_;
  std::vector<DemoVertex> vertices_;
  rtw::sw_renderer::VertexLayout layout_{make_demo_layout()};
  const rtw::sw_renderer::Texture* diffuse_texture_{nullptr};

  rtw::sw_renderer::Matrix4x4F projection_matrix_{rtw::sw_renderer::Matrix4x4F::identity()};
  rtw::sw_renderer::Matrix4x4F model_matrix_{rtw::sw_renderer::Matrix4x4F::identity()};
  rtw::sw_renderer::Matrix4x4F view_matrix_{rtw::sw_renderer::Matrix4x4F::identity()};
  rtw::sw_renderer::EulerAnglesF rotation_{0.0_degF, 0.0_degF, 0.0_degF};
  rtw::sw_renderer::Vector3F translation_{0.0F, 0.0F, -5.0F};
  rtw::sw_renderer::Vector3F scale_{1.0F, 1.0F, 1.0F};
  rtw::sw_renderer::Camera camera_{
      rtw::sw_renderer::Point3F{0.0F, 0.0F, 0.0F},
      rtw::sw_renderer::Vector3F{0.0F, 0.0F, -1.0F},
      rtw::sw_renderer::Vector3F{0.0F, 0.0F, 0.0F},
  };

  ShaderKind shader_kind_{ShaderKind::TEXTURED};
  rtw::sw_renderer::CullMode cull_mode_{rtw::sw_renderer::CullMode::BACK};
  rtw::sw_renderer::FilterMode filter_mode_{rtw::sw_renderer::FilterMode::LINEAR};
  std::array<float, 4> color_{1.0F, 0.5F, 0.2F, 1.0F};
  std::array<float, 3> light_{0.3F, -0.5F, -1.0F};
  bool depth_test_enabled_{true};
  bool blend_enabled_{false};
  bool show_demo_window_{false};
  bool uncapped_{false};

  std::vector<float> draw_ms_history_;
  std::size_t draw_ms_cursor_{0U};
};

Application::Application(const std::size_t width, const std::size_t height, const bool uncapped)
    : framebuffer_(width, height), uncapped_(uncapped)
{
  draw_ms_history_.assign(DRAW_HISTORY_SIZE, 0.0F);
  const auto aspect_ratio = static_cast<rtw::sw_renderer::single_precision>(framebuffer_.aspect_ratio());
  const auto fov_y = 60.0_degF;
  const auto frustum_params = rtw::math::make_perspective_parameters(
      fov_y, aspect_ratio, rtw::sw_renderer::single_precision{0.1F}, rtw::sw_renderer::single_precision{100.0F});
  projection_matrix_ = rtw::math::make_perspective_projection_matrix(frustum_params);
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

  sdl_window_ = SDL_CreateWindow("Programmable Software Renderer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                 1'280, 960, SDL_WINDOW_ALLOW_HIGHDPI);

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
                                   static_cast<std::int32_t>(framebuffer_.width()),
                                   static_cast<std::int32_t>(framebuffer_.height()));

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
    fmt::print("Could not load mesh: {}\n", mesh_path.string());
    return false;
  }

  mesh_ = std::move(maybe_mesh.value());
  if (!load_textures(resources_folder, mesh_))
  {
    return false;
  }

  vertices_ = build_interleaved(mesh_);
  if (!mesh_.textures.empty())
  {
    diffuse_texture_ = &mesh_.textures.begin()->second;
  }
  return true;
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

  rotation_.y() += 0.5_degF;

  model_matrix_ = rtw::math::transform3::make_transform(scale_, rotation_, translation_);
  view_matrix_ = rtw::sw_renderer::make_look_at(camera_.position, target);
}

void Application::shader_radio(const char* label, const ShaderKind kind)
{
  if (ImGui::RadioButton(label, shader_kind_ == kind))
  {
    shader_kind_ = kind;
  }
}

void Application::cull_radio(const char* label, const rtw::sw_renderer::CullMode mode)
{
  if (ImGui::RadioButton(label, cull_mode_ == mode))
  {
    cull_mode_ = mode;
  }
}

rtw::sw_renderer::Vector4F Application::object_color() const
{
  return rtw::sw_renderer::Vector4F{
      rtw::sw_renderer::single_precision{color_[0]}, rtw::sw_renderer::single_precision{color_[1]},
      rtw::sw_renderer::single_precision{color_[2]}, rtw::sw_renderer::single_precision{color_[3]}};
}

rtw::sw_renderer::Vector3F Application::light_direction() const
{
  const rtw::sw_renderer::Vector3F direction{rtw::sw_renderer::single_precision{light_[0]},
                                             rtw::sw_renderer::single_precision{light_[1]},
                                             rtw::sw_renderer::single_precision{light_[2]}};
  if (rtw::math::dot(direction, direction) <= rtw::sw_renderer::single_precision{0.0F})
  {
    return rtw::sw_renderer::Vector3F{rtw::sw_renderer::single_precision{0.0F},
                                      rtw::sw_renderer::single_precision{0.0F},
                                      rtw::sw_renderer::single_precision{-1.0F}};
  }
  return rtw::math::normalize(direction);
}

void Application::render_imgui()
{
  // NOLINTBEGIN(hicpp-signed-bitwise)
  ImGui::Begin("Programmable Pipeline", nullptr,
               ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoNavInputs);
  // NOLINTEND(hicpp-signed-bitwise)
  ImGui::SetWindowPos(ImVec2(0, 0));

  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg, hicpp-vararg)
  ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0F / ImGui::GetIO().Framerate,
              ImGui::GetIO().Framerate);

  ImGui::Separator();
  ImGui::TextUnformatted("Shader");
  shader_radio("Flat colour", ShaderKind::FLAT);
  shader_radio("Vertex colour", ShaderKind::VERTEX_COLOR);
  shader_radio("Textured", ShaderKind::TEXTURED);
  shader_radio("Lit (Lambert)", ShaderKind::LIT);

  if ((shader_kind_ == ShaderKind::FLAT) || (shader_kind_ == ShaderKind::LIT))
  {
    ImGui::ColorEdit4("Colour", color_.data());
  }
  if (shader_kind_ == ShaderKind::LIT)
  {
    ImGui::SliderFloat3("Light direction", light_.data(), -1.0F, 1.0F);
  }
  if (shader_kind_ == ShaderKind::TEXTURED)
  {
    bool linear = filter_mode_ == rtw::sw_renderer::FilterMode::LINEAR;
    if (ImGui::Checkbox("Bilinear filtering", &linear))
    {
      filter_mode_ = linear ? rtw::sw_renderer::FilterMode::LINEAR : rtw::sw_renderer::FilterMode::NEAREST;
    }
  }

  ImGui::Separator();
  ImGui::TextUnformatted("Rasteriser state");
  cull_radio("Cull none", rtw::sw_renderer::CullMode::NONE);
  cull_radio("Cull back", rtw::sw_renderer::CullMode::BACK);
  cull_radio("Cull front", rtw::sw_renderer::CullMode::FRONT);
  ImGui::Checkbox("Depth test", &depth_test_enabled_);
  ImGui::Checkbox("Alpha blending (src-over)", &blend_enabled_);

  ImGui::Separator();
  ImGui::TextUnformatted("Statistics");
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg, hicpp-vararg)
  ImGui::Text("submitted %zu  clipped %zu", stats_.triangles_submitted, stats_.triangles_clipped);
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg, hicpp-vararg)
  ImGui::Text("culled %zu  rendered %zu", stats_.triangles_culled, stats_.triangles_rendered);

  const auto draw_ms_minmax = std::minmax_element(draw_ms_history_.begin(), draw_ms_history_.end());
  const auto draw_ms_avg =
      std::accumulate(draw_ms_history_.begin(), draw_ms_history_.end(), 0.0F) / static_cast<float>(DRAW_HISTORY_SIZE);
  const auto draw_ms_last = draw_ms_history_[(draw_ms_cursor_ + DRAW_HISTORY_SIZE - 1U) % DRAW_HISTORY_SIZE];
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg, hicpp-vararg)
  ImGui::Text("draw %.3f ms  avg %.3f  min %.3f  max %.3f", draw_ms_last, draw_ms_avg, *draw_ms_minmax.first,
              *draw_ms_minmax.second);
  ImGui::PlotLines("##draw_ms", draw_ms_history_.data(), static_cast<int>(DRAW_HISTORY_SIZE),
                   static_cast<int>(draw_ms_cursor_), nullptr, 0.0F, *draw_ms_minmax.second, ImVec2(0.0F, 40.0F));

  ImGui::Separator();
  ImGui::Checkbox("ImGui demo window", &show_demo_window_);
  if (show_demo_window_)
  {
    ImGui::ShowDemoWindow(&show_demo_window_);
  }

  ImGui::End();
}

void Application::configure_state()
{
  state_.viewport = rtw::sw_renderer::Viewport{0, 0, static_cast<std::int32_t>(framebuffer_.width()),
                                               static_cast<std::int32_t>(framebuffer_.height())};
  state_.cull_mode = cull_mode_;
  state_.front_face = rtw::sw_renderer::FrontFace::COUNTER_CLOCKWISE;
  state_.depth_test_enabled = depth_test_enabled_;
  state_.depth_write_enabled = true;

  state_.blend.enabled = blend_enabled_;
  state_.blend.src_rgb = rtw::sw_renderer::BlendFactor::SRC_ALPHA;
  state_.blend.dst_rgb = rtw::sw_renderer::BlendFactor::ONE_MINUS_SRC_ALPHA;
  state_.blend.eq_rgb = rtw::sw_renderer::BlendEquation::ADD;
  state_.blend.src_alpha = rtw::sw_renderer::BlendFactor::ONE;
  state_.blend.dst_alpha = rtw::sw_renderer::BlendFactor::ONE_MINUS_SRC_ALPHA;
  state_.blend.eq_alpha = rtw::sw_renderer::BlendEquation::ADD;
}

rtw::sw_renderer::IShaderProgram* Application::configure_program()
{
  const auto mvp = projection_matrix_ * view_matrix_ * model_matrix_;

  switch (shader_kind_)
  {
  case ShaderKind::FLAT:
    flat_shader_.set_mvp_matrix(mvp);
    flat_shader_.set_color(object_color());
    return &flat_shader_;
  case ShaderKind::VERTEX_COLOR:
    vertex_color_shader_.set_mvp_matrix(mvp);
    return &vertex_color_shader_;
  case ShaderKind::TEXTURED:
    textured_shader_.set_mvp_matrix(mvp);
    if (diffuse_texture_ != nullptr)
    {
      textured_shader_.set_sampler(
          rtw::sw_renderer::Sampler2D{*diffuse_texture_, rtw::sw_renderer::WrapMode::REPEAT, filter_mode_});
    }
    return &textured_shader_;
  case ShaderKind::LIT:
    lit_shader_.set_mvp_matrix(mvp);
    lit_shader_.set_color(object_color());
    lit_shader_.set_normal_matrix(view_matrix_ * model_matrix_);
    lit_shader_.set_light_direction(light_direction());
    return &lit_shader_;
  }
  return &textured_shader_;
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
  framebuffer_.clear(GREY, 1.0F);

  configure_state();
  auto* program = configure_program();
  stats_.reset();

  if ((program != nullptr) && !vertices_.empty())
  {
    const rtw::sw_renderer::RawVertexStream stream{layout_, rtw::stl::as_bytes(rtw::stl::make_span(vertices_))};
    const auto draw_start = std::chrono::steady_clock::now();
    pipeline_.draw_arrays(*program, stream, state_, framebuffer_, stats_);
    const rtw::time_constants::Milliseconds draw_ms = std::chrono::steady_clock::now() - draw_start;
    draw_ms_history_[draw_ms_cursor_] = draw_ms.count();
    draw_ms_cursor_ = (draw_ms_cursor_ + 1U) % DRAW_HISTORY_SIZE;
  }

  SDL_UpdateTexture(sdl_texture_, nullptr, framebuffer_.color_buffer().data(),
                    static_cast<std::int32_t>(framebuffer_.color_buffer().pitch()));
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
      if ((sleep_time > rtw::time_constants::Milliseconds::zero()) && (sleep_time < TARGET_FRAME_TIME))
      {
        std::this_thread::sleep_for(sleep_time);
      }
    }
  }
}

int main(int argc, char* argv[]) // NOLINT(bugprone-exception-escape)
{
  CLI::App cli_app{"Programmable Software Renderer"};

  std::filesystem::path mesh_path;
  cli_app.add_option("-m,--mesh", mesh_path, "Mesh file path")
      ->check(CLI::ExistingFile)
      ->default_val("sw_renderer/resources/textured_cube.obj");

  bool uncapped = false;
  cli_app.add_flag("--uncapped", uncapped, "Disable vsync and the frame-rate limiter (uncapped FPS for benchmarking)");

  CLI11_PARSE(cli_app, argc, argv);

  Application app(640, 480, uncapped);

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
