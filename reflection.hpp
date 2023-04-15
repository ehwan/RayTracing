#pragma once

/*
  defines objects' material;
  such as, Mirror, Uniform Diffusion, Fuzzy Diffusion( like metal )
  and light source
*/
#include "global.hpp"
#include "math.hpp"

namespace eh
{

struct ReflectionModel
{
  virtual ~ReflectionModel(){}

  // r : input rays
  // calculate colors ray out to $r$
  virtual vec3 get_color( Ray&r, World &w, GeometryObject& g ) = 0;
};

struct MirrorReflection : ReflectionModel
{
  vec3 get_color( Ray &r, World &w, GeometryObject &g ) override;
};
struct FuzzyMirrorReflection : ReflectionModel
{
  float fuzzyness = 0.0f;;
  vec3 get_color( Ray &r, World &w, GeometryObject &g ) override;
};

struct DiffuseReflection : ReflectionModel
{
  vec3 get_color( Ray &r, World &w, GeometryObject &g ) override;
};

// light source that omit directional light
struct DirectionalLightSource : ReflectionModel
{
  vec3 source_color;
  vec3 get_color( Ray &r, World &w, GeometryObject &g ) override;
};

}