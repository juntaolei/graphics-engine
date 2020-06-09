/*
Serve as the interpreter for mdl.
When an mdl script goes through a lexer and parser,
the resulting operations will be in the array op[].
*/

#include "parser.h"
#include "symtab.h"
#include "y.tab.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "display.h"
#include "draw.h"
#include "gmath.h"
#include "matrix.h"
#include "mesh.h"
#include "ml6.h"
#include "stack.h"

void
first_pass()
{
  /*
  Checks the op array for any animation commands (frames, basename, vary).
  Should set num_frames and basename if the frames or basename commands are
  present. If vary is found, but frames is not, the entire program should exit.
  If frames is found, but basename is not, set name to some default value, and
  print out a message with the name being used.

  @param: No parameters

  @return: void
  */
  int i;
  char vary_check = 0;
  char name_check = 0;
  char frame_check = 0;
  extern int num_frames;
  extern char name[128];

  num_frames = 1;

  for (i = 0; i < lastop; i++) {
    if (op[i].opcode == FRAMES) {
      num_frames = op[i].op.frames.num_frames;
      frame_check = 1;
    } else if (op[i].opcode == BASENAME) {
      strncpy(name, op[i].op.basename.p->name, sizeof(name));
      name_check = 1;
    } else if (op[i].opcode == VARY) {
      vary_check = 1;
    }

    if (vary_check && !frame_check) {
      printf("Error: Vary command found but number of frames not set\b");
      exit(0);
    }
    if (frame_check && vary_check && !name_check) {
      printf("Warning: Animation code present but basename not set. Using "
             "\"frame\" as basename\n");
      strncpy(name, "frame", sizeof(name));
    }
  }
}

struct vary_node**
second_pass()
{
  /*
  Each index should contain a linked list of vary_nodes, each node contains a
  knob name, a value, and a pointer to the next node.

  @param: No paramters

  @return: struct vary_node** knobs
  */
  int i, k;
  int start_frame, end_frame;
  double start_value, end_value, delta;
  struct vary_node* curr = NULL;
  struct vary_node** knobs;
  knobs = (struct vary_node**)calloc(num_frames, sizeof(struct vary_node*));

  for (i = 0; i < lastop; i++) {
    if (op[i].opcode == VARY) {
      start_frame = op[i].op.vary.start_frame;
      end_frame = op[i].op.vary.end_frame;
      start_value = op[i].op.vary.start_val;
      end_value = op[i].op.vary.end_val;
      delta = (end_value - start_value) / (end_frame - start_frame);

      if (end_frame < start_frame) {
        printf("Error: end frame is before start frame for knob: %s\n",
               op[i].op.vary.p->name);
        exit(-1);
      }
      for (k = 0; k < num_frames; k++) {
        char found = 0;
        curr = knobs[k];

        while (curr) {
          if (strncmp(curr->name, op[i].op.vary.p->name, sizeof(curr->name)) ==
              0) {
            found = 1;
            break;
          }
          curr = curr->next;
        }

        if (!found) {
          curr = (struct vary_node*)calloc(1, sizeof(struct vary_node));
          strncpy(curr->name, op[i].op.vary.p->name, sizeof(curr->name));
          curr->value = 0;
          curr->next = knobs[k];
          knobs[k] = curr;
        }

        if (k == start_frame)
          curr->value = start_value;

        else if (k > start_frame && k <= end_frame)
          curr->value = start_value + (k - start_frame) * delta;

        printf("knob: %s\t%lf\n", curr->name, curr->value);
      }
    }
  }
  return knobs;
}

