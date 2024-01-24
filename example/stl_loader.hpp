#pragma once

#include <geometry.hpp>
#include <vector>

namespace eh
{
  std::vector<Triangle> load_stl( std::string const& filename );
  void write_stl( std::string const& filename, std::vector<Triangle> const& triangles );
}