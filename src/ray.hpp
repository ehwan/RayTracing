#pragma once

#include "math.hpp"
#include "global.hpp"
#include <cassert>
#include <limits>

namespace eh {

class Ray
{
protected:
  vec3 _origin;
  vec3 _direction;
  vec3 _inv_direction;

public:
  // bounce count for exiting recursive-raytracing
  float bounce = 0;
  int thread_id = 0;

  Ray( vec3 origin, vec3 direction, int tid )
    : _origin(origin), _direction(direction), thread_id(tid)
  {
    // |direction|=1
    assert( direction.norm() > 0.0f );
    assert( direction.norm()-1.0f < 1e-3f );
    _inv_direction = 1.0 / direction.array();
  }

  vec3 const& origin() const { return _origin; }
  vec3 const& direction() const { return _direction; }
  vec3 const& inv_direction() const { return _inv_direction; }

};

// for ray-casting hittest
struct RayHit
{
  // hit point = origin + t * direction
  float t;
  // hit object
  Object const *surface = nullptr;
  // normal vector at hit surface, |normal|=1
  vec3 normal;

  vec3 point( Ray const& r ) const
  {
    return r.origin() + t*r.direction();
  }
  vec3 reflect( Ray const& r ) const
  {
    return r.direction() - 2*normal * normal.dot( r.direction() );
  }
  static RayHit no_hit()
  {
    RayHit ret;
    ret.t = std::numeric_limits<float>::infinity();
    ret.surface = nullptr;
    return ret;
  }
};

}
