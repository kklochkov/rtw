#pragma once

#include "sw_renderer/mesh.h"

#include <filesystem>
#include <istream>
#include <optional>

namespace rtw::sw_renderer
{

/// Result of parsing a .obj file.
struct ObjParseResult
{
  Mesh mesh;                          ///< The parsed mesh geometry and face data.
  std::vector<std::string> materials; ///< Material library filenames referenced by the .obj.
};

/// Parse a Wavefront .obj mesh from a stream.
/// @param[in,out] stream The input stream containing .obj data.
/// @return Parsed mesh and referenced material library filenames.
ObjParseResult load_obj(std::istream& stream);

/// Parse a Wavefront .mtl material library from a stream into an existing mesh.
/// @param[in,out] stream The input stream containing .mtl data.
/// @param[in,out] mesh The mesh to populate with materials and textures.
void load_mtl(std::istream& stream, Mesh& mesh);

/// Load a complete .obj model from disk (including referenced .mtl files).
/// @param[in] path Filesystem path to the .obj file.
/// @return The loaded mesh, or std::nullopt if the file cannot be opened.
std::optional<Mesh> load_obj(const std::filesystem::path& path);

} // namespace rtw::sw_renderer
