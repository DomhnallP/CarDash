#ifndef PRECALC_H
#define PRECALC_H

#include <Arduino.h>

void precalculate_indices(float indices_inner_x[], float  indices_inner_y[], float  indices_outer_x[], float  indices_outer_y[], float needle_end_x[], float needle_end_y[], float CLOCK_R, float NEEDLE_LENGTH);
void calculate_needle_coords(int16_t x, int16_t y, float *xp, float *yp, int16_t r, float a);
void calculate_index_coords(float inner_x[], float inner_y[], float outer_x[], float outer_y[], int16_t r, int16_t len, float a, int i, float CLOCK_R);


#endif