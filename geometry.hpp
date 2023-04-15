#pragma once

#include <cmath>
#include <utility>
#include "global.hpp"
#include "ray.hpp"
#include "math.hpp"

namespace eh {

struct GeometryObject
{
  constexpr static float RAYHIT_EPS = 1e-3f;

  ReflectionModel *reflection;
  vec3 color;

  virtual ~GeometryObject(){}
  virtual void raycast( Ray& r ) = 0;
  virtual vec3 get_color( vec3 point ){ return color; }
};

struct Sphere : GeometryObject
{
  vec3 center;
  float radius;

  void raycast( Ray& r ) override
  {
    const float a = r.direction.dot(r.direction);
    const float b = r.direction.dot(r.origin-center);
    const float c = (r.origin-center).dot(r.origin-center) - radius*radius;

    float Det = b*b - a*c;
    if( Det < 0 )
    {
      return;
    }
    Det = std::sqrt(Det);


    // t = (-b \pm Det)/a
    const float t1 = (-b - Det)/a;
    const float t2 = (-b + Det)/a;
    if( t1 > RAYHIT_EPS )
    {
      r.t = t1;
    }else if( t2 > RAYHIT_EPS )
    {
      r.t = t2;
    }else {
      return;
    }

    r.surface = this;
    r.normal = r.point() - center;
    r.normal.normalize();
  }
};

struct Plane : GeometryObject
{
  vec3 center;
  vec3 normal;

  void raycast( Ray& r ) override
  {
    float denom = r.direction.dot(normal);
    if( std::abs(denom) < 1e-3 )
    {
      return;
    }
    float t = (center-r.origin).dot(normal)/denom;
    if( t > RAYHIT_EPS )
    {
      r.normal = normal;
      r.surface = this;
      r.t = t;
    }
  }
  vec3 get_color( vec3 point ) override
  {
    float width = 1.2f;
    int ix = (int)std::floor(point.x()/width);
    int iz = (int)std::floor(point.z()/width);
    if( (ix+iz)&1 )
    {
      return vec3(1,1,1);
    }
    return vec3(0.2f,0.2f,0.2f);
  }
};

}