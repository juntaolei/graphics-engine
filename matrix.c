#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "matrix.h"

struct matrix*
make_bezier()
{
  /*
  Return a Bezier matrix.

  @param: No parameters

  @return: struct matrix*
  */
  struct matrix* bm = new_matrix(4, 4);

  bm->lastcol = 4;
  bm->m[0][0] = -1;
  bm->m[0][1] = 3;
  bm->m[0][2] = -3;
  bm->m[0][3] = 1;

  bm->m[1][0] = 3;
  bm->m[1][1] = -6;
  bm->m[1][2] = 3;
  bm->m[1][3] = 0;

  bm->m[2][0] = -3;
  bm->m[2][1] = 3;
  bm->m[2][2] = 0;
  bm->m[2][3] = 0;

  bm->m[3][0] = 1;
  bm->m[3][1] = 0;
  bm->m[3][2] = 0;
  bm->m[3][3] = 0;

  return bm;
}

struct matrix*
make_hermite()
{
  /*
  Return a Hermite matrix.

  @param: No parameters

  @return: struct matrix*
  */
  struct matrix* bm = new_matrix(4, 4);

  bm->lastcol = 4;
  bm->m[0][0] = 2;
  bm->m[0][1] = -2;
  bm->m[0][2] = 1;
  bm->m[0][3] = 1;

  bm->m[1][0] = -3;
  bm->m[1][1] = 3;
  bm->m[1][2] = -2;
  bm->m[1][3] = -1;

  bm->m[2][0] = 0;
  bm->m[2][1] = 0;
  bm->m[2][2] = 1;
  bm->m[2][3] = 0;

  bm->m[3][0] = 1;
  bm->m[3][1] = 0;
  bm->m[3][2] = 0;
  bm->m[3][3] = 0;

  return bm;
}

struct matrix*
generate_curve_coefs(double p0, double p1, double p2, double p3, int type)
{
  /*
  Return a matrix containing the values for a, b, c and d of the equation at^3 +
  bt^2 + ct + d for the curve defined by p1, p2, p3 and p4.

  @param: double p1
  @param: double p2
  @param: double p3
  @param: double p4
  @param: int type

  @return: struct matrix*
  */
  struct matrix* curve;
  struct matrix* coefs = new_matrix(4, 1);

  coefs->lastcol = 1;
  coefs->m[0][0] = p0;
  coefs->m[1][0] = p1;
  coefs->m[2][0] = p2;
  coefs->m[3][0] = p3;

  if (type == HERMITE)
    curve = make_hermite();
  else
    curve = make_bezier();

  matrix_mult(curve, coefs);
  free_matrix(curve);

  return coefs;
}

struct matrix*
make_translate(double x, double y, double z)
{
  /*
  Return the translation matrix created using x, y and z as the translation
  offsets.

  @param: int x
  @param: int y
  @param: int z

  @return: struct matrix*
  */
  struct matrix* t = new_matrix(4, 4);

  ident(t);

  t->m[0][3] = x;
  t->m[1][3] = y;
  t->m[2][3] = z;

  return t;
}

struct matrix*
make_scale(double x, double y, double z)
{
  /*
  Return the translation matrix creates using x, y and z as the scale factors

  @param: int x
  @param: int y
  @param: int z

  @return: struct matrix*
  */
  struct matrix* t = new_matrix(4, 4);

  ident(t);

  t->m[0][0] = x;
  t->m[1][1] = y;
  t->m[2][2] = z;

  return t;
}

struct matrix*
make_rotX(double theta)
{
  /*
  Return the rotation matrix created using theta as the angle of rotation and X
  as the axis of rotation.

  @param: double theta

  @return: struct matrix*
  */
  struct matrix* t = new_matrix(4, 4);

  ident(t);

  t->m[1][1] = cos(theta);
  t->m[1][2] = -1 * sin(theta);
  t->m[2][1] = sin(theta);
  t->m[2][2] = cos(theta);

  return t;
}

