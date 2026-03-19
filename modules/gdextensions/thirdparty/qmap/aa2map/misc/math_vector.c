#include <math.h>
#include <float.h>
#include "misc/defines.h"
#include "math_vector.h"


/*
From: John Carmack
Sent: 26 April 2004 19:51
Subject: Re: Origin of fast approximated inverse square root

At 06:38 PM 4/26/2004 +0100, you wrote:

>Hi John,
>
>There's a discussion on Beyond3D.com's forums about who the author of
>the following is:
>
>float InvSqrt (float x){
> float xhalf = 0.5f*x;
> int i = *(int*)
> i = 0x5f3759df - (i>>1);
> x = *(float*)
> x = x*(1.5f - xhalf*x*x);
> return x;
>}
>
>Is that something we can attribute to you? Analysis shows it to be
>extremely clever in its method and supposedly from the Q3 source.
>Most people say it's your work, a few say it's Michael Abrash's. Do
>you know who's responsible, possibly with a history of sorts?

Nope, the reason you want a fast 1/sqrt(x) in graphics is so you can
normalise 3D vectors quickly (something that most 3D apps have to do a LOT).

Slow:
    const float length = sqrt( v.x*v.x + v.y*v.y + v.z*v.z );
    v.x /= length;
    v.y /= length;
    v.z /= length;

Fast:
    const float recip_length = InvSqrt( v.x*v.x + v.y*v.y + v.z*v.z );
    v.x *= recip_length;
    v.y *= recip_length;
    v.z *= recip_length;

The 2nd version has no divides, and no call to sqrt, which makes it *loads*
faster.
*/


void
math_cp (st_vec3_t * o, const st_vec3_t * a)
{
  o->x = a->x;
  o->y = a->y;
  o->z = a->z;
}


void
math_add (st_vec3_t * o, const st_vec3_t * a, const st_vec3_t * b)
{
  o->x = a->x + b->x;
  o->y = a->y + b->y;
  o->z = a->z + b->z;
}


void
math_sub (st_vec3_t * o, const st_vec3_t * a, const st_vec3_t * b)
{
  o->x = a->x - b->x;
  o->y = a->y - b->y;
  o->z = a->z - b->z;
}


void
math_scale (st_vec3_t * o, const st_vec3_t * a, float scale)
{
  o->x = a->x * scale;
  o->y = a->y * scale;
  o->z = a->z * scale;
}


void
math_div (st_vec3_t * o, const st_vec3_t * a, float scale)
{
  o->x = a->x / scale;
  o->y = a->y / scale;
  o->z = a->z / scale;
}


void
math_cross (st_vec3_t * o, const st_vec3_t * a, const st_vec3_t * b)
{
  o->x = a->y * b->z - a->z * b->y;
  o->y = a->z * b->x - a->x * b->z;
  o->z = a->x * b->y - a->y * b->x;
}


float
math_scalar (const st_vec3_t * a, const st_vec3_t * b)
{
  return a->x * b->x + a->y * b->y + a->z * b->z;
}


float
math_angle (const st_vec3_t * a, const st_vec3_t * b)
{
  return MATH_RAD2DEG (acos (math_scalar (a, b)));
}


float
math_mag (const st_vec3_t * a)
{
  return (float) sqrt (math_scalar (a, a));
}


void
math_norm (st_vec3_t * o, const st_vec3_t * a)
{
  st_vec3_t tmp;
  math_cp (&tmp, a);
  math_div (o, &tmp, math_mag (a)); 
}


void
math_minmax (st_vec3_t *min, st_vec3_t *max, st_vec3_t *a, int a_len)
{
  int i = 0;

  min->x =
  min->y =
  min->z = FLT_MAX;
  max->x =
  max->y =
  max->z = -FLT_MAX;

  for (; i < a_len; i++)
    {
      min->x = MIN (min->x, a[i].x);
      min->y = MIN (min->y, a[i].y);
      min->z = MIN (min->z, a[i].z);
      max->x = MAX (max->x, a[i].x);
      max->y = MAX (max->y, a[i].y);
      max->z = MAX (max->z, a[i].z);
    }
}


void
math_center (st_vec3_t *o, st_vec3_t *a, int a_len)                 
{
  st_vec3_t diff, min, max, tmp;

  math_minmax (&min, &max, a, a_len);

  math_sub (&diff, &max, &min);
  math_div (&tmp, &diff, 2.0);
  math_add (o, &min, &tmp);
}


float
math_radius (st_vec3_t *a, int a_len)                 
{
  st_vec3_t diff, min, max, tmp;
  float radius = 0.0;

  math_minmax (&min, &max, a, a_len);

  math_sub (&diff, &max, &min);
  math_div (&tmp, &diff, 2.0);

  radius = MAX (fabs (diff.z), MAX (fabs (diff.x), fabs (diff.y)));
  //should really calculate length of center to extreme point, but just take
  // 75% of this length and that will be ok?
  radius *= 0.75;

  return radius;
}


#if 0
typedef struct m_plane_struct
{
  vec3_t normal;
  float  distance;
} plane_t;


float M_distanceToPlane(plane_t plane, vec3_t point)
{
  return M_vdot(plane.normal, point) + plane.distance;
}


void M_raytracePlane(vec3_t *r, plane_t plane, vec3_t rpos, vec3_t rdir)
{
  float a = M_vdot(plane.normal, rdir), d;

  if(a == 0) // ray is parallel to plane
  {
    *r[0] = rpos[0];
    *r[1] = rpos[1];
    *r[2] = rpos[2];
  }

  M_vsubtract(*r, rpos, rdir);

  d = M_distanceToPlane(plane, rpos)/a;
 
  *r[0] *= d;
  *r[1] *= d;
  *r[2] *= d;  
} 


st_vec3_t *
draw_gl_normal (st_vec3_t * v[])
{
  st_vec3_t v1 = v[2] - v[0];
  st_vec3_t v2 = v[1] - v[0];
  st_vec3_t normal = draw_gl_cross (v1, v2);

  normal = draw_gl_normalize (normal);

  return normal;
}
#endif

