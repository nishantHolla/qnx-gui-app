#include "rmp_log.h"

int main(void) {
  rmp_log_error("main", "This is an error (%d)\n", 42);
  rmp_log_info("main", "This is an info (%d)\n", 42);
  rmp_log_warn("main", "This is a wanring (%d)\n", 42);
  return 0;
}
