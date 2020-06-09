#ifndef DRAW_H
#define DRAW_H

#include "matrix.h"
#include "ml6.h"
#include "symtab.h"

void
draw_scanline(int, double, int, double, int, screen, zbuffer, color);

void
scanline_convert(struct matrix*, int, screen, zbuffer, color);

void
add_polygon(struct matrix*,
            double,
            double,
            double,
            double,
            double,
            double,
            double,
            double,
            double);

void
draw_polygons(struct matrix*,
              screen,
              zbuffer,
              double*,
              int,
              double[MAX_LIGHTS][2][3],
              color,
              struct constants*);

void
add_box(struct matrix*, double, double, double, double, double, double);

void
add_sphere(struct matrix*, double, double, double, double, int);

struct matrix*
generate_sphere(double, double, double, double, int);

void
add_torus(struct matrix*, double, double, double, double, double, int);

struct matrix*
generate_torus(double, double, double, double, double, int);

void
add_circle(struct matrix*, double, double, double, double, int);
void
add_curve(struct matrix*,
          double,
          double,
          double,
          double,
          double,
          double,
          double,
          double,
          int,
          int);

void
add_point(struct matrix*, double, double, double);

void
add_edge(struct matrix*, double, double, double, double, double, double);
void
draw_lines(struct matrix*, screen, zbuffer, color);

void
draw_line(int, int, double, int, int, double, screen, zbuffer, color);

#endif
