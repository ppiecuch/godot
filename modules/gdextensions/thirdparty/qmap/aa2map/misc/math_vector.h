#ifndef MISC_MATH_H
#define MISC_MATH_H


#define M_PI_180 0.01745329251994329576  // PI/180.0
#define M_180_PI 57.29577951308232087679 // 180.0/PI


#define MATH_DEG2RAD(x)   ((x) * M_PI_180)
#define MATH_RAD2DEG(x)   ((x) * M_180_PI)


//#define DotProduct(a,b)			((a)[0]*(b)[0]+(a)[1]*(b)[1]+(a)[2]*(b)[2])
//#define VectorSubtract(a,b,c)	((c)[0]=(a)[0]-(b)[0],(c)[1]=(a)[1]-(b)[1],(c)[2]=(a)[2]-(b)[2])
//#define VectorAdd(a,b,c)		((c)[0]=(a)[0]+(b)[0],(c)[1]=(a)[1]+(b)[1],(c)[2]=(a)[2]+(b)[2])
//#define VectorCopy(a,b)			((b)[0]=(a)[0],(b)[1]=(a)[1],(b)[2]=(a)[2])
//#define VectorCopy(a,b)			((b).x=(a).x,(b).y=(a).y,(b).z=(a).z])

//#define	VectorScale(v, s, o)	((o)[0]=(v)[0]*(s),(o)[1]=(v)[1]*(s),(o)[2]=(v)[2]*(s))
#define	__VectorMA(v, s, b, o)	((o)[0]=(v)[0]+(b)[0]*(s),(o)[1]=(v)[1]+(b)[1]*(s),(o)[2]=(v)[2]+(b)[2]*(s))
//#define CrossProduct(a,b,c)		((c)[0]=(a)[1]*(b)[2]-(a)[2]*(b)[1],(c)[1]=(a)[2]*(b)[0]-(a)[0]*(b)[2],(c)[2]=(a)[0]*(b)[1]-(a)[1]*(b)[0])

#define DotProduct4(x,y)		((x)[0]*(y)[0]+(x)[1]*(y)[1]+(x)[2]*(y)[2]+(x)[3]*(y)[3])
#define VectorSubtract4(a,b,c)	((c)[0]=(a)[0]-(b)[0],(c)[1]=(a)[1]-(b)[1],(c)[2]=(a)[2]-(b)[2],(c)[3]=(a)[3]-(b)[3])
#define VectorAdd4(a,b,c)		((c)[0]=(a)[0]+(b)[0],(c)[1]=(a)[1]+(b)[1],(c)[2]=(a)[2]+(b)[2],(c)[3]=(a)[3]+(b)[3])
#define VectorCopy4(a,b)		((b)[0]=(a)[0],(b)[1]=(a)[1],(b)[2]=(a)[2],(b)[3]=(a)[3])
#define	VectorScale4(v, s, o)	((o)[0]=(v)[0]*(s),(o)[1]=(v)[1]*(s),(o)[2]=(v)[2]*(s),(o)[3]=(v)[3]*(s))
#define	VectorMA4(v, s, b, o)	((o)[0]=(v)[0]+(b)[0]*(s),(o)[1]=(v)[1]+(b)[1]*(s),(o)[2]=(v)[2]+(b)[2]*(s),(o)[3]=(v)[3]+(b)[3]*(s))


//#define VectorClear(a)			((a)[0]=(a)[1]=(a)[2]=0)
#define VectorNegate(a,b)		((b)[0]=-(a)[0],(b)[1]=-(a)[1],(b)[2]=-(a)[2])
//#define VectorSet(v, x, y, z)	((v)[0]=(x), (v)[1]=(y), (v)[2]=(z))
#define Vector4Copy(a,b)		((b)[0]=(a)[0],(b)[1]=(a)[1],(b)[2]=(a)[2],(b)[3]=(a)[3])

#define	SnapVector(v) {v[0]=(int)v[0];v[1]=(int)v[1];v[2]=(int)v[2];}



typedef struct
{
  float x, y, z;
} st_vec3_t;


/*
  math_cp()     o=a
  math_add()    o=a+b
  math_sub()    o=a-b
  math_scale()  o=a*s
  math_div()    o=a/s

  math_cross()
  math_scalar() scalar (dot) product   
  math_norm()   unit vector for a vector
  math_angle()  angle between two vectors, in 2D space
  math_mag()    magnitude
  math_minmax() axisbound min and max coords of an array (model)
  math_center() center of an array (model)
  math_radius() bounding sphere radius of an array (model)
                  (e.g. for fast but bad collision detection)
*/
void math_cp (st_vec3_t * o, const st_vec3_t * a);   
void math_add (st_vec3_t * o, const st_vec3_t * a, const st_vec3_t * b);
void math_sub (st_vec3_t * o, const st_vec3_t * a, const st_vec3_t * b);
void math_scale (st_vec3_t * o, const st_vec3_t * a, float scale);        
void math_div (st_vec3_t * o, const st_vec3_t * a, float scale);        

extern void math_cross (st_vec3_t * o, const st_vec3_t * a, const st_vec3_t * b);
extern float math_scalar (const st_vec3_t * a, const st_vec3_t * b);
#define math_dot math_scalar
extern void math_norm (st_vec3_t * o, const st_vec3_t * a);
extern float math_angle (const st_vec3_t * a, const st_vec3_t * b);
extern float math_mag (const st_vec3_t * a);
extern void math_minmax (st_vec3_t *min, st_vec3_t *max, st_vec3_t *a, int a_len);
extern void math_center (st_vec3_t *o, st_vec3_t *a, int a_len);
extern float math_radius (st_vec3_t *a, int a_len);


#endif // MISC_MATH_H
