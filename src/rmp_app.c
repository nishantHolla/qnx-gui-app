#include "rmp_app.h"
#include "rmp_time.h"
#include "rmp_log.h"

#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

#define RMP_APP_TARGET_FPS 5
#define RMP_APP_FRAME_TIME_US (1000000 / RMP_APP_TARGET_FPS)

rmp_appRet_e rmp_app_init(rmp_app_t* app) {
  if (!app) {
    return RMP_APP_BAD_ARGS;
  }

  app->running = true;
  pthread_mutex_init(&app->mutex, NULL);
  pthread_cond_init(&app->cond, NULL);

  return RMP_APP_OK;
}

rmp_appRet_e rmp_app_free(rmp_app_t* app) {
  if (!app) {
    return RMP_APP_BAD_ARGS;
  }

  pthread_mutex_destroy(&app->mutex);
  pthread_cond_destroy(&app->cond);

  return RMP_APP_OK;
}

void* rmp_app_run(void* args) {
  if (!args) {
    return NULL;
  }

  rmp_app_t* app = (rmp_app_t*)args;

  while (true) {
    pthread_mutex_lock(&app->mutex);
    if (!app->running) {
      break;
    }
    pthread_mutex_unlock(&app->mutex);

    time_t frame_start = rmp_time_get_us();

    /// App Logic goes here
    rmp_log_info("app", "Running at frame rate %d\n", RMP_APP_TARGET_FPS);

    time_t frame_duration = rmp_time_get_us() - frame_start;
    if (frame_duration < RMP_APP_FRAME_TIME_US) {
      usleep(RMP_APP_FRAME_TIME_US - frame_duration);
    }
  }
  pthread_mutex_unlock(&app->mutex);

  return NULL;
}
