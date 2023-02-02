#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <string.h>

#include "utils.h"
#include "imath.h"

#ifndef __min
# define __min min
#endif

#ifndef __max
# define __max max
#endif

struct Point {
	real_t alpha, beta, rayon;

	int operator==(Point &b);
	int operator>(Point &b);
	int operator<(Point &b);

	Point();
	Point(real_t a, real_t b, real_t r);
	Point(const Point & a);
};

Point Canonic(const Point &a) {
	Point b;
	b.alpha=fmod(360.0f+a.alpha,360.0f);
	b.beta =fmod(360.0f+a.beta,360.0f);
	b.rayon=a.rayon;
	return b;
}

Point::Point() {
	alpha = -1;
	beta = 0;
	rayon = 0;
}

Point::Point(real_t a, real_t b, real_t r) {
	alpha=a;
	beta =b;
	rayon=r;
}

Point::Point(const Point & a)
 {
  alpha = a.alpha;
  beta = a.beta;
  rayon = a.rayon;
 }

int Point::operator==(Point &b)
 {
  return (alpha==b.alpha) && (beta==b.beta);
 }

int Point::operator>(Point &b)
 {
  return (alpha>b.alpha) || ((alpha==b.alpha) && (beta>b.beta));
 }

int Point::operator<(Point &b)
 {
  return (alpha<b.alpha) || ((alpha==b.alpha) && (beta<b.beta));
 }

// Global variables.
int MaxProf;
int mode=0;   // sharp or smoothed triangles ?

// Look-up table functions and variables.
unsigned int   MaxPoint=0;        // sizeof table
unsigned int   PointPtr=0;       // pointer to next entry past last
unsigned int   dp;              // prime displacement
Point         *PointTable=nullptr; // point table

unsigned int PrimaryHash(Point & a)
 {
  unsigned int p1,p2;
  p1=*((unsigned int *)&a.alpha);
  p2=*((unsigned int *)&a.beta);
  return (p1 ^ (p2 >> 10) ) % MaxPoint;
 }

Point Empty(-1, 0, 0);

unsigned int PosTable(Point & a)
 {
  unsigned int i=PrimaryHash(a);

  while ( (!(PointTable[i]==Empty)) &&
          (!(PointTable[i]==a)))
   i=(i+dp) % MaxPoint;

  return i;
 }

// Checks if a point is in the look-up table
unsigned int PointExiste(Point & a)
 {
  if (PointPtr)
       {
        Point b=Canonic(a);
        unsigned int t=PosTable(b);
        return (PointTable[t]==b);
       }
  else return 0;
 }

// Puts a point in the look-up table
void PutPoint(Point & a)
 {
  Point b=Canonic(a);
  unsigned int t=PosTable(b);
  PointTable[t]=b;
  PointPtr++;
 }

// Gets a point from the look-up table
void GetPoint(Point & a)
 {
  Point b=Canonic(a);
  unsigned int t=PosTable(b);
  a.rayon=PointTable[t].rayon;
 }

// From spherical to rectangular coordinates
const real_t degree = 57.29577951308232;
real_t Polar2Cart_X(const Point & a)
 {
  return (real_t)(a.rayon * sin(a.beta/degree) * cos(a.alpha/degree));
 }

real_t Polar2Cart_Y(const Point & a)
 {
  return (real_t)(a.rayon * cos(a.beta/degree));
 }

real_t Polar2Cart_Z(const Point & a)
 {
  return (real_t)(a.rayon * sin(a.alpha/degree) * sin(a.beta/degree));
 }

Point Normalise(real_t x, real_t y, real_t z)
 {
  Point a;
  real_t norm = sqrt(x*x+y*y+z*z);

  a.alpha = x/norm;
  a.beta  = y/norm;
  a.rayon = z/norm;

  return a;
 }

void CompactPrint(real_t f, int mode)
 {
  int i,j,e;
  char s[20];
  if (mode) {
       sprintf(s,"%.2f",f);
  } else {
	  sprintf(s,"%.5f",f);
  }
  j=i=strlen(s)-1;

  e=i;
  //i--;
  while (s[i]=='0')
   {
    s[i]='_';
    i--;
   }
  if (s[i]=='.') s[i]='_';

  for (i=0;i<j+1;i++)
   if (s[i]!='_')
    printf("%c",s[i]);
 }

