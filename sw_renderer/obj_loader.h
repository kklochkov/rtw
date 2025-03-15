#pragma once

#include "sw_renderer/mesh.h"

#include <filesystem>
#include <istream>
#include <optional>

namespace rtw::sw_renderer
{

template <typename T>
struct GenericObjParseResult
{
  GenericMesh<T> mesh;
  std::vector<std::string> materials;
};

using ObjParseResult = GenericObjParseResult<float>;
using ObjParseResultQ16 = GenericObjParseResult<fixed_point::FixedPoint16>;
using ObjParseResultQ32 = GenericObjParseResult<fixed_point::FixedPoint32>;

ObjParseResult load_obj(std::istream& stream);
void load_mtl(std::istream& stream, Mesh& mesh);

std::optional<Mesh> load_obj(const std::filesystem::path& path);

ObjParseResultQ16 load_obj_q16(std::istream& stream);
void load_mtl(std::istream& stream, MeshQ16& mesh);

std::optional<MeshQ16> load_obj_q16(const std::filesystem::path& path);

ObjParseResultQ32 load_obj_q32(std::istream& stream);
void load_mtl(std::istream& stream, MeshQ32& mesh);

std::optional<MeshQ32> load_obj_q32(const std::filesystem::path& path);

} // namespace rtw::sw_renderer
