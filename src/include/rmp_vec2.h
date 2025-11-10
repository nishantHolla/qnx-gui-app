#ifndef RMP_VEC2_H_
#define RMP_VEC2_H_

typedef struct {
  double x, y;
} rmp_vec2_t;

void rmp_vec2_set(rmp_vec2_t* vec, double x, double y);
void rmp_vec2_add(rmp_vec2_t* dst, rmp_vec2_t vec1, rmp_vec2_t vec2);
void rmp_vec2_sub(rmp_vec2_t* dst, rmp_vec2_t vec1, rmp_vec2_t vec2);
void rmp_vec2_scale(rmp_vec2_t* dst, rmp_vec2_t vec, double s);
double rmp_vec2_dot(rmp_vec2_t a, rmp_vec2_t b);
double rmp_vec2_len(rmp_vec2_t vec);
void rmp_vec2_normalize(rmp_vec2_t* dst, rmp_vec2_t vec);
void rmp_vec2_clamp(rmp_vec2_t* dst, rmp_vec2_t vec, rmp_vec2_t min, rmp_vec2_t max);

#endif // !RMP_VEC2_H_
