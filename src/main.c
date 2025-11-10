#include "rmp_app.h"
#include "rmp_log.h"

#include <stdlib.h>

int main(void) {
  rmp_app_t app;
  rmp_app_init(&app);

  pthread_t app_tid;
  if (pthread_create(&app_tid, NULL, rmp_app_run, (void*)&app) != 0) {
    rmp_log_error("main", "Failed to create app thread\n");
    return EXIT_FAILURE;
  }

  pthread_mutex_lock(&app.mutex);
  while (app.running) {
    pthread_cond_wait(&app.cond, &app.mutex);
  }
  pthread_mutex_unlock(&app.mutex);

  pthread_join(app_tid, NULL);

  rmp_app_free(&app);
  return EXIT_SUCCESS;
}
