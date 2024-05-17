#pragma once

// RTree spatial index library
// https://github.com/ehwan/RTree

#include "geometry.hpp"
#include <RTree.hpp>

namespace eh
{
namespace rtree
{

template <>
struct geometry_traits<BoundingBox>
{
  using area_type = float;

  using bb_type = BoundingBox;
  using rect_t = bb_type;

  constexpr static int DIM = 3;

  static bool is_inside(rect_t const& rect, rect_t const& rect2)
  {
    return (rect.min_.array() <= rect2.min_.array()).all()
           && (rect2.max_.array() <= rect.max_.array()).all();
  }

  static bool is_overlap(rect_t const& rect, rect_t const& rect2)
  {
    if ((rect2.min_.array() > rect.max_.array()).any())
    {
      return false;
    }
    if ((rect.min_.array() > rect2.max_.array()).any())
    {
      return false;
    }
    return true;
  }

  static rect_t merge(rect_t const& rect, rect_t const& rect2)
  {
    return { rect.min_.array().min(rect2.min_.array()),
             rect.max_.array().max(rect2.max_.array()) };
  }

  static area_type area(rect_t const& rect)
  {
    area_type ret = 1;
    for (unsigned int i = 0; i < 3; ++i)
    {
      ret *= (rect.max_[i] - rect.min_[i]);
    }
    return ret;
  }

  // get scalar value of min_point in axis
  static auto min_point(rect_t const& bound, int axis)
  {
    return bound.min_[axis];
  }
  // get scalar value of max_point in axis
  static auto max_point(rect_t const& bound, int axis)
  {
    return bound.max_[axis];
  }
  // sum of all length of bound for all dimension
  static auto margin(rect_t const& bound)
  {
    area_type ret = 0;
    for (unsigned int i = 0; i < 3; ++i)
    {
      ret += bound.max_[i] - bound.min_[i];
    }
    return ret;
  }

  static rect_t intersection(rect_t const& rect1, rect_t const& rect2)
  {
    const vec3 ret_min = rect1.min_.array().max(rect2.min_.array());
    return { ret_min,
             rect1.max_.array().max(rect2.max_.array()).min(ret_min.array()) };
  }

  // distance between center of bounds
  // used in reinserting
  // returned value is used to sort the reinserted nodes,
  // so no need to call sqrt() nor to be super-accurate
  static auto distance_center(rect_t const& rect1, rect_t const& rect2)
  {
    return (rect1.min_ + rect1.max_ - rect2.min_ - rect2.max_).squaredNorm();
  }

  // ===================== MUST IMPLEMENT =====================
};

}
}