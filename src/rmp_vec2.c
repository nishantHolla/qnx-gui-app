#include "rmp_vec2.h"

#include <math.h>

#define MIN(a, b) (( (a) < (b) ) ? (a) : (b))
#define MAX(a, b) (( (a) > (b) ) ? (a) : (b))

void rmp_vec2_set(rmp_vec2_t* vec, double x, double y) {
  if (!vec) return;
  vec->x = x;
  vec->y = y;
}

void rmp_vec2_add(rmp_vec2_t* dst, rmp_vec2_t vec1, rmp_vec2_t vec2) {
  if (!dst) return;
  dst->x = vec1.x + vec2.x;
  dst->y = vec1.y + vec2.y;
}

void rmp_vec2_sub(rmp_vec2_t* dst, rmp_vec2_t vec1, rmp_vec2_t vec2) {
  if (!dst) return;
  dst->x = vec1.x - vec2.x;
  dst->y = vec1.y - vec2.y;
}

void rmp_vec2_scale(rmp_vec2_t* dst, rmp_vec2_t vec, double s) {
  if (!dst) return;
  dst->x = vec.x * s;
  dst->y = vec.y * s;
}

double rmp_vec2_dot(rmp_vec2_t a, rmp_vec2_t b) {
  return a.x * b.x + a.y * b.y;
}

double rmp_vec2_len(rmp_vec2_t vec) {
  return sqrt(vec.x * vec.x + vec.y * vec.y);
}

void rmp_vec2_normalize(rmp_vec2_t* dst, rmp_vec2_t vec) {
  if (!dst) return;
  double len = sqrt(vec.x * vec.x + vec.y * vec.y);
  if (len > 1e-12) {
    dst->x = vec.x / len;
    dst->y = vec.y / len;
  } else {
    dst->x = 0.0;
    dst->y = 0.0;
  }
}

void rmp_vec2_clamp(rmp_vec2_t* dst, rmp_vec2_t vec, rmp_vec2_t min, rmp_vec2_t max) {
  if (!dst) return;
  dst->x = MAX(min.x, MIN(max.x, vec.x));
  dst->y = MAX(min.y, MIN(max.y, vec.y));
}

