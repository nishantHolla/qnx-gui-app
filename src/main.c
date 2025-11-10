#include "rmp_log.h"
#include "rmp_time.h"

#include <stdbool.h>

#define TARGET_FPS 2
#define FRAME_TIME_US (1000000 / TARGET_FPS)

int main(void) {

  while (true) {
    time_t frame_start = rmp_time_get_us();
    rmp_log_info("main", "Running at frame rate %d\n", TARGET_FPS);
    time_t frame_duration = rmp_time_get_us() - frame_start;
    if (frame_duration < FRAME_TIME_US) {
      usleep(FRAME_TIME_US - frame_duration);
    }
  }

  return 0;
}
