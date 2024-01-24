#include "reflection.hpp"
#include "geometry.hpp"
#include "global.hpp"
#include "world.hpp"
#include <tuple>

namespace eh
{
vec3 MirrorReflection::get_color( Ray const& r, RayHit const& hit, World &w ) const
{
  Ray newray( hit.point(r), hit.reflect(r), r.thread_id );
  newray.bounce = r.bounce + 0.3f;
  return w.get_color(newray).array() * color.array();
}
vec3 FuzzyMirrorReflection::get_color( Ray const& r, RayHit const& hit, World &w ) const
{
  vec3 color = vec3::Zero();
  vec3 reflection = hit.reflect(r);
  int cnt = 0;
  for( int i=0; i<sample_count; ++i )
  {
    float angle = (1-std::sin( w.random01(r.thread_id)*w.PI/2.0f ))*w.PI/2.0f*fuzzyness;
    float angle2 = w.random01(r.thread_id)*w.PI*2;
    float x = std::cos(angle2)*std::sin(angle);
    float y = std::sin(angle2)*std::sin(angle);
    float z = std::cos(angle);
    vec3 unitx, unity;
    std::tie(unitx,unity) = make_unit(reflection);

    Ray newray( hit.point(r), (x*unitx+y*unity+z*reflection).normalized(), r.thread_id );
    newray.bounce = r.bounce + 0.3;
    if( newray.direction().dot(hit.normal) < 0 ){ continue; }

    ++cnt;
    color += w.get_color(newray);
  }
  color = color/(float)std::max(cnt,1);

  return color.array() * this->color.array();
}


// Uniform Lambertial Diffusive Reflection
vec3 DiffuseReflection::get_color( Ray const& r, RayHit const& hit, World &w ) const
{
  // diffusive random rays
  vec3 color = vec3::Zero();
  for( int i=0; i<sample_count; ++i )
  {
    // angle on xy plane
    float angle = w.random01(r.thread_id) * w.PI * 2;

    // sin theta from normal vector
    float sinZ = w.random01(r.thread_id);
    float cosZ = std::sqrt( 1.0f - sinZ*sinZ );

    float z = cosZ;
    float x = sinZ*std::cos(angle);
    float y = sinZ*std::sin(angle);

    // make unit vectors from normal vector
    vec3 unitx, unity;
    std::tie(unitx,unity) = make_unit(hit.normal);
    Ray diffusive_ray( hit.point(r), x*unitx+y*unity+z*hit.normal, r.thread_id );
    diffusive_ray.bounce = r.bounce + 1;
    color += w.get_color(diffusive_ray);
  }
  color = color/(float)sample_count;
  return color.array() * this->color.array();
}
vec3 Refragtion::get_color( Ray const& r,RayHit const& hit, World &w ) const
{
  vec3 n = hit.normal*hit.normal.dot(r.direction());
  vec3 tangent = r.direction() - n;
  float i = index;
  if( hit.normal.dot(r.direction()) > 0 ){ i = 1.0f/i; }
  float a2 = index*index*n.squaredNorm();
  float a1 = 1.0f - index*index*tangent.squaredNorm();
  if( a1 <= GeometryObject::EPSILON )
  {
    // full reflection
    Ray newray( hit.point(r), tangent - n, r.thread_id );
    // newray.bounce = r.bounce;
    newray.bounce = r.bounce + 0.3f;
    vec3 color = w.get_color(newray);
    return color.array() * this->color.array();
  }
  else {
    // refraction
    float alpha = std::sqrt(a2/a1);

    Ray newray( hit.point(r), (n+alpha*tangent).normalized(), r.thread_id );
    newray.bounce = r.bounce;
    vec3 color = w.get_color(newray);
    return color.array() * this->color.array();
  }
}

vec3 CombineReflection::get_color( Ray const& r, RayHit const& hit, World &w ) const
{
  vec3 c1 = r1->get_color(r,hit,w);
  vec3 c2 = r2->get_color(r,hit,w);
  return c1*s1 + c2*s2;
}
vec3 MultiplyReflection::get_color( Ray const& r, RayHit const& hit, World &w ) const
{
  vec3 c1 = r1->get_color(r,hit,w);
  vec3 c2 = r2->get_color(r,hit,w);
  return c1.array() * c2.array();
}
vec3 FaceReflection::get_color( Ray const& r, RayHit const& hit, World &w ) const
{
  if( hit.normal.dot(r.direction()) < 0 )
  {
    return front->get_color(r,hit,w);
  }else {
    return back->get_color(r,hit,w);
  }
}

vec3 LightSource::get_color( Ray const& r, RayHit const& hit, World &w ) const
{
  return this->color;
}


}
