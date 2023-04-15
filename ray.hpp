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

  vec3 normal;
  float t;
  int bounce = 0;
  GeometryObject *surface = nullptr;

  vec3 point() const
  {
    return origin + direction*t;
  }
  Ray refract( float index ) const
  {
    vec3 n = normal*normal.dot(direction);
    vec3 tangent = direction - n;

    Ray ray;
    ray.direction = tangent*index + n;
    ray.origin = point();

    return ray;
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