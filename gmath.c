#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "gmath.h"
#include "matrix.h"
#include "ml6.h"
#include "symtab.h"

color
get_lighting(double* normal,
             double* view,
             color alight,
             int lights,
             double light[MAX_LIGHTS][2][3],
             struct constants* reflect)
{
  /*
  Get the lighting values.

  @param: double* normal
  @param: double* view
  @param: color alight
  @param: double light[MAX_LIGHTS][2][3]
  @param: struct constants* reflect

  @return: color
  */
  color a, d, s, i;
  normalize(normal);

  a = calculate_ambient(alight, reflect);

  i.red = a.red;
  i.green = a.green;
  i.blue = a.blue;

  int n;

  for (n = 0; n < lights; n++) {
    d = calculate_diffuse(light[n], reflect, normal);
    s = calculate_specular(light[n], reflect, view, normal);

    i.red = i.red + d.red + s.red;
    i.green = i.green + d.green + s.green;
    i.blue = i.blue + d.blue + s.blue;
  }

  limit_color(&i);

  return i;
}

color
calculate_ambient(color alight, struct constants* reflect)
{
  /*
  Calculate the ambient light values.

  @param: color alight
  @param: struct constants* reflect

  @return: color
  */
  color a;

  a.red = alight.red * reflect->r[AMBIENT_R];
  a.green = alight.green * reflect->g[AMBIENT_R];
  a.blue = alight.blue * reflect->b[AMBIENT_R];

  return a;
}

color
calculate_diffuse(double light[2][3], struct constants* reflect, double* normal)
{
  /*
  Calculate the diffuse light values.

  @param: double light[2][3]
  @param: struct constants* reflect
  @param: double* normal

  @return: color
  */
  color d;
  double dot;
  double lvector[3];

  lvector[0] = light[LOCATION][0];
  lvector[1] = light[LOCATION][1];
  lvector[2] = light[LOCATION][2];

  normalize(lvector);

  dot = dot_product(normal, lvector);

  d.red = (int)(light[COLOR][RED] * reflect->r[DIFFUSE_R] * dot);
  d.green = (int)(light[COLOR][GREEN] * reflect->g[DIFFUSE_R] * dot);
  d.blue = (int)(light[COLOR][BLUE] * reflect->b[DIFFUSE_R] * dot);

  return d;
}

color
calculate_specular(double light[2][3],
                   struct constants* reflect,
                   double* view,
                   double* normal)
{
  /*
  Calculate the specular values.

  @param: double light[2][3]
  @param: struct constants* reflect
  @param: double* view
  @param: double* normal

  @return: color
  */
  color s;
  double lvector[3];
  double result;
  double n[3];

  lvector[0] = light[LOCATION][0];
  lvector[1] = light[LOCATION][1];
  lvector[2] = light[LOCATION][2];
  normalize(lvector);

  result = 2 * dot_product(normal, lvector);
  n[0] = (normal[0] * result) - lvector[0];
  n[1] = (normal[1] * result) - lvector[1];
  n[2] = (normal[2] * result) - lvector[2];

  result = dot_product(n, view);
  result = result > 0 ? result : 0;
  result = pow(result, SPECULAR_EXP);

  s.red = (int)(light[COLOR][RED] * reflect->r[SPECULAR_R] * result);
  s.green = (int)(light[COLOR][GREEN] * reflect->g[SPECULAR_R] * result);
  s.blue = (int)(light[COLOR][BLUE] * reflect->b[SPECULAR_R] * result);

  return s;
}

void
limit_color(color* c)
{
  /*
  Limit the RGB values to 255.

  @param: color* c

  @return: void
  */
  c->red = c->red > 255 ? 255 : c->red;
  c->green = c->green > 255 ? 255 : c->green;
  c->blue = c->blue > 255 ? 255 : c->blue;

  c->red = c->red < 0 ? 0 : c->red;
  c->green = c->green < 0 ? 0 : c->green;
  c->blue = c->blue < 0 ? 0 : c->blue;
}

void
normalize(double* vector)
{
  /*
  Normalize the vector.

  @param: double* vector

  @return: void
  */
  double magnitude;
  magnitude =
    sqrt(vector[0] * vector[0] + vector[1] * vector[1] + vector[2] * vector[2]);
  int i;
  for (i = 0; i < 3; i++) {
    vector[i] = vector[i] / magnitude;
  }
}

double
dot_product(double* a, double* b)
{
  /*
  Find the dot product of vectors a and b.

  @param: double* a
  @param: double* b

  @return: void
  */
  return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

double*
calculate_normal(struct matrix* polygons, int i)
{
  /*
  Calculate the normal vector.

  @param: struct matrix* polygons
  @param: int i

  @return: double*
  */
  double A[3];
  double B[3];
  double* N = (double*)malloc(3 * sizeof(double));

  A[0] = polygons->m[0][i + 1] - polygons->m[0][i];
  A[1] = polygons->m[1][i + 1] - polygons->m[1][i];
  A[2] = polygons->m[2][i + 1] - polygons->m[2][i];

  B[0] = polygons->m[0][i + 2] - polygons->m[0][i];
  B[1] = polygons->m[1][i + 2] - polygons->m[1][i];
  B[2] = polygons->m[2][i + 2] - polygons->m[2][i];

  N[0] = A[1] * B[2] - A[2] * B[1];
  N[1] = A[2] * B[0] - A[0] * B[2];
  N[2] = A[0] * B[1] - A[1] * B[0];

  return N;
}
