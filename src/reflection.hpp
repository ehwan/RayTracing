#pragma once

#include "global.hpp"
#include "math.hpp"

namespace eh
{

/*
  reflection model interface

  defines objects' material
  such as, Mirror, Uniform Diffusion, Fuzzy Diffusion( like metal )
  and light source
*/
struct ReflectionModel
{
  // color absorbtion rate
  vec3 color = vec3::Zero();

  // number of random rays
  // only if this ray is diffusive ( shoot ray to random direction and take
  // average of them )
  int sample_count = 1;

  ReflectionModel()
  {
  }
  ReflectionModel(vec3 _color, int _sample_count = 1)
  {
    color = _color;
    sample_count = _sample_count;
  }

  virtual ~ReflectionModel()
  {
  }

  // r : input rays ( from eye to object )
  // hit : raycast result of ray toward object
  // calculate colors for ray
  virtual vec3 get_color(Ray const& r, RayHit const& hit, World& world) const
  {
    return color;
  }
};

// fully mirrored reflection
struct MirrorReflection : ReflectionModel
{
  vec3 get_color(Ray const& r, RayHit const& hit, World& world) const override;
};
struct FuzzyMirrorReflection : ReflectionModel
{
  float fuzzyness = 0.0f;
  vec3 get_color(Ray const& r, RayHit const& hit, World& world) const override;
};

// random diffuse reflection
struct DiffuseReflection : ReflectionModel
{
  vec3 get_color(Ray const& r, RayHit const& hit, World& world) const override;
};

struct Refragtion : ReflectionModel
{
  float index = 1.0f;
  vec3 get_color(Ray const& r, RayHit const& hit, World& world) const override;
};

struct CombineReflection : ReflectionModel
{
  ReflectionModel *r1, *r2;
  float s1 = 0.5f, s2 = 0.5f;

  vec3 get_color(Ray const& r, RayHit const& hit, World& world) const override;
};

struct MultiplyReflection : ReflectionModel
{
  ReflectionModel *r1, *r2;

  vec3 get_color(Ray const& r, RayHit const& hit, World& world) const override;
};

// difference reflection model between front and back
struct FaceReflection : ReflectionModel
{
  ReflectionModel *front, *back;
  vec3 get_color(Ray const& r, RayHit const& hit, World& world) const override;
};

// light source that omit constant light
struct LightSource : ReflectionModel
{
  LightSource(vec3 col)
  {
    color = col;
  }
  vec3 get_color(Ray const& r, RayHit const& hit, World& world) const override;
};

}
