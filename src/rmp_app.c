#include "rmp_app.h"
#include "rmp_time.h"
#include "rmp_vec2.h"
#include "rmp_log.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

#define RMP_APP_TARGET_FPS 30
#define RMP_APP_FRAME_TIME_US (1000000 / RMP_APP_TARGET_FPS)

const rmp_vec2_t RMP_SCREEN_START_VEC = { .x = 50,    .y = 30  };
const rmp_vec2_t RMP_SCREEN_END_VEC   = { .x = 1870, .y = 1050 };
const int RMP_SCREEN_WIDTH = RMP_SCREEN_END_VEC.x - RMP_SCREEN_START_VEC.x;
const int RMP_SCREEN_HEIGHT = RMP_SCREEN_END_VEC.y - RMP_SCREEN_START_VEC.y;

static void step(rmp_app_t* app);
static void reset_ball_pos(rmp_app_t* app, int ball_size);

rmp_appRet_e rmp_app_init(rmp_app_t* app) {
  if (!app) {
    return RMP_APP_BAD_ARGS;
  }

  app->running = true;
  pthread_mutex_init(&app->mutex, NULL);
  pthread_cond_init(&app->cond, NULL);

  int pad_size_x = 20;
  int pad_size_y = 150;
  int pad_padding = 50;
  int pad_pos_y = (RMP_SCREEN_HEIGHT / 2) - (pad_size_y / 2);

  rmp_vec2_set(&app->pad_a.size, pad_size_x, pad_size_y);
  rmp_vec2_set(&app->pad_a.pos, RMP_SCREEN_START_VEC.x + pad_padding, pad_pos_y);
  rmp_vec2_set(&app->pad_a.vel, 0, 0);

  rmp_vec2_set(&app->pad_b.size, pad_size_x, pad_size_y);
  rmp_vec2_set(&app->pad_b.pos, RMP_SCREEN_END_VEC.x - pad_padding - pad_size_x, pad_pos_y);
  rmp_vec2_set(&app->pad_b.vel, 0, 0);

  int ball_size = 20;
  rmp_vec2_set(&app->ball.size, ball_size, ball_size);
  reset_ball_pos(app, ball_size);
  rmp_vec2_set(&app->ball.vel, -5, 1);

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

    step(app);

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

static void step(rmp_app_t* app) {
  rmp_vec2_add(&app->ball.pos, app->ball.pos, app->ball.vel);

  if (app->ball.pos.y < RMP_SCREEN_START_VEC.y) {
    app->ball.pos.y = RMP_SCREEN_START_VEC.y;
    app->ball.vel.y *= -1;
  }

  if (app->ball.pos.y + app->ball.size.y >= RMP_SCREEN_END_VEC.y) {
    app->ball.pos.y = RMP_SCREEN_END_VEC.y - app->ball.size.y;
    app->ball.vel.y *= -1;
  }

  if (app->ball.pos.x < app->pad_a.pos.x + app->pad_a.size.x &&
      app->pad_a.pos.y <= app->ball.pos.y &&
      app->pad_a.pos.y + app->pad_a.size.y >= app->ball.pos.y + app->ball.size.y) {
    app->ball.pos.x = app->pad_a.pos.x + app->pad_a.size.x;
    app->ball.vel.x *= -1;
  }

  if (app->ball.pos.x + app->ball.size.x >= app->pad_b.pos.x &&
      app->pad_b.pos.y <= app->ball.pos.y &&
      app->pad_b.pos.y + app->pad_b.size.y >= app->ball.pos.y + app->ball.size.y) {
    app->ball.pos.x = app->pad_b.pos.x - app->ball.size.x;
    app->ball.vel.x *= -1;
  }

  if (app->ball.pos.x < RMP_SCREEN_START_VEC.x ||
    app->ball.pos.x + app->ball.size.x >= RMP_SCREEN_END_VEC.x) {
    reset_ball_pos(app, app->ball.size.x);
  }
}

static void reset_ball_pos(rmp_app_t* app, int ball_size) {
  if (!app) {
    return;
  }

  int ball_pos_x = (RMP_SCREEN_WIDTH / 2) - (ball_size / 2);
  int ball_pos_y = (RMP_SCREEN_HEIGHT / 2) - (ball_size / 2);

  rmp_vec2_set(&app->ball.pos, ball_pos_x, ball_pos_y);
}
