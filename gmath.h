#ifndef GMATH_H
#define GMATH_H

#include "matrix.h"
#include "ml6.h"
#include "symtab.h"

#define LOCATION 0
#define COLOR 1
#define RED 0
#define GREEN 1
#define BLUE 2

#define SPECULAR_EXP 4

color
get_lighting(double*,
             double*,
             color,
             int,
             double light[MAX_LIGHTS][2][3],
             struct constants*);

color
calculate_ambient(color, struct constants*);

color
calculate_diffuse(double light[2][3], struct constants*, double*);

color
calculate_specular(double light[2][3], struct constants*, double*, double*);

void
limit_color(color*);

void
normalize(double*);

double
dot_product(double*, double*);

double*
calculate_normal(struct matrix*, int);

#endif
