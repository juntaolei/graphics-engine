#include <stdio.h>
#include <stdlib.h>

#include "display.h"
#include "draw.h"
#include "gmath.h"
#include "math.h"
#include "matrix.h"
#include "ml6.h"
#include "symtab.h"

void
draw_scanline(int x0,
              double z0,
              int x1,
              double z1,
              int y,
              screen s,
              zbuffer zb,
              color c)
{
  /*
  Line algorithm specifically for horizontal scanlines.

  @param: struct matrix *points
  @param: int i
  @param: screen s
  @param: zbuffer zb
  @param: color c

  @return: void
  */
  int tx, tz;

  if (x0 > x1) {
    tx = x0;
    tz = z0;
    x0 = x1;
    z0 = z1;
    x1 = tx;
    z1 = tz;
  }

  double delta_z;
  delta_z = (x1 - x0) != 0 ? (z1 - z0) / (x1 - x0 + 1) : 0;
  int x;
  double z = z0;

  for (x = x0; x <= x1; x++) {
    plot(s, zb, c, x, y, z);
    z += delta_z;
  }
}

void
scanline_convert(struct matrix* points, int i, screen s, zbuffer zb, color il)
{
  /*
  Fills in polygon i by drawing consecutive horizontal (or vertical) lines.

  @param: struct matrix *points
  @param: int i
  @param: screen s
  @param: zbuffer zb
  @param: color il

  @return: void
  */
  int top, mid, bot, y;
  int distance0, distance1, distance2;
  double x0, x1, y0, y1, y2, dx0, dx1, z0, z1, dz0, dz1;
  int flip = 0;

  z0 = z1 = dz0 = dz1 = 0;

  y0 = points->m[1][i];
  y1 = points->m[1][i + 1];
  y2 = points->m[1][i + 2];

  if (y0 <= y1 && y0 <= y2) {
    bot = i;
    if (y1 <= y2) {
      mid = i + 1;
      top = i + 2;
    } else {
      mid = i + 2;
      top = i + 1;
    }
  } else if (y1 <= y0 && y1 <= y2) {
    bot = i + 1;
    if (y0 <= y2) {
      mid = i;
      top = i + 2;
    } else {
      mid = i + 2;
      top = i;
    }
  } else {
    bot = i + 2;
    if (y0 <= y1) {
      mid = i;
      top = i + 1;
    } else {
      mid = i + 1;
      top = i;
    }
  }

  x0 = points->m[0][bot];
  x1 = points->m[0][bot];
  z0 = points->m[2][bot];
  z1 = points->m[2][bot];
  y = (int)(points->m[1][bot]);

  distance0 = (int)(points->m[1][top]) - y + 1;
  distance1 = (int)(points->m[1][mid]) - y + 1;
  distance2 = (int)(points->m[1][top]) - (int)(points->m[1][mid]) + 1;

  dx0 = distance0 > 0 ? (points->m[0][top] - points->m[0][bot]) / distance0 : 0;
  dx1 = distance1 > 0 ? (points->m[0][mid] - points->m[0][bot]) / distance1 : 0;
  dz0 = distance0 > 0 ? (points->m[2][top] - points->m[2][bot]) / distance0 : 0;
  dz1 = distance1 > 0 ? (points->m[2][mid] - points->m[2][bot]) / distance1 : 0;

  while (y <= (int)points->m[1][top]) {
    if (!flip && y >= (int)(points->m[1][mid])) {
      flip = 1;
      dx1 =
        distance2 > 0 ? (points->m[0][top] - points->m[0][mid]) / distance2 : 0;
      dz1 =
        distance2 > 0 ? (points->m[2][top] - points->m[2][mid]) / distance2 : 0;
      x1 = points->m[0][mid];
      z1 = points->m[2][mid];
    }

    draw_scanline(x0, z0, x1, z1, y, s, zb, il);

    x0 += dx0;
    x1 += dx1;
    z0 += dz0;
    z1 += dz1;
    y++;
  }
}

