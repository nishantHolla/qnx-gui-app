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
#include <math.h>

#define RMP_APP_TARGET_FPS 30
#define RMP_APP_FRAME_TIME_US (1000000 / RMP_APP_TARGET_FPS)

static void step(rmp_app_t* app);
static void reset_ball_pos(rmp_app_t* app);
static void make_ai_move(rmp_app_t* app);

rmp_appRet_e rmp_app_init(rmp_app_t* app) {
  if (!app) {
    return RMP_APP_BAD_ARGS;
  }

  app->running = true;
  app->paused = true;
  app->recalibrating = false;
  app->ai_is_playing = true;

  pthread_mutex_init(&app->mutex, NULL);
  pthread_cond_init(&app->cond, NULL);

  rmp_vec2_set(&app->SCREEN_START, 50, 30);
  rmp_vec2_set(&app->SCREEN_END, 1870, 1050);

  rmp_vec2_set(&app->pad_size, 20, 150);
  app->pad_padding = 50;
  app->pad_speed = 20;

  int pad_pos_y = app->SCREEN_START.y + ((SCREEN_HEIGHT_P(app) / 2.0) - (app->pad_size.y / 2.0));
  rmp_vec2_set(&app->pad_a.size, app->pad_size.x, app->pad_size.y);
  rmp_vec2_set(&app->pad_a.pos, app->SCREEN_START.x + app->pad_padding, pad_pos_y);
  rmp_vec2_set(&app->pad_a.vel, 0, 0);

  rmp_vec2_set(&app->pad_b.size, app->pad_size.x, app->pad_size.y);
  rmp_vec2_set(&app->pad_b.pos, app->SCREEN_END.x - app->pad_padding - app->pad_size.x, pad_pos_y);
  rmp_vec2_set(&app->pad_b.vel, 0, 0);

  app->ball_size = 20;
  rmp_vec2_set(&app->ball.size, app->ball_size, app->ball_size);
  reset_ball_pos(app);
  rmp_vec2_set(&app->ball.vel, 12, 12);

  rmp_log_info("app", "Initialized app\n");
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

  rmp_log_info("app", "Started app run\n");
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

void rmp_app_recalibrate(rmp_app_t* app) {
  if (!app) {
    return;
  }

  int pad_pos_y = app->SCREEN_START.y + ((SCREEN_HEIGHT_P(app) / 2.0) - (app->pad_size.y / 2.0));
  rmp_vec2_set(&app->pad_a.size, app->pad_size.x, app->pad_size.y);
  rmp_vec2_set(&app->pad_a.pos, app->SCREEN_START.x + app->pad_padding, pad_pos_y);
  rmp_vec2_set(&app->pad_a.vel, 0, 0);

  rmp_vec2_set(&app->pad_b.size, app->pad_size.x, app->pad_size.y);
  rmp_vec2_set(&app->pad_b.pos, app->SCREEN_END.x - app->pad_padding - app->pad_size.x, pad_pos_y);
  rmp_vec2_set(&app->pad_b.vel, 0, 0);
}

static void step(rmp_app_t* app) {
  if (app->paused || app->recalibrating) {
    return;
  }

  if (app->ai_is_playing) {
    make_ai_move(app);
  }

  // Update paddle A
  rmp_vec2_add(&app->pad_a.pos, app->pad_a.pos, app->pad_a.vel);
  app->pad_a.pos.y = fmaxf(app->SCREEN_START.y,
                           fminf(app->pad_a.pos.y, app->SCREEN_END.y - app->pad_a.size.y));

  // Update paddle B
  rmp_vec2_add(&app->pad_b.pos, app->pad_b.pos, app->pad_b.vel);
  app->pad_b.pos.y = fmaxf(app->SCREEN_START.y,
                           fminf(app->pad_b.pos.y, app->SCREEN_END.y - app->pad_b.size.y));

  // Store previous ball position for proper collision detection
  float prev_ball_x = app->ball.pos.x;

  // Update ball position
  rmp_vec2_add(&app->ball.pos, app->ball.pos, app->ball.vel);

  // Top/bottom wall collision
  if (app->ball.pos.y <= app->SCREEN_START.y) {
    app->ball.pos.y = app->SCREEN_START.y;
    app->ball.vel.y = fabs(app->ball.vel.y); // Force downward
  }
  if (app->ball.pos.y + app->ball.size.y >= app->SCREEN_END.y) {
    app->ball.pos.y = app->SCREEN_END.y - app->ball.size.y;
    app->ball.vel.y = -fabs(app->ball.vel.y); // Force upward
  }

  // Paddle A collision (left paddle)
  if (app->ball.vel.x < 0 && // Ball moving left
    prev_ball_x >= app->pad_a.pos.x + app->pad_a.size.x && // Was right of paddle
    app->ball.pos.x <= app->pad_a.pos.x + app->pad_a.size.x && // Now overlapping
    app->ball.pos.y + app->ball.size.y > app->pad_a.pos.y && // Vertical overlap check
    app->ball.pos.y < app->pad_a.pos.y + app->pad_a.size.y) {
    app->ball.pos.x = app->pad_a.pos.x + app->pad_a.size.x;
    app->ball.vel.x = fabs(app->ball.vel.x); // Force rightward
  }

  // Paddle B collision (right paddle)
  if (app->ball.vel.x > 0 && // Ball moving right
    prev_ball_x + app->ball.size.x <= app->pad_b.pos.x && // Was left of paddle
    app->ball.pos.x + app->ball.size.x >= app->pad_b.pos.x && // Now overlapping
    app->ball.pos.y + app->ball.size.y > app->pad_b.pos.y && // Vertical overlap check
    app->ball.pos.y < app->pad_b.pos.y + app->pad_b.size.y) {
    app->ball.pos.x = app->pad_b.pos.x - app->ball.size.x;
    app->ball.vel.x = -fabs(app->ball.vel.x); // Force leftward
  }

  // Score/reset (left or right boundary)
  if (app->ball.pos.x < app->SCREEN_START.x ||
    app->ball.pos.x + app->ball.size.x >= app->SCREEN_END.x) {
    reset_ball_pos(app);
  }
}

static void reset_ball_pos(rmp_app_t* app) {
  if (!app) {
    return;
  }

  int ball_pos_x = app->SCREEN_START.x + (SCREEN_WIDTH_P(app) / 2.0) - (app->ball_size / 2.0);
  int ball_pos_y = app->SCREEN_START.y + (SCREEN_HEIGHT_P(app) / 2.0) - (app->ball_size / 2.0);

  rmp_vec2_set(&app->ball.pos, ball_pos_x, ball_pos_y);
}

static float predict_ball_intersection(rmp_app_t* app) {
  if (app->ball.vel.x <= 0) {
    return app->ball.pos.y + app->ball.size.y / 2.0f;
  }

  float dx = app->pad_b.pos.x - (app->ball.pos.x + app->ball.size.x);
  float time_to_reach = dx / app->ball.vel.x;

  float predicted_y = app->ball.pos.y + app->ball.vel.y * time_to_reach;
  float ball_center_y = predicted_y + app->ball.size.y / 2.0f;

  float field_height = SCREEN_HEIGHT_P(app);

  while (ball_center_y < 0 || ball_center_y > field_height) {
    if (ball_center_y < 0) {
      ball_center_y = -ball_center_y;
    }
    else if (ball_center_y > field_height) {
      ball_center_y = 2 * field_height - ball_center_y;
    }
  }

  return ball_center_y;
}

static void make_ai_move(rmp_app_t* app) {
  if (!app) {
    return;
  }

  float predicted_y = predict_ball_intersection(app);

  float error_margin = 5.0f;
  predicted_y += (rand() % (int)(error_margin * 2)) - error_margin;

  float paddle_center = app->pad_b.pos.y + app->pad_b.size.y / 2.0f;
  float target_y = predicted_y;

  float dead_zone = app->pad_b.size.y * 0.3f;

  if (target_y < paddle_center - dead_zone) {
    rmp_vec2_set(&app->pad_b.vel, 0, -app->pad_speed);
  }
  else if (target_y > paddle_center + dead_zone) {
    rmp_vec2_set(&app->pad_b.vel, 0, app->pad_speed);
  }
  else {
    rmp_vec2_set(&app->pad_b.vel, 0, 0);
  }
}

