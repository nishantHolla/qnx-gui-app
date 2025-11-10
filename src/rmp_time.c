#include "rmp_time.h"

#include <stdlib.h>
#include <unistd.h>
#include <time.h>

time_t rmp_time_get_us(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (ts.tv_sec * 1000000) + (ts.tv_nsec / 1000);
}
