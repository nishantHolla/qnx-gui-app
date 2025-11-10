#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "rmp_vec2.h"

#define EPSILON 1e-9
#define ASSERT_EQ_DOUBLE(a, b) assert(fabs((a) - (b)) < EPSILON)
#define ASSERT_VEC_EQ(v, xval, yval) \
    do { \
        ASSERT_EQ_DOUBLE((v).x, (xval)); \
        ASSERT_EQ_DOUBLE((v).y, (yval)); \
    } while (0)

int main(void) {
    printf("Running rmp_vec2_t function API tests...\n");

    rmp_vec2_t a, b, c, min, max;
    rmp_vec2_set(&a, 3.0, 4.0);
    ASSERT_VEC_EQ(a, 3.0, 4.0);

    rmp_vec2_set(&b, 1.0, 2.0);
    rmp_vec2_add(&c, a, b);
    ASSERT_VEC_EQ(c, 4.0, 6.0);

    rmp_vec2_sub(&c, a, b);
    ASSERT_VEC_EQ(c, 2.0, 2.0);

    rmp_vec2_scale(&c, a, 2.0);
    ASSERT_VEC_EQ(c, 6.0, 8.0);

    double dot = rmp_vec2_dot(a, b);
    ASSERT_EQ_DOUBLE(dot, 3.0 * 1.0 + 4.0 * 2.0);  // 11.0

    double len = rmp_vec2_len(a);
    ASSERT_EQ_DOUBLE(len, 5.0);

    rmp_vec2_normalize(&c, a);
    ASSERT_EQ_DOUBLE(rmp_vec2_len(c), 1.0);

    rmp_vec2_set(&a, 10.0, -5.0);
    rmp_vec2_set(&min, 0.0, 0.0);
    rmp_vec2_set(&max, 5.0, 5.0);
    rmp_vec2_clamp(&c, a, min, max);
    ASSERT_VEC_EQ(c, 5.0, 0.0);

    printf("All rmp_vec2_t function API tests passed successfully\n");
    return 0;
}