void
add_polygon(struct matrix* polygons,
            double x0,
            double y0,
            double z0,
            double x1,
            double y1,
            double z1,
            double x2,
            double y2,
            double z2)
{
  /*
  Adds the vertices (x0, y0, z0), (x1, y1, z1) and (x2, y2, z2) to the polygon
  matrix. They define a single triangle surface.

  @param: struct matrix *polygons
  @param: double x0
  @param: double y0
  @param: double z0
  @param: double x1
  @param: double y1
  @param: double z1
  @param: double x2
  @param: double y2
  @param: double z2

  @return: void
  */
  add_point(polygons, x0, y0, z0);
  add_point(polygons, x1, y1, z1);
  add_point(polygons, x2, y2, z2);
}

void
draw_polygons(struct matrix* polygons,
              screen s,
              zbuffer zb,
              double* view,
              int lights,
              double light[MAX_LIGHTS][2][3],
              color ambient,
              struct constants* reflect)
{
  /*
  Goes through polygons 3 points at a time, drawing lines connecting each points
  to create bounding triangles.

  @param: struct matrix *polygons
  @param: screen s
  @param: color c

  @return: void
  */
  if (polygons->lastcol < 3) {
    printf("Need at least 3 points to draw a polygon!\n");
    return;
  }

  int point;
  double* normal;

  for (point = 0; point < polygons->lastcol - 2; point += 3) {
    normal = calculate_normal(polygons, point);

    if (normal[2] > 0) {
      color i = get_lighting(normal, view, ambient, lights, light, reflect);
      scanline_convert(polygons, point, s, zb, i);
    }
  }
}

void
add_box(struct matrix* polygons,
        double x,
        double y,
        double z,
        double width,
        double height,
        double depth)
{
  /*
  Add the points for a rectagular prism whose upper-left-front corner is (x, y,
  z) with width, height and depth dimensions.

  @param: struct matrix * edges
  @param: double x
  @param: double y
  @param: double z
  @param: double width
  @param: double height
  @param: double depth

  @return: void
  */
  double x0, y0, z0, x1, y1, z1;

  x0 = x;
  x1 = x + width;
  y0 = y;
  y1 = y - height;
  z0 = z;
  z1 = z - depth;

  add_polygon(polygons, x, y, z, x1, y1, z, x1, y, z);
  add_polygon(polygons, x, y, z, x, y1, z, x1, y1, z);

  add_polygon(polygons, x1, y, z1, x, y1, z1, x, y, z1);
  add_polygon(polygons, x1, y, z1, x1, y1, z1, x, y1, z1);

  add_polygon(polygons, x1, y, z, x1, y1, z1, x1, y, z1);
  add_polygon(polygons, x1, y, z, x1, y1, z, x1, y1, z1);

  add_polygon(polygons, x, y, z1, x, y1, z, x, y, z);
  add_polygon(polygons, x, y, z1, x, y1, z1, x, y1, z);

  add_polygon(polygons, x, y, z1, x1, y, z, x1, y, z1);
  add_polygon(polygons, x, y, z1, x, y, z, x1, y, z);

  add_polygon(polygons, x, y1, z, x1, y1, z1, x1, y1, z);
  add_polygon(polygons, x, y1, z, x, y1, z1, x1, y1, z1);
}

/*======== void add_sphere() ==========
  Inputs:   struct matrix * points
            double cx
            double cy
            double cz
            double r
            int step

  adds all the points for a sphere with center (cx, cy, cz)
  and radius r using step points per circle/semicircle.

  Since edges are drawn using 2 points, add each point twice,
  or add each point and then another point 1 pixel away.

  should call generate_sphere to create the necessary points
  ====================*/
