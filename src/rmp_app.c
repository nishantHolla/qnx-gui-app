#include "rmp_app.h"
#include "rmp_time.h"
#include "rmp_log.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

#define RMP_APP_TARGET_FPS 1
#define RMP_APP_FRAME_TIME_US (1000000 / RMP_APP_TARGET_FPS)

const rmp_vec2_t RMP_SCREEN_START_VEC = { .x = 0,    .y = 0 };
const rmp_vec2_t RMP_SCREEN_END_VEC   = { .x = 1920, .y = 1080 };

rmp_appRet_e rmp_app_init(rmp_app_t* app) {
  if (!app) {
    return RMP_APP_BAD_ARGS;
  }

  app->running = true;
  pthread_mutex_init(&app->mutex, NULL);
  pthread_cond_init(&app->cond, NULL);

  int pad_size_x = 20;
  int pad_size_y = 150;
  int pad_padding = 100;
  int pad_pos_y = (RMP_APP_SCREEN_HEIGHT / 2) - (pad_size_y / 2);

  rmp_vec2_set(&app->pad_a.size, pad_size_x, pad_size_y);
  rmp_vec2_set(&app->pad_a.pos, pad_padding, pad_pos_y);
  rmp_vec2_set(&app->pad_a.vel, 0, 0);

  rmp_vec2_set(&app->pad_b.size, pad_size_x, pad_size_y);
  rmp_vec2_set(&app->pad_b.pos, RMP_APP_SCREEN_WIDTH - pad_padding - pad_size_x, pad_pos_y);
  rmp_vec2_set(&app->pad_b.vel, 0, 0);

  int ball_size = 20;
  int ball_pos_x = (RMP_APP_SCREEN_WIDTH / 2) - (ball_size / 2);
  int ball_pos_y = (RMP_APP_SCREEN_HEIGHT / 2) - (ball_size / 2);

  rmp_vec2_set(&app->ball.size, ball_size, ball_size);
  rmp_vec2_set(&app->ball.pos, ball_pos_x, ball_pos_y);
  rmp_vec2_set(&app->ball.vel, 0, 0);

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
    // rmp_log_info("app", "Running at frame rate %d\n", RMP_APP_TARGET_FPS);

    time_t frame_duration = rmp_time_get_us() - frame_start;
    if (frame_duration < RMP_APP_FRAME_TIME_US) {
      usleep(RMP_APP_FRAME_TIME_US - frame_duration);
    }
  }
  pthread_mutex_unlock(&app->mutex);

  return NULL;
}

void rmp_app_log_entity(const char* name, rmp_app_entity_t entity) {
  rmp_log_info("app", "Entity %s\n", name ? name : "");
  printf("    pos : %.2lf %.2lf\n", entity.pos.x, entity.pos.y);
  printf("    vel : %.2lf %.2lf\n", entity.vel.x, entity.vel.y);
  printf("    size: %.2lf %.2lf\n", entity.size.x, entity.size.y);
}
