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
    r.normal = (r.point() - center)/radius;
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

struct Triangle : GeometryObject
{
  vec3 p0, p1, p2;
  vec3 n0, n1, n2;

  vec3 c0, c1, c2;
  vec3 r0, r1, r2;
  vec3 b;
  float det;
  float u, v, t;

  void raycast( Ray &r ) override
  {
    c0 = p1 - p0;
    c1 = p2 - p0;
    c2 = -r.direction;
    det = c0.dot( c1.cross(c2) );
    if( std::abs(det) < 1e-3 ){ return; }
    det = 1.0f / det;
    b = r.origin - p0;
    r0 = c1.cross(c2);
    r1 = c2.cross(c0);
    r2 = c0.cross(c1);

    u = det*r0.dot(b);
    v = det*r1.dot(b);
    t = det*r2.dot(b);

    if( u>0 && v>0 && u+v<1 && t>RAYHIT_EPS )
    {
      r.t = t;
      r.surface = this;
      r.normal = n0 + (n1-n0)*u + (n2-n0)*v;
      r.normal.normalize();
    }
  }
  vec3 get_color( vec3 point ) override
  {
    return color;
  }
};

}