void InstancieTriangle(const Point & a, const Point & b, const Point & c)
 {
  if (mode==0)
      {
       printf("triangle{<");
       CompactPrint(Polar2Cart_X(a),0); printf(",");
       CompactPrint(Polar2Cart_Y(a),0); printf(",");
       CompactPrint(Polar2Cart_Z(a),0); printf(">,<");
       CompactPrint(Polar2Cart_X(b),0); printf(",");
       CompactPrint(Polar2Cart_Y(b),0); printf(",");
       CompactPrint(Polar2Cart_Z(b),0); printf(">,<");
       CompactPrint(Polar2Cart_X(c),0); printf(",");
       CompactPrint(Polar2Cart_Y(c),0); printf(",");
       CompactPrint(Polar2Cart_Z(c),0); printf(">}\n");
      }
  else {
        Point N1 = Normalise(Polar2Cart_X(a),Polar2Cart_Y(a),Polar2Cart_Z(a));
        Point N2 = Normalise(Polar2Cart_X(b),Polar2Cart_Y(b),Polar2Cart_Z(b));
        Point N3 = Normalise(Polar2Cart_X(c),Polar2Cart_Y(c),Polar2Cart_Z(c));

        printf("smooth_triangle{<");

        CompactPrint(Polar2Cart_X(a),0); printf(",");
        CompactPrint(Polar2Cart_Y(a),0); printf(",");
        CompactPrint(Polar2Cart_Z(a),0); printf(">,<");

        CompactPrint(N1.alpha,1); printf(",");
        CompactPrint(N1.beta ,1); printf(",");
        CompactPrint(N1.rayon,1); printf(">,<");

        CompactPrint(Polar2Cart_X(b),0); printf(",");
        CompactPrint(Polar2Cart_Y(b),0); printf(",");
        CompactPrint(Polar2Cart_Z(b),0); printf(">,<");

        CompactPrint(N2.alpha,1); printf(",");
        CompactPrint(N2.beta ,1); printf(",");
        CompactPrint(N2.rayon,1); printf(">,<");

        CompactPrint(Polar2Cart_X(c),0); printf(",");
        CompactPrint(Polar2Cart_Y(c),0); printf(",");
        CompactPrint(Polar2Cart_Z(c),0); printf(">,<");

        CompactPrint(N3.alpha,1); printf(",");
        CompactPrint(N3.beta ,1); printf(",");
        CompactPrint(N3.rayon,1); printf(">}\n");

       }
 }

real_t Expos=1.0;

real_t Perturbation(int profondeur)
 {
  return ( (rand()/(real_t)RAND_MAX) - (rand()/(real_t)RAND_MAX) ) /
           exp( Expos*(1+MaxProf-profondeur) );
 }

real_t Perturbe(const Point & a, const Point & b, int profondeur)
 {
  return ((a.rayon+b.rayon)/2) + Perturbation(profondeur);
 }

void Recurse(const Point &a, const Point &b, const Point &c, int profondeur, int Polar = 0)
 {
  if (profondeur)
      {
       // recursons joyeusement
       Point d,e,f;

       if (Polar)
         // north polar region
        {
         d.alpha=b.alpha;
         d.beta=(a.beta+b.beta)/2.0f;
         if (PointExiste(d))
              GetPoint(d);
         else {
               d.rayon=Perturbe(a,b,profondeur);
               PutPoint(d);
              }

         e.alpha=c.alpha;
         e.beta=(a.beta+c.beta)/2.0f;
         if (PointExiste(e))
              GetPoint(e);
         else {
               e.rayon=Perturbe(a,c,profondeur);
               PutPoint(e);
              }

         f=Point( (b.alpha+c.alpha)/2.0f, (b.beta+c.beta)/2.0f, 0.0f );
         if (PointExiste(f))
              GetPoint(f);
         else {
               f.rayon=Perturbe(b,c,profondeur);
               PutPoint(f);
              }

         Recurse(a,d,e,profondeur-1,1);
         Recurse(d,e,f,profondeur-1);
         Recurse(d,b,f,profondeur-1);
         Recurse(e,f,c,profondeur-1);
        }
       else
         {
          d=Point( (a.alpha+b.alpha)/2.0f, (a.beta+b.beta)/2.0f, 0.0f );
          if ( PointExiste(d))
               GetPoint(d);
          else {
                d.rayon=Perturbe(a,b,profondeur);
                PutPoint(d);
               }

          e=Point( (a.alpha+c.alpha)/2.0f,(a.beta+c.beta)/2.0f,0.0f);
          if ( PointExiste(e))
               GetPoint(e);
          else {
                e.rayon=Perturbe(a,c,profondeur);
                PutPoint(e);
               }

          f=Point( (b.alpha+c.alpha)/2.0f,(b.beta+c.beta)/2.0f,0.0f);
          if ( PointExiste(f))
               GetPoint(f);
          else {
                f.rayon=Perturbe(b,c,profondeur);
                PutPoint(f);
               }

          Recurse(a,d,e,profondeur-1);
          Recurse(d,e,f,profondeur-1);
          Recurse(d,b,f,profondeur-1);
          Recurse(e,f,c,profondeur-1);
         }
      }
  else InstancieTriangle(a,b,c);
 }

