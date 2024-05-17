#pragma once

#include "global.hpp"
#include "math.hpp"
#include "ray.hpp"
#include <cmath>
#include <utility>

namespace eh
{

// bounding box representation for RTree
struct BoundingBox
{
  vec3 min_, max_;
  bool is_inside(vec3 const& p) const
  {
    return (min_.array() < p.array()).all() && (p.array() < max_.array()).all();
  }
  bool is_inside(BoundingBox const& b) const
  {
    return is_inside(b.min_) && is_inside(b.max_);
  }
  bool is_overlap(BoundingBox const& b) const
  {
    return (min_.array() < b.max_.array()).all()
           && (b.min_.array() < max_.array()).all();
  }
  float area() const
  {
    return (max_.x() - min_.x()) * (max_.y() - min_.y())
           * (max_.z() - min_.z());
  }
  bool raycast(Ray const& r, float& tmin, float& tmax) const
  {
    float t0 = -std::numeric_limits<float>::infinity();
    float t1 = std::numeric_limits<float>::infinity();
    for (int i = 0; i < 3; i++)
    {
      float tNear = (min_[i] - r.origin()[i]) * r.inv_direction()[i];
      float tFar = (max_[i] - r.origin()[i]) * r.inv_direction()[i];
      if (tNear > tFar)
      {
        std::swap(tNear, tFar);
      }
      t0 = std::max(t0, tNear);
      t1 = std::min(t1, tFar);
      if (t0 > t1)
      {
        return false;
      }
    }
    tmin = t0;
    tmax = t1;
    if (tmax < 0)
    {
      return false;
    }
    tmin = std::max(tmin, 0.0f);
    return true;
  }
};

// geometry object interface
struct GeometryObject
{
  constexpr static float EPSILON = 1e-3f;

  virtual ~GeometryObject()
  {
  }
  virtual RayHit raycast(Ray const& r) const = 0;
  virtual BoundingBox bounding_box() const = 0;
};

// sphere geometry
struct Sphere : GeometryObject
{
  vec3 center;
  float radius;

  Sphere()
  {
  }
  Sphere(vec3 _center, float _radius)
  {
    center = _center;
    radius = _radius;
  }

  /*
   * p0 : ray origin
   * d : ray direction
   * t : real number
   * c0 : sphere center
   * r : sphere radius
   *
   * (p0 + t*d - c0)^2 = r^2
   *
   * t^2*d^2 + 2*t*d.dot(p0-c0) + (p0-c0)^2 - r^2 = 0
   * --> because d is normalized, d^2 = 1
   * t^2 + 2*t*(p0-c0) + (p0-c0)^2 - r^2 = 0
   */
  RayHit raycast(Ray const& r) const override
  {
    const float half_b = r.direction().dot(r.origin() - center);
    const float c = (r.origin() - center).squaredNorm() - radius * radius;

    float Det = half_b * half_b - c;
    if (Det < EPSILON)
    {
      return RayHit::no_hit();
    }
    Det = std::sqrt(Det);

    // t = (-b \pm Det)/a
    RayHit ret;
    const float t1 = (-half_b - Det);
    const float t2 = (-half_b + Det);
    if (t1 > EPSILON)
    {
      ret.t = t1;
    }
    else if (t2 > EPSILON)
    {
      ret.t = t2;
    }
    else
    {
      return RayHit::no_hit();
    }
    ret.normal = (ret.point(r) - center) / radius;
    ret.surface = reinterpret_cast<Object const*>(1);
    return ret;
  }
  BoundingBox bounding_box() const override
  {
    BoundingBox ret;
    ret.min_ = center - vec3::Constant(radius);
    ret.max_ = center + vec3::Constant(radius);
    return ret;
  }
};

// infinite plane geometry
struct Plane : GeometryObject
{
  vec3 center;
  vec3 normal;

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
  RayHit raycast(Ray const& r) const override
  {
    const float denom = r.direction().dot(normal);
    if (std::abs(denom) < EPSILON)
    {
      return RayHit::no_hit();
    }
    const float t = (center - r.origin()).dot(normal) / denom;
    if (t > EPSILON)
    {
      RayHit ret;
      ret.t = t;
      ret.normal = normal;
      ret.surface = reinterpret_cast<Object const*>(1);
      return ret;
    }
    return RayHit::no_hit();
  }

  BoundingBox bounding_box() const override
  {
    BoundingBox ret;
    ret.min_ = vec3::Constant(-std::numeric_limits<float>::infinity());
    ret.max_ = vec3::Constant(std::numeric_limits<float>::infinity());
    return ret;
  }
};

// triangle geometry
struct Triangle : GeometryObject
{
  vec3 p0, p1, p2;
  vec3 n0, n1, n2;

  Triangle()
  {
  }
  Triangle(vec3 const& _p0,
           vec3 const& _p1,
           vec3 const& _p2,
           vec3 const& _n0,
           vec3 const& _n1,
           vec3 const& _n2)
      : p0(_p0)
      , p1(_p1)
      , p2(_p2)
      , n0(_n0)
      , n1(_n1)
      , n2(_n2)
  {
  }

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
  RayHit raycast(Ray const& r) const override
  {
    // form 3x3 matrix
    const vec3 c0 = p1 - p0;
    const vec3 c1 = p2 - p0;
    const vec3 c2 = -r.direction();
    float det = c0.dot(c1.cross(c2));
    if (std::abs(det) < EPSILON)
    {
      return RayHit::no_hit();
    }
    det = 1.0f / det;
    const vec3 b = r.origin() - p0;
    const vec3 r0 = c1.cross(c2);
    const vec3 r1 = c2.cross(c0);
    const vec3 r2 = c0.cross(c1);

    const float u = det * r0.dot(b);
    const float v = det * r1.dot(b);
    const float t = det * r2.dot(b);

    if (u >= 0 && v >= 0 && u + v <= 1 && t > EPSILON)
    {
      RayHit ret;
      ret.t = t;
      ret.surface = reinterpret_cast<Object const*>(1);
      ret.normal = n0 + (n1 - n0) * u + (n2 - n0) * v;
      ret.normal.normalize();
      return ret;
    }
    return RayHit::no_hit();
  }
  BoundingBox bounding_box() const override
  {
    BoundingBox ret;
    ret.min_ = p0.cwiseMin(p1).cwiseMin(p2);
    ret.max_ = p0.cwiseMax(p1).cwiseMax(p2);
    return ret;
  }
};

}
