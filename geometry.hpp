#pragma once

#include <cmath>
#include <utility>
#include "global.hpp"
#include <iostream>
#include "ray.hpp"
#include "math.hpp"

namespace eh {

struct GeometryObject
{
  constexpr static float RAYHIT_EPS = 1e-3f;

  ReflectionModel *reflection;
  vec3 color;

  virtual ~GeometryObject(){}
  virtual RayHit raycast( Ray const& r ) = 0;
  virtual vec3 get_color( vec3 point ){ return color; }
};

struct Sphere : GeometryObject
{
  vec3 center;
  float radius;

  /*
   * p0 : ray origin
   * d : ray direction
   * t : real number
   * c0 : sphere center
   * r : sphere radius
   *
   * (p0 + t*d - c0)^2 = r^2
   *
   * t^2*d^2 + 2*t*(p0-c0) + (p0-c0)^2 - r^2 = 0
  */
  RayHit raycast( Ray const& r ) override
  {
    const float a = r.direction.dot(r.direction);
    const float half_b = r.direction.dot(r.origin-center);
    const float c = (r.origin-center).dot(r.origin-center) - radius*radius;

    float Det = half_b*half_b - a*c;
    if( Det < 0 )
    {
      return RayHit::no_hit();
    }
    Det = std::sqrt(Det);


    // t = (-b \pm Det)/a
    RayHit ret;
    const float t1 = (-half_b - Det)/a;
    const float t2 = (-half_b + Det)/a;
    if( t1 > RAYHIT_EPS )
    {
      ret.t = t1;
    }else if( t2 > RAYHIT_EPS )
    {
      ret.t = t2;
    }else {
      return RayHit::no_hit();
    }
    ret.normal = (ret.point(r) - center)/radius;
    ret.surface = this;
    return ret;
  }
};

/*
 * p0 : ray origin
 * d : ray direction
 * t : real number
 * c0 : Plane center
 * n : Plane normal
 *
 * ( p0 + t*d - c0 ) dot n = 0
 * t * (d dot n) = (c0 - p0) dot n
 */
struct Plane : GeometryObject
{
  vec3 center;
  vec3 normal;

  RayHit raycast( Ray const& r ) override
  {
    const float denom = r.direction.dot(normal);
    if( std::abs(denom) < 1e-3 )
    {
      return RayHit::no_hit();
    }
    const float t = (center-r.origin).dot(normal)/denom;
    if( t > RAYHIT_EPS )
    {
      RayHit ret;
      ret.t = t;
      ret.normal = normal;
      ret.surface = this;
      return ret;
    }
    return RayHit::no_hit();
  }

  // checkered-tile coloring
  vec3 get_color( vec3 point ) override
  {
    float width = 0.85f;
    int ix = (int)std::floor(point.x()/width);
    int iz = (int)std::floor(point.z()/width);
    if( (ix+iz)&1 )
    {
      return vec3(1,1,1);
    }
    return vec3(0.2f,0.2f,0.2f);
  }
};
struct SkySphere : Sphere
{
  vec3 get_color( vec3 point ) override
  {
    /*
    float angle_around_y = std::atan2(point.z(),point.x());
    int ayi = (int)std::floor( angle_around_y/(0.3141592f) );
    int yi = (int)std::floor( point.y()/(radius*3.141592f/10.0f) );
    if( ((ayi+yi)&1) == 1 )
    {
      return vec3(0.6f,0.8f,1.0f);
    }else {
      return vec3(0.6f,0.8f,1.0f)*1.5f;
    }
    */
    return vec3(1.0f,1.0f,1.0f);
  }
};

/*
 * r0 : ray origin
 * d : ray direction
 * t, u, v : real number
 * p0, p1, p2 : 3 vertices of triangle
 *
 * r0 + t*d = p0 + u*(p1-p0) + v*(p2-p0)
 *
 *                     ( u )
 * ( p1-p0, p2-p0, -d )( v ) = r0 - p0
 *                     ( t )
 */
struct Triangle : GeometryObject
{
  vec3 p0, p1, p2;
  vec3 n0, n1, n2;

  RayHit raycast( Ray const& r ) override
  {
    const vec3 c0 = p1 - p0;
    const vec3 c1 = p2 - p0;
    const vec3 c2 = -r.direction;
    float det = c0.dot( c1.cross(c2) );
    if( std::abs(det) < 1e-3 ){ return RayHit::no_hit(); }
    det = 1.0f / det;
    const vec3 b = r.origin - p0;
    const vec3 r0 = c1.cross(c2);
    const vec3 r1 = c2.cross(c0);
    const vec3 r2 = c0.cross(c1);

    const float u = det*r0.dot(b);
    const float v = det*r1.dot(b);
    const float t = det*r2.dot(b);

    if( u>0 && v>0 && u+v<1 && t>RAYHIT_EPS )
    {
      RayHit ret;
      ret.t = t;
      ret.surface = this;
      ret.normal = n0 + (n1-n0)*u + (n2-n0)*v;
      ret.normal.normalize();
      return ret;
    }
    return RayHit::no_hit();
  }
};

}