void
add_sphere(struct matrix* edges,
           double cx,
           double cy,
           double cz,
           double r,
           int step)
{
  /*
  Adds all the points for a sphere with center (cx, cy, cz) and radius r using
  step points per circle/semicircle. Since edges are drawn using 2 points, add
  each point twice, or add each point and then another point 1 pixel away.
  should call generate_sphere to create the necessary points

  @param: struct matrix * points
  @param: double cx
  @param: double cy
  @param: double cz
  @param: double r
  @param: int step

  @return: void
  */
  struct matrix* points = generate_sphere(cx, cy, cz, r, step);
  int p0, p1, p2, p3, lat, longt;
  int latStop, longStop, latStart, longStart;

  latStart = 0;
  latStop = step;
  longStart = 1;
  longStop = step;

  for (lat = latStart; lat < latStop; lat++) {
    for (longt = longStart; longt < longStop; longt++) {
      p0 = lat * (step + 1) + longt;
      p1 = p0 + 1;
      p2 = (p1 + step) % (step * (step + 1));
      p3 = (p0 + step) % (step * (step + 1));

      add_polygon(edges,
                  points->m[0][p0],
                  points->m[1][p0],
                  points->m[2][p0],
                  points->m[0][p1],
                  points->m[1][p1],
                  points->m[2][p1],
                  points->m[0][p2],
                  points->m[1][p2],
                  points->m[2][p2]);
      add_polygon(edges,
                  points->m[0][p0],
                  points->m[1][p0],
                  points->m[2][p0],
                  points->m[0][p2],
                  points->m[1][p2],
                  points->m[2][p2],
                  points->m[0][p3],
                  points->m[1][p3],
                  points->m[2][p3]);
    }
  }
  free_matrix(points);
}

struct matrix*
generate_sphere(double cx, double cy, double cz, double r, int step)
{
  /*
  Generates all the points along the surface of a sphere with center (cx, cy,
  cz) and radius r using step points per circle/semicircle. Returns a matrix of
  those points.

  @param: struct matrix * points
  @param: double cx
  @param: double cy
  @param: double cz
  @param: double r
  @param: int step

  @return: struct matrix*
  */
  struct matrix* points = new_matrix(4, step * step);
  int circle, rotation, rot_start, rot_stop, circ_start, circ_stop;
  double x, y, z, rot, circ;

  rot_start = 0;
  rot_stop = step;
  circ_start = 0;
  circ_stop = step;

  for (rotation = rot_start; rotation < rot_stop; rotation++) {
    rot = (double)rotation / step;

    for (circle = circ_start; circle <= circ_stop; circle++) {
      circ = (double)circle / step;

      x = r * cos(M_PI * circ) + cx;
      y = r * sin(M_PI * circ) * cos(2 * M_PI * rot) + cy;
      z = r * sin(M_PI * circ) * sin(2 * M_PI * rot) + cz;

      add_point(points, x, y, z);
    }
  }

  return points;
}

void
add_torus(struct matrix* edges,
          double cx,
          double cy,
          double cz,
          double r1,
          double r2,
          int step)
{
  /*
  Adds all the points required for a torus with center (cx, cy, cz), circle
  radius r1 and torus radius r2 using step points per circle. Should call
  generate_torus to create the necessary points.

  @param: struct matrix * points
  @param: double cx
  @param: double cy
  @param: double cz
  @param: double r1
  @param: double r2
  @param: double step

  @return: void
  */
  struct matrix* points = generate_torus(cx, cy, cz, r1, r2, step);
  int p0, p1, p2, p3, lat, longt;
  int latStop, longStop, latStart, longStart;

  latStart = 0;
  latStop = step;
  longStart = 0;
  longStop = step;

  for (lat = latStart; lat < latStop; lat++) {
    for (longt = longStart; longt < longStop; longt++) {
      p0 = lat * step + longt;

      if (longt == step - 1)
        p1 = p0 - longt;
      else
        p1 = p0 + 1;

      p2 = (p1 + step) % (step * step);
      p3 = (p0 + step) % (step * step);

      add_polygon(edges,
                  points->m[0][p0],
                  points->m[1][p0],
                  points->m[2][p0],
                  points->m[0][p3],
                  points->m[1][p3],
                  points->m[2][p3],
                  points->m[0][p2],
                  points->m[1][p2],
                  points->m[2][p2]);
      add_polygon(edges,
                  points->m[0][p0],
                  points->m[1][p0],
                  points->m[2][p0],
                  points->m[0][p2],
                  points->m[1][p2],
                  points->m[2][p2],
                  points->m[0][p1],
                  points->m[1][p1],
                  points->m[2][p1]);
    }
  }
  free_matrix(points);
}

