#ifndef DISPLAY_H
#define DISPLAY_H

#include "ml6.h"

void
plot(screen, zbuffer, color, int, int, double);

void clear_screen(screen);

void clear_zbuffer(zbuffer);

void
save_ppm(screen, char*);

void
save_extension(screen, char*);

void display(screen);

void
make_animation(char*);

#endif
