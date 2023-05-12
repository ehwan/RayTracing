#include "reflection.hpp"
#include "global.hpp"
#include "world.hpp"
#include <tuple>

namespace eh
{

vec3 MirrorReflection::get_color( Ray &r, World &w, GeometryObject &g )
{
  Ray newray = r.reflect();
  newray.bounce = r.bounce + 1;
  return w.get_color(newray).array()*g.get_color(r.point()).array();
}
vec3 FuzzyMirrorReflection::get_color( Ray &r, World &w, GeometryObject &g )
{
  vec3 color = vec3::Zero();
  vec3 reflection = r.direction - 2*r.normal*r.normal.dot(r.direction);
  int sample_count = w.sample_count;
  for( int i=0; i<sample_count; ++i )
  {
    Ray newray;
    float angle = std::sin( w.uniform_dist(w.mt_twister)*w.PI/2.0f )*w.PI/2.0f*fuzzyness;
    float angle2 = w.uniform_dist(w.mt_twister);
    float z = std::cos(angle);
    float x = std::cos(angle2)*std::sin(angle);
    float y = std::sin(angle2)*std::sin(angle);
    vec3 unitx, unity;
    std::tie(unitx,unity) = make_unit(reflection);
    newray.direction = x*unitx + y*unity + z*reflection;
    if( newray.direction.dot(r.normal) < 0 ){ continue; }
    newray.bounce = r.bounce + 1;
    newray.origin = r.point();

    color += w.get_color(newray);
  }
  color = color/(float)sample_count;

  return color.array()*g.get_color(r.point()).array();
}

vec3 DiffuseReflection::get_color( Ray &r, World &w, GeometryObject &g )
{
  // diffusive random rays
  vec3 color = vec3::Zero();
  for( int i=0; i<w.sample_count; ++i )
  {
    Ray diffusive_ray;
    diffusive_ray.origin = r.point();
    float angle = std::sin( w.uniform_dist(w.mt_twister)*w.PI/2.0f )*w.PI/2.0f;
    float angle2 = w.uniform_dist(w.mt_twister);
    float z = std::cos(angle);
    float x = std::cos(angle2)*std::sin(angle);
    float y = std::sin(angle2)*std::sin(angle);
    vec3 unitx, unity;
    std::tie(unitx,unity) = make_unit(r.normal);
    diffusive_ray.direction = x*unitx + y*unity + z*r.normal;
    // diffusive_ray.direction = w.random_sphere() + r.normal;
    // diffusive_ray.direction.normalize();
    diffusive_ray.bounce = r.bounce + 1;

    color += w.get_color(diffusive_ray);
  }
  color = color/(float)w.sample_count;
  return color.array() * g.get_color(r.point()).array();
}
vec3 Refragtion::get_color( Ray &r, World &w, GeometryObject &g )
{
  vec3 n = r.normal*r.normal.dot(r.direction);
  vec3 tangent = r.direction - n;
  float i = index;
  if( r.normal.dot(r.direction) > 0 ){ i = 1.0f/i; }
  float a2 = index*index*n.squaredNorm();
  float a1 = 1.0f - index*index*tangent.squaredNorm();
  if( a1 <= 0 )
  {
    // full reflection
    Ray newray;
    newray.origin = r.point();
    newray.direction = tangent - n;
    // newray.bounce = r.bounce;
    newray.bounce = r.bounce + 0.6f;
    return w.get_color(newray);
  }
  else {
    // refraction
    float alpha = std::sqrt(a2/a1);
    Ray newray;
    newray.origin = r.point();
    newray.direction = n + alpha * tangent;
    newray.direction.normalize();
    newray.bounce = r.bounce + 0.3;
    vec3 c = w.get_color(newray);
    return c.array() * g.get_color(r.point()).array();
  }
}

vec3 CombineReflection::get_color( Ray &r, World &w, GeometryObject &g )
{
  vec3 c1 = r1->get_color(r,w,g);
  vec3 c2 = r2->get_color(r,w,g);
  return c1*s1 + c2*s2;
}
vec3 FaceReflection::get_color( Ray &r, World &w, GeometryObject &g )
{
  if( r.normal.dot(r.direction) < 0 )
  {
    return front->get_color(r,w,g);
  }else {
    return back->get_color(r,w,g);
  }
}

vec3 DirectionalLightSource::get_color( Ray &r, World &w, GeometryObject &g )
{
  float coeff = std::abs(r.direction.dot( r.normal ));
  return coeff * g.get_color(r.point());
}


}