struct matrix*
generate_torus(double cx, double cy, double cz, double r1, double r2, int step)
{
  /*
  Generates all the points along the surface of a torus with center (cx, cy,
  cz), circle radius r1 and torus radius r2 using step points per circle.
  Returns a matrix of those points.

  @param: struct matrix * points
  @param: double cx
  @param: double cy
  @param: double cz
  @param: double r
  @param: int step

  @return: struct matrix*
  */
  struct matrix* points = new_matrix(4, step * step);
  int circle, rotation, rot_start, rot_stop, circ_start, circ_stop;
  double x, y, z, rot, circ;

  rot_start = 0;
  rot_stop = step;
  circ_start = 0;
  circ_stop = step;

  for (rotation = rot_start; rotation < rot_stop; rotation++) {
    rot = (double)rotation / step;

    for (circle = circ_start; circle < circ_stop; circle++) {
      circ = (double)circle / step;

      x = cos(2 * M_PI * rot) * (r1 * cos(2 * M_PI * circ) + r2) + cx;
      y = r1 * sin(2 * M_PI * circ) + cy;
      z = -1 * sin(2 * M_PI * rot) * (r1 * cos(2 * M_PI * circ) + r2) + cz;

      add_point(points, x, y, z);
    }
  }
  return points;
}

/*======== void add_circle() ==========
  Inputs:   struct matrix * edges
            double cx
            double cy
            double r
            double step

  Adds the circle at (cx, cy) with radius r to edges
  ====================*/
void
add_circle(struct matrix* edges,
           double cx,
           double cy,
           double cz,
           double r,
           int step)
{
  /*
  Adds the circle at (cx, cy) with radius r to edges.

  @param: struct matrix * edges
  @param: double cx
  @param: double cy
  @param: double r
  @param: double step

  @return: void
  */
  double x0, y0, x1, y1, t;
  int i;

  x0 = r + cx;
  y0 = cy;

  for (i = 1; i <= step; i++) {
    t = (double)i / step;
    x1 = r * cos(2 * M_PI * t) + cx;
    y1 = r * sin(2 * M_PI * t) + cy;

    add_edge(edges, x0, y0, cz, x1, y1, cz);

    x0 = x1;
    y0 = y1;
  }
}

void
add_curve(struct matrix* edges,
          double x0,
          double y0,
          double x1,
          double y1,
          double x2,
          double y2,
          double x3,
          double y3,
          int step,
          int type)
{
  /*
  Adds the curve bounded by the 4 points passsed as parameters of type specified
  in type (see matrix.h for curve type constants) to the matrix edges.

  @param: struct matrix *edges
  @param: double x0
  @param: double y0
  @param: double x1
  @param: double y1
  @param: double x2
  @param: double y2
  @param: double x3
  @param: double y3
  @param: double step

  @return: void
  int type
  */
  double t, x, y;
  int i;
  struct matrix* xcoefs;
  struct matrix* ycoefs;

  xcoefs = generate_curve_coefs(x0, x1, x2, x3, type);
  ycoefs = generate_curve_coefs(y0, y1, y2, y3, type);

  for (i = 1; i <= step; i++) {
    t = (double)i / step;

    x = xcoefs->m[0][0] * t * t * t + xcoefs->m[1][0] * t * t +
        xcoefs->m[2][0] * t + xcoefs->m[3][0];
    y = ycoefs->m[0][0] * t * t * t + ycoefs->m[1][0] * t * t +
        ycoefs->m[2][0] * t + ycoefs->m[3][0];

    add_edge(edges, x0, y0, 0, x, y, 0);
    x0 = x;
    y0 = y;
  }

  free_matrix(xcoefs);
  free_matrix(ycoefs);
}

void
add_point(struct matrix* points, double x, double y, double z)
{
  /*
  Adds point (x, y, z) to points and increment point's lastcol if points is
  full. Should call grow on points.

  @param: struct matrix * points
  @param: int x
  @param: int y
  @param: int z

  @return: void
  */
  if (points->lastcol == points->cols)
    grow_matrix(points, points->lastcol + 100);

  points->m[0][points->lastcol] = x;
  points->m[1][points->lastcol] = y;
  points->m[2][points->lastcol] = z;
  points->m[3][points->lastcol] = 1;
  points->lastcol++;
}

