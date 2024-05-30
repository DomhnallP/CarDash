#include "precalc.h"

float xp = 0.0f, yp = 0.0f;

void precalculate_indices(float indices_inner_x[], float  indices_inner_y[], float  indices_outer_x[], float  indices_outer_y[], float needle_end_x[], float needle_end_y[], float CLOCK_R, float NEEDLE_LENGTH){
  for (int i = 0; i < 360; i++) {
    calculate_needle_coords(CLOCK_R, CLOCK_R, &xp, &yp, NEEDLE_LENGTH, i);
    needle_end_x[i] = xp;
    needle_end_y[i] = yp;

    if (i % 30 == 0) {
      calculate_index_coords(indices_inner_x, indices_inner_y, indices_outer_x, indices_outer_y, 210, 30, i, i, CLOCK_R);
    } else if (i % 15 == 0) {
      calculate_index_coords(indices_inner_x, indices_inner_y, indices_outer_x, indices_outer_y, 220, 20, i, i, CLOCK_R);
    } else if (i % 3 == 0) {
      calculate_index_coords(indices_inner_x, indices_inner_y, indices_outer_x, indices_outer_y, 225, 10, i, i, CLOCK_R);
    } else {
      indices_inner_x[i] = 0.0;  //outer points of Speed gauges
      indices_inner_y[i] = 0.0;
      indices_outer_x[i] = 0.0;  //outer points of Speed gauges
      indices_outer_y[i] = 0.0;
    }
  }
}

#define DEG2RAD 0.0174532925
void calculate_needle_coords(int16_t x, int16_t y, float *xp, float *yp, int16_t r, float a) {
  float sx1 = cos((a - 90) * DEG2RAD);
  float sy1 = sin((a - 90) * DEG2RAD);
  *xp = sx1 * r + x;
  *yp = sy1 * r + y;
}

void calculate_index_coords(float inner_x[], float inner_y[], float outer_x[], float outer_y[], int16_t r, int16_t len, float a, int i, float CLOCK_R) {
  inner_x[i] = r * cos(DEG2RAD * (a - 90)) + CLOCK_R;
  inner_y[i] = r * sin(DEG2RAD * (a - 90)) + CLOCK_R;
  outer_x[i] = (r + len) * cos(DEG2RAD * (a - 90)) + CLOCK_R;  //outer points of Speed gauges
  outer_y[i] = (r + len) * sin(DEG2RAD * (a - 90)) + CLOCK_R;
}