int rock_gen( int depth, int randseed, real_t smoothness, char mode = '') {
	if (argc>1)
			MaxProf=__max(0,__min(8,atoi(argv[1])));
	else MaxProf=3;

	if (argc>2) {
		if  (argv[2][0]=='+')
				{
				int i = time(NULL);
				fprintf(stderr,"seed:%d\n",i);
				srand(i);
				}
		else srand(atoi(argv[2]));
	}

	if (argc>3) Expos=__max(0.0,(real_t)atof(argv[3]));

	if (argc>4) mode = (argv[4][0]=='+');


	fprintf(stderr,"mesh has %d triangles\n", 20 * ExpoDiscrete(4,MaxProf));
	//MaxPoint = 10 * ExpoDiscrete(2,MaxProf)*(ExpoDiscrete(2,MaxProf)+1);
	MaxPoint = 13 * ExpoDiscrete(2,MaxProf)*(ExpoDiscrete(2,MaxProf)+1);
	fprintf(stderr,"mesh has ~%d summits\n",MaxPoint);

	dp = RelativementPremierPhi(MaxPoint);

	PointTable = new Point[MaxPoint];
	PointPtr=0;

	printf("mesh\n {\n");

		// couronne

		Recurse(Point(0,0,1),Point(-36,60,1),Point( +36,60,1), MaxProf,1);  fprintf(stderr,".");
		Recurse(Point(72,0,1),Point(+36,60,1),Point( 108,60,1), MaxProf,1); fprintf(stderr,".");
		Recurse(Point(144,0,1),Point(108,60,1),Point(180,60,1), MaxProf,1); fprintf(stderr,".");
		Recurse(Point(-72,0,1),Point(-108,60,1),Point(-36,60,1), MaxProf,1); fprintf(stderr,".");
		Recurse(Point(-144,0,1),Point(-180,60,1),Point(-108,60,1), MaxProf,1); fprintf(stderr,".");

		// ceinture
		Recurse(Point(0,120,1),Point(-36,60,1),Point( +36,60,1), MaxProf);  fprintf(stderr,".");
		Recurse(Point(72,120,1),Point(+36,60,1),Point( 108,60,1), MaxProf); fprintf(stderr,".");
		Recurse(Point(144,120,1),Point(108,60,1),Point(180,60,1), MaxProf); fprintf(stderr,".");
		Recurse(Point(-72,120,1),Point(-108,60,1),Point(-36,60,1), MaxProf); fprintf(stderr,".");
		Recurse(Point(-144,120,1),Point(-180,60,1),Point(-108,60,1), MaxProf); fprintf(stderr,".");

		Recurse(Point(36,60,1),Point(0,120,1),Point(72,120,1), MaxProf); fprintf(stderr,".");
		Recurse(Point(108,60,1),Point(72,120,1),Point(144,120,1), MaxProf); fprintf(stderr,".");
		Recurse(Point(-180,60,1),Point(-144,120,1),Point(-216,120,1), MaxProf); fprintf(stderr,".");
		Recurse(Point(-36,60,1),Point(0,120,1),Point(-72,120,1), MaxProf); fprintf(stderr,".");
		Recurse(Point(-108,60,1),Point(-72,120,1),Point(-144,120,1), MaxProf); fprintf(stderr,".");

		// base
		Recurse(Point(36,180,1),Point(0,120,1),Point(72,120,1), MaxProf,1); fprintf(stderr,".");
		Recurse(Point(108,180,1),Point(72,120,1),Point(144,120,1), MaxProf,1); fprintf(stderr,".");
		Recurse(Point(-180,180,1),Point(-144,120,1),Point(-216,120,1), MaxProf,1); fprintf(stderr,".");
		Recurse(Point(-36,180,1),Point(0,120,1),Point(-72,120,1), MaxProf,1); fprintf(stderr,".");
		Recurse(Point(-108,180,1),Point(-72,120,1),Point(-144,120,1), MaxProf,1); fprintf(stderr,".");

	printf(" }\n");

	delete [] PointTable;
}
