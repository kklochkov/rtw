#pragma once

#include "sw_renderer/mesh.h"

#include <filesystem>
#include <istream>
#include <optional>

namespace rtw::sw_renderer
{

struct ObjParseResult
{
  Mesh mesh;
  std::vector<std::string> materials;
};

ObjParseResult load_obj(std::istream& stream);
void load_mtl(std::istream& stream, Mesh& mesh);

std::optional<Mesh> load_obj(const std::filesystem::path& path);

} // namespace rtw::sw_renderer