void
script()
{
  /*
  Run a given MDL script.

  @param: No paramters

  @return: void
  */
  struct vary_node** knobs;
  struct vary_node* vn;
  first_pass();
  knobs = second_pass();
  char frame_name[200];
  int f;

  int i;
  int lights;
  struct matrix* tmp;
  struct stack* systems;
  screen t;
  zbuffer zb;
  double step_3d = 100;
  double theta, xval, yval, zval, knob_value;

  color ambient;
  ambient.red = 50;
  ambient.green = 50;
  ambient.blue = 50;

  double light[MAX_LIGHTS][2][3];

  double view[3];
  view[0] = 0;
  view[1] = 0;
  view[2] = 1;

  struct constants white;
  white.r[AMBIENT_R] = 0.1;
  white.g[AMBIENT_R] = 0.1;
  white.b[AMBIENT_R] = 0.1;

  white.r[DIFFUSE_R] = 0.5;
  white.g[DIFFUSE_R] = 0.5;
  white.b[DIFFUSE_R] = 0.5;

  white.r[SPECULAR_R] = 0.5;
  white.g[SPECULAR_R] = 0.5;
  white.b[SPECULAR_R] = 0.5;

  struct constants* reflect;
  reflect = &white;

  color g;
  g.red = 255;
  g.green = 255;
  g.blue = 255;

  SYMTAB* sym;

  for (f = 0; f < num_frames; f++) {
    systems = new_stack();
    tmp = new_matrix(4, 1000);
    clear_screen(t);
    clear_zbuffer(zb);

    lights = 0;

    sprintf(frame_name, "anim/%s_%03d.png", name, f);

    vn = knobs[f];

    while (vn) {
      printf("\tknob: %s value:%lf\n", vn->name, vn->value);
      set_value(lookup_symbol(vn->name), vn->value);
      vn = vn->next;
    }

    printf("\nFrame: %d of %d\n", f + 1, num_frames);

    for (i = 0; i < lastop; i++) {
      printf("%d: ", i);

      switch (op[i].opcode) {
        case LIGHT:
          printf("Light: %s at: %6.2f %6.2f %6.2f",
                 op[i].op.light.p->name,
                 op[i].op.light.c[0],
                 op[i].op.light.c[1],
                 op[i].op.light.c[2]);
          sym = lookup_symbol(op[i].op.light.p->name);
          if (lights < MAX_LIGHTS) {
            light[lights][LOCATION][0] = sym->s.l->l[0];
            light[lights][LOCATION][1] = sym->s.l->l[1];
            light[lights][LOCATION][2] = sym->s.l->l[2];

            light[lights][COLOR][RED] = sym->s.l->c[0];
            light[lights][COLOR][GREEN] = sym->s.l->c[1];
            light[lights][COLOR][BLUE] = sym->s.l->c[2];

            if (op[i].op.light.b) {
              sym = lookup_symbol(op[i].op.light.b->name);

              light[lights][COLOR][RED] *= sym->s.value;
              light[lights][COLOR][GREEN] *= sym->s.value;
              light[lights][COLOR][BLUE] *= sym->s.value;
            }
            lights += 1;
          }
          break;
        case AMBIENT:
          printf("Ambient: %6.2f %6.2f %6.2f",
                 op[i].op.ambient.c[0],
                 op[i].op.ambient.c[1],
                 op[i].op.ambient.c[2]);
          break;
        case CONSTANTS:
          printf("Constants: %s", op[i].op.constants.p->name);
          break;
        case SAVE_COORDS:
          printf("Save Coords: %s", op[i].op.save_coordinate_system.p->name);
          break;
        case CAMERA:
          printf("Camera: eye: %6.2f %6.2f %6.2f\taim: %6.2f %6.2f %6.2f",
                 op[i].op.camera.eye[0],
                 op[i].op.camera.eye[1],
                 op[i].op.camera.eye[2],
                 op[i].op.camera.aim[0],
                 op[i].op.camera.aim[1],
                 op[i].op.camera.aim[2]);
          break;
        case SPHERE:
          printf("Sphere: %6.2f %6.2f %6.2f r=%6.2f",
                 op[i].op.sphere.d[0],
                 op[i].op.sphere.d[1],
                 op[i].op.sphere.d[2],
                 op[i].op.sphere.r);
          if (op[i].op.sphere.constants != NULL) {
            printf("\tconstants: %s", op[i].op.sphere.constants->name);
            reflect = lookup_symbol(op[i].op.sphere.constants->name)->s.c;
          }
          if (op[i].op.sphere.cs != NULL) {
            printf("\tcs: %s", op[i].op.sphere.cs->name);
          }
          add_sphere(tmp,
                     op[i].op.sphere.d[0],
                     op[i].op.sphere.d[1],
                     op[i].op.sphere.d[2],
                     op[i].op.sphere.r,
                     step_3d);
          matrix_mult(peek(systems), tmp);
          draw_polygons(tmp, t, zb, view, lights, light, ambient, reflect);
          tmp->lastcol = 0;
          reflect = &white;
          break;
        case TORUS:
          printf("Torus: %6.2f %6.2f %6.2f r0=%6.2f r1=%6.2f",
                 op[i].op.torus.d[0],
                 op[i].op.torus.d[1],
                 op[i].op.torus.d[2],
                 op[i].op.torus.r0,
                 op[i].op.torus.r1);
          if (op[i].op.torus.constants != NULL) {
            printf("\tconstants: %s", op[i].op.torus.constants->name);
            reflect = lookup_symbol(op[i].op.sphere.constants->name)->s.c;
          }
          if (op[i].op.torus.cs != NULL) {
            printf("\tcs: %s", op[i].op.torus.cs->name);
          }
          add_torus(tmp,
                    op[i].op.torus.d[0],
                    op[i].op.torus.d[1],
                    op[i].op.torus.d[2],
                    op[i].op.torus.r0,
                    op[i].op.torus.r1,
                    step_3d);
          matrix_mult(peek(systems), tmp);
          draw_polygons(tmp, t, zb, view, lights, light, ambient, reflect);
          tmp->lastcol = 0;
          reflect = &white;
          break;
        case BOX:
          printf("Box: d0: %6.2f %6.2f %6.2f d1: %6.2f %6.2f %6.2f",
                 op[i].op.box.d0[0],
                 op[i].op.box.d0[1],
                 op[i].op.box.d0[2],
                 op[i].op.box.d1[0],
                 op[i].op.box.d1[1],
                 op[i].op.box.d1[2]);
          if (op[i].op.box.constants != NULL) {
            printf("\tconstants: %s", op[i].op.box.constants->name);
            reflect = lookup_symbol(op[i].op.sphere.constants->name)->s.c;
          }
          if (op[i].op.box.cs != NULL) {
            printf("\tcs: %s", op[i].op.box.cs->name);
          }
          add_box(tmp,
                  op[i].op.box.d0[0],
                  op[i].op.box.d0[1],
                  op[i].op.box.d0[2],
                  op[i].op.box.d1[0],
                  op[i].op.box.d1[1],
                  op[i].op.box.d1[2]);
          matrix_mult(peek(systems), tmp);
          draw_polygons(tmp, t, zb, view, lights, light, ambient, reflect);
          tmp->lastcol = 0;
          reflect = &white;
          break;
        case LINE:
          printf("Line: from: %6.2f %6.2f %6.2f to: %6.2f %6.2f %6.2f",
                 op[i].op.line.p0[0],
                 op[i].op.line.p0[1],
                 op[i].op.line.p0[2],
                 op[i].op.line.p1[0],
                 op[i].op.line.p1[1],
                 op[i].op.line.p1[2]);
          if (op[i].op.line.constants != NULL) {
            printf("\n\tConstants: %s", op[i].op.line.constants->name);
          }
          if (op[i].op.line.cs0 != NULL) {
            printf("\n\tCS0: %s", op[i].op.line.cs0->name);
          }
          if (op[i].op.line.cs1 != NULL) {
            printf("\n\tCS1: %s", op[i].op.line.cs1->name);
          }
          add_edge(tmp,
                   op[i].op.line.p0[0],
                   op[i].op.line.p0[1],
                   op[i].op.line.p0[2],
                   op[i].op.line.p1[0],
                   op[i].op.line.p1[1],
                   op[i].op.line.p1[2]);
          matrix_mult(peek(systems), tmp);
          draw_lines(tmp, t, zb, g);
          tmp->lastcol = 0;
          break;
        case MESH:
          printf("Mesh: filename: %s", op[i].op.mesh.name);
          if (op[i].op.mesh.constants != NULL) {
            reflect = lookup_symbol(op[i].op.mesh.constants->name)->s.c;
          }
          obj_parser(tmp, op[i].op.mesh.name);
          matrix_mult(peek(systems), tmp);
          draw_polygons(tmp, t, zb, view, lights, light, ambient, reflect);
          tmp->lastcol = 0;
          reflect = &white;
          break;
        case SET:
          printf(
            "Set: %s %6.2f", op[i].op.set.p->name, op[i].op.set.p->s.value);
          break;
        case MOVE:
          xval = op[i].op.move.d[0];
          yval = op[i].op.move.d[1];
          zval = op[i].op.move.d[2];
          printf("Move: %6.2f %6.2f %6.2f", xval, yval, zval);
          if (op[i].op.move.p != NULL) {
            printf("\tknob: %s", op[i].op.move.p->name);
            knob_value = lookup_symbol(op[i].op.move.p->name)->s.value;
            xval *= knob_value;
            yval *= knob_value;
            zval *= knob_value;
          }
          tmp = make_translate(xval, yval, zval);
          matrix_mult(peek(systems), tmp);
          copy_matrix(tmp, peek(systems));
          tmp->lastcol = 0;
          break;
        case SCALE:
          xval = op[i].op.scale.d[0];
          yval = op[i].op.scale.d[1];
          zval = op[i].op.scale.d[2];
          printf("Scale: %6.2f %6.2f %6.2f", xval, yval, zval);
          if (op[i].op.scale.p != NULL) {
            printf("\tknob: %s", op[i].op.scale.p->name);
            knob_value = lookup_symbol(op[i].op.scale.p->name)->s.value;
            xval *= knob_value;
            yval *= knob_value;
            zval *= knob_value;
          }
          tmp = make_scale(xval, yval, zval);
          matrix_mult(peek(systems), tmp);
          copy_matrix(tmp, peek(systems));
          tmp->lastcol = 0;
          break;
        case ROTATE:
          printf("Rotate: axis: %6.2f degrees: %6.2f",
                 op[i].op.rotate.axis,
                 op[i].op.rotate.degrees);
          theta = op[i].op.rotate.degrees * (M_PI / 180);
          if (op[i].op.rotate.p != NULL) {
            printf("\tknob: %s", op[i].op.rotate.p->name);
            knob_value = lookup_symbol(op[i].op.rotate.p->name)->s.value;
            theta *= knob_value;
          }

          if (op[i].op.rotate.axis == 0)
            tmp = make_rotX(theta);
          else if (op[i].op.rotate.axis == 1)
            tmp = make_rotY(theta);
          else
            tmp = make_rotZ(theta);

          matrix_mult(peek(systems), tmp);
          copy_matrix(tmp, peek(systems));
          tmp->lastcol = 0;
          break;
        case BASENAME:
          printf("Basename: %s", name);
          break;
        case SAVE_KNOBS:
          printf("Save knobs: %s", op[i].op.save_knobs.p->name);
          break;
        case TWEEN:
          printf("Tween: %4.0f %4.0f, %s %s",
                 op[i].op.tween.start_frame,
                 op[i].op.tween.end_frame,
                 op[i].op.tween.knob_list0->name,
                 op[i].op.tween.knob_list1->name);
          break;
        case FRAMES:
          printf("Num frames: %4.0f", op[i].op.frames.num_frames);
          break;
        case VARY:
          printf("Vary: %4.0f %4.0f, %4.0f %4.0f",
                 op[i].op.vary.start_frame,
                 op[i].op.vary.end_frame,
                 op[i].op.vary.start_val,
                 op[i].op.vary.end_val);
          break;
        case PUSH:
          printf("Push");
          push(systems);
          break;
        case POP:
          printf("Pop");
          pop(systems);
          break;
        case GENERATE_RAYFILES:
          printf("Generate Ray Files");
          break;
        case SAVE:
          printf("Save: %s", op[i].op.save.p->name);
          save_extension(t, op[i].op.save.p->name);
          break;
        case SHADING:
          printf("Shading: %s", op[i].op.shading.p->name);
          break;
        case SETKNOBS:
          printf("Setknobs: %f", op[i].op.setknobs.value);
          break;
        case FOCAL:
          printf("Focal: %f", op[i].op.focal.value);
          break;
        case DISPLAY:
          printf("Display");
          display(t);
          break;
      }
      save_extension(t, frame_name);
      printf("\n");
    }

    free_stack(systems);
    free_matrix(tmp);
  }
  if (num_frames > 1)
    make_animation(name);
}
