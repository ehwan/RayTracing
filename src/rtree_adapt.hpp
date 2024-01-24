#pragma once

// RTree spatial index library
// https://github.com/ehwan/RTree

#include <RTree.hpp>
#include "geometry.hpp"

namespace eh { namespace rtree {

template <>
struct geometry_traits<BoundingBox>
{
  using area_type = float;

  using bb_type = BoundingBox;

  static bool is_inside( bb_type const& bb, bb_type const& bb2 )
  {
    return bb.is_inside(bb2);
  }
  static bool is_inside( bb_type const& bb, vec3 const& v )
  {
    return bb.is_inside(v);
  }

  static bool is_overlap( bb_type const& bb, bb_type const& bb2 )
  {
    return bb.is_overlap(bb2);
  }

  static area_type area( bb_type const& bb )
  {
    return bb.area();
  }

  static bb_type merge( bb_type const& a, bb_type const& b )
  {
    return { a.min_.cwiseMin(b.min_), a.max_.cwiseMax(b.max_) };
  }
  static bb_type merge( bb_type const& bb, vec3 const& p )
  {
    return { bb.min_.cwiseMin(p), bb.max_.cwiseMax(p) };
  }

  static bb_type intersection( bb_type const& a, bb_type const& b )
  {
    return bb_type{ {0,0,0}, {0,0,0} };
  }
};

}}