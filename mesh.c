#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "draw.h"
#include "matrix.h"
#include "mesh.h"

char**
process_line(char* line)
{
  /*
  Process the given line.

  @param: char* args

  @return: char**
  */
  char* processed;
  char** tokens = calloc(sizeof(char*), 10);

  int i = 0;

  while (line) {
    tokens[i] = strsep(&line, " ");

    if (strcmp(tokens[i], "")) {
      processed = calloc(sizeof(char), 1);

      strcpy(processed, strsep(&tokens[i], "/"));

      tokens[i] = processed;

      i++;
    }
  }

  return tokens;
}

void
add_mesh_point(struct matrix* points, double vals[4], int type)
{
  /*
  Add points to matrix.

  @param: struct matrix* points
  @param: double vals[4]
  @param: int type

  @return: void
  */
  if (points->lastcol == points->cols) {
    grow_matrix(points, points->lastcol + M_SIZE);
  }

  points->m[0][points->lastcol] = vals[0];
  points->m[1][points->lastcol] = vals[1];
  points->m[2][points->lastcol] = vals[2];

  if (type == F) {
    points->m[3][points->lastcol] = vals[3];
  }

  points->lastcol += 1;
}

void
add_mesh(struct matrix* polygons, struct matrix* v, struct matrix* f)
{
  /*
  Add the vertices and faces to polygons.

  @param: struct matrix* polygons,
  @param: struct matrix* v
  @param: struct matrix* f

  @return: void
  */
  int i;
  int v0;
  int v1;
  int v2;
  int v3;

  for (i = 0; i < f->lastcol; i++) {
    v0 = ((int)f->m[0][i]) - 1;
    v1 = ((int)f->m[1][i]) - 1;
    v2 = ((int)f->m[2][i]) - 1;
    v3 = ((int)f->m[3][i]) - 1;

    add_polygon(polygons,
                v->m[0][v0],
                v->m[1][v0],
                v->m[2][v0],
                v->m[0][v1],
                v->m[1][v1],
                v->m[2][v1],
                v->m[0][v2],
                v->m[1][v2],
                v->m[2][v2]);

    if (v3 > 0) {
      add_polygon(polygons,
                  v->m[0][v0],
                  v->m[1][v0],
                  v->m[2][v0],
                  v->m[0][v2],
                  v->m[1][v2],
                  v->m[2][v2],
                  v->m[0][v3],
                  v->m[1][v3],
                  v->m[2][v3]);
    }
  }

  free_matrix(v);
  free_matrix(f);
}

void
obj_parser(struct matrix* polygons, char* file)
{
  /*
  Parse OBJ format and add its polygons.

  @param: struct matrix* polygons
  @param: char* file

  @return: void
  */
  struct matrix* v;
  struct matrix* f;

  v = new_matrix(3, M_SIZE);
  f = new_matrix(4, M_SIZE);

  FILE* fs;

  fs = fopen(file, "r");

  int i;

  char line[256];

  double vals[4];

  char type[3];

  char** args;

  while (fgets(line, sizeof(line), fs)) {
    line[strlen(line) - 1] = '\0';

    if (!strncmp(line, "f", 1)) {
      args = process_line(line);

      i = 0;

      while (args[i + 1] && i < 4) {
        vals[i] = atof(args[i + 1]);
        i++;
      }

      add_mesh_point(f, vals, F);
    } else if (!strncmp(line, "v", 1)) {
      sscanf(line, "%s %lf %lf %lf", type, vals, vals + 1, vals + 2);

      if (!strncmp(type, "v", strlen(type))) {
        add_mesh_point(v, vals, V);
      }
    }
  }

  free(args);

  add_mesh(polygons, v, f);
}