struct matrix*
make_rotY(double theta)
{
  /*
  Return the rotation matrix created using theta as the angle of rotation and Y
  as the axis of rotation.

  @param: double theta

  @return: struct matrix*
  */
  struct matrix* t = new_matrix(4, 4);

  ident(t);

  t->m[0][0] = cos(theta);
  t->m[2][0] = -1 * sin(theta);
  t->m[0][2] = sin(theta);
  t->m[2][2] = cos(theta);

  return t;
}

struct matrix*
make_rotZ(double theta)
{
  /*
  Return the rotation matrix created using theta as the angle of rotation and Z
  as the axis of rotation.

  @param: double theta

  @return: struct matrix*
  */
  struct matrix* t = new_matrix(4, 4);

  ident(t);

  t->m[0][0] = cos(theta);
  t->m[0][1] = -1 * sin(theta);
  t->m[1][0] = sin(theta);
  t->m[1][1] = cos(theta);

  return t;
}

void
print_matrix(struct matrix* m)
{
  /*
  Print the matrix.

  @param: struct matrix *m

  @return: void
  */
  int r, c;

  for (r = 0; r < m->rows; r++) {
    for (c = 0; c < m->lastcol; c++)
      printf("%0.2f ", m->m[r][c]);
    printf("\n");
  }
}

void
ident(struct matrix* m)
{
  /*
  Turns m in to an identity matrix.

  @param: struct matrix* m

  @return: void
  */
  int r, c;

  for (r = 0; r < m->rows; r++)
    for (c = 0; c < m->cols; c++)
      if (r == c)
        m->m[r][c] = 1;
      else
        m->m[r][c] = 0;
  m->lastcol = m->cols;
}

void
matrix_mult(struct matrix* a, struct matrix* b)
{
  /*
  Multiply a by b, modifying b to be the product.

  @param: struct matrix *a
  @param: struct matrix *b

  @return: void
  */
  int r, c;
  struct matrix* tmp;

  tmp = new_matrix(4, 1);

  for (c = 0; c < b->lastcol; c++) {
    for (r = 0; r < b->rows; r++)
      tmp->m[r][0] = b->m[r][c];

    for (r = 0; r < b->rows; r++)
      b->m[r][c] = a->m[r][0] * tmp->m[0][0] + a->m[r][1] * tmp->m[1][0] +
                   a->m[r][2] * tmp->m[2][0] + a->m[r][3] * tmp->m[3][0];
  }

  free_matrix(tmp);
}

struct matrix*
new_matrix(int rows, int cols)
{
  /*
  Return an allocated matrix.

  @param: int rows
  @param: int cols

  @return: struct matrix*
  */
  double** tmp;
  int i;
  struct matrix* m;

  tmp = (double**)malloc(rows * sizeof(double*));
  for (i = 0; i < rows; i++) {
    tmp[i] = (double*)malloc(cols * sizeof(double));
  }

  m = (struct matrix*)malloc(sizeof(struct matrix));
  m->m = tmp;
  m->rows = rows;
  m->cols = cols;
  m->lastcol = 0;

  return m;
}

void
free_matrix(struct matrix* m)
{
  /*
  Free a matrix struct.

  @param: struct matrix *m

  @return: void
  */
  int i;
  for (i = 0; i < m->rows; i++) {
    free(m->m[i]);
  }
  free(m->m);
  free(m);
}

void
grow_matrix(struct matrix* m, int newcols)
{
  /*
  Reallocates the memory for m->m such that it now has newcols number of
  collumns.

  @param: struct matrix *m
  @param: int newcols

  @return: void
  */
  int i;

  for (i = 0; i < m->rows; i++) {
    m->m[i] = realloc(m->m[i], newcols * sizeof(double));
  }

  m->cols = newcols;
}

void
copy_matrix(struct matrix* a, struct matrix* b)
{
  /*
  Copy matrix a to matrix b.

  @param: struct matrix *a
  @param: struct matrix *b

  @return: void
  */
  int r, c;

  for (r = 0; r < a->rows; r++)
    for (c = 0; c < a->cols; c++)
      b->m[r][c] = a->m[r][c];
}
