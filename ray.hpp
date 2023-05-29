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


  // for ray-casting hittest
  //
  // normal vector at hit surface
  vec3 normal;
  // hit point = origin + t * direction
  float t;
  // bounce count for quitting recursive-raytracing
  float bounce = 0;
  // hit surface object
  GeometryObject *surface = nullptr;

  vec3 point() const
  {
    return origin + direction*t;
  }
  Ray reflect() const
  {
    Ray ray;
    ray.direction = direction - 2*normal*normal.dot(direction);
    ray.origin = point();
    return ray;
  }
};

}
