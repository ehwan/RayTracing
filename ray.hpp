#pragma once

#include "math.hpp"
#include "global.hpp"
#include <limits>
#include <numeric>
#include <utility>

namespace eh {

struct Ray
{
  vec3 origin;
  vec3 direction;

  // bounce count for exiting recursive-raytracing
  float bounce = 0;


};

// for ray-casting hittest
struct RayHit
{
  // hit point = origin + t * direction
  float t;
  // hit surface object
  GeometryObject *surface = nullptr;
  // normal vector at hit surface
  vec3 normal;

  vec3 point( Ray const& r ) const
  {
    return r.origin + t*r.direction;
  }
  vec3 reflect( Ray const& r ) const
  {
    return r.direction - 2*normal*normal.dot(r.direction);
  }

  static RayHit no_hit()
  {
    RayHit ret;
    ret.surface = nullptr;
    return ret;
  }
};

}
