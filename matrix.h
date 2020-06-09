#ifndef MATRIX_H
#define MATRIX_H

#define HERMITE 0
#define BEZIER 1

struct matrix
{
  double** m;
  int rows, cols;
  int lastcol;
} matrix;

struct matrix*
make_bezier();

struct matrix*
make_hermite();

struct matrix*
generate_curve_coefs(double, double, double, double, int);

struct matrix*
make_translate(double, double, double);

struct matrix*
make_scale(double, double, double);

struct matrix*
make_rotX(double);

struct matrix*
make_rotY(double);

struct matrix*
make_rotZ(double);

struct matrix*
new_matrix(int, int);

void
free_matrix(struct matrix*);

void
grow_matrix(struct matrix*, int);

void
copy_matrix(struct matrix*, struct matrix*);

void
print_matrix(struct matrix*);

void
ident(struct matrix*);

void
matrix_mult(struct matrix*, struct matrix*);

#endif
