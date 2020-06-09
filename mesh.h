#ifndef MESH_H
#define MESH_H

#include "draw.h"
#include "matrix.h"

#define V 0
#define F 1
#define M_SIZE 128

char**
process_line(char*);

void
obj_parser(struct matrix*, char*);

void
add_mesh(struct matrix*, struct matrix*, struct matrix*);

void
add_mesh_point(struct matrix*, double[4], int);

#endif