void
add_edge(struct matrix* points,
         double x0,
         double y0,
         double z0,
         double x1,
         double y1,
         double z1)
{
  /*
  Add the line connecting (x0, y0, z0) to (x1, y1, z1) to points.

  @param: struct matrix * points
  @param: int x0
  @param: int y0
  @param: int z0
  @param: int x1
  @param: int y1
  @param: int z1

  @return: void
  */
  add_point(points, x0, y0, z0);
  add_point(points, x1, y1, z1);
}

void
draw_lines(struct matrix* points, screen s, zbuffer zb, color c)
{
  /*
  Go through points 2 at a time and call draw_line to add that line to the
  screen.

  @param: struct matrix * points
  @param: screen s
  @param: color c

  @return: void
  */
  if (points->lastcol < 2) {
    printf("Need at least 2 points to draw a line!\n");
    return;
  }

  int point;

  for (point = 0; point < points->lastcol - 1; point += 2)
    draw_line(points->m[0][point],
              points->m[1][point],
              points->m[2][point],
              points->m[0][point + 1],
              points->m[1][point + 1],
              points->m[2][point + 1],
              s,
              zb,
              c);
}

void
draw_line(int x0,
          int y0,
          double z0,
          int x1,
          int y1,
          double z1,
          screen s,
          zbuffer zb,
          color c)
{
  /*
  Implement Bresenham's line algorithm.

  @param: int x0,
  @param: int y0,
  @param: double z0,
  @param: int x1,
  @param: int y1,
  @param: double z1,
  @param: screen s,
  @param: zbuffer zb,
  @param: color c

  @return: void
  */
  int x, y, d, A, B;
  int dy_east, dy_northeast, dx_east, dx_northeast, d_east, d_northeast;
  int loop_start, loop_end;
  double distance;
  double z, dz;

  int xt, yt;
  if (x0 > x1) {
    xt = x0;
    yt = y0;
    x0 = x1;
    y0 = y1;
    z0 = z1;
    x1 = xt;
    y1 = yt;
  }

  x = x0;
  y = y0;
  A = 2 * (y1 - y0);
  B = -2 * (x1 - x0);

  int wide = 0;
  int tall = 0;

  if (abs(x1 - x0) >= abs(y1 - y0)) { // octant 1/8
    wide = 1;
    loop_start = x;
    loop_end = x1;
    dx_east = dx_northeast = 1;
    dy_east = 0;
    d_east = A;
    distance = x1 - x + 1;
    if (A > 0) {
      d = A + B / 2;
      dy_northeast = 1;
      d_northeast = A + B;
    } else {
      d = A - B / 2;
      dy_northeast = -1;
      d_northeast = A - B;
    }
  } else {
    tall = 1;
    dx_east = 0;
    dx_northeast = 1;
    distance = abs(y1 - y) + 1;
    if (A > 0) {
      d = A / 2 + B;
      dy_east = dy_northeast = 1;
      d_northeast = A + B;
      d_east = B;
      loop_start = y;
      loop_end = y1;
    } else {
      d = A / 2 - B;
      dy_east = dy_northeast = -1;
      d_northeast = A - B;
      d_east = -1 * B;
      loop_start = y1;
      loop_end = y;
    }
  }

  z = z0;
  dz = (z1 - z0) / distance;

  while (loop_start < loop_end) {
    plot(s, zb, c, x, y, z);

    if ((wide && ((A > 0 && d > 0) || (A < 0 && d < 0))) ||
        (tall && ((A > 0 && d < 0) || (A < 0 && d > 0)))) {
      y += dy_northeast;
      d += d_northeast;
      x += dx_northeast;
    } else {
      x += dx_east;
      y += dy_east;
      d += d_east;
    }
    z += dz;
    loop_start++;
  }

  plot(s, zb, c, x1, y1, z);
}
