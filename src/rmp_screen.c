#include "rmp_screen.h"
#include "rmp_app.h"
#include "rmp_time.h"
#include "rmp_log.h"

#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

#define RMP_SCREEN_TARGET_FPS 60
#define RMP_SCREEN_FRAME_TIME_US (1000000 / RMP_SCREEN_TARGET_FPS)
#define BACKGROUND_COLOR 0xff000000
#define PAD_COLOR        0xffffffff
#define BALL_COLOR       0xffffffff

static void render(rmp_screen_t* screen, rmp_app_t* app);
static void draw_rectangle(rmp_screen_t* screen, int x, int y, int width, int height, uint32_t color);

rmp_screenRet_e rmp_screen_init(rmp_screen_t* screen, rmp_app_t* app) {
  if (!screen || !app) {
    return RMP_SCREEN_BAD_ARGS;
  }

  int rc = screen_create_context(&screen->ctx, 0);
  if (rc) {
    rmp_log_error("screen", "Failed to create screen context\n");
    return RMP_SCREEN_BAD_INIT;
  }

  rc = screen_create_window(&screen->win, screen->ctx);
  if (rc) {
    rmp_log_error("screen", "Failed to create screen window\n");
    screen_destroy_context(screen->ctx);
    return RMP_SCREEN_BAD_INIT;
  }

  int format = SCREEN_FORMAT_RGBA8888;
  screen_set_window_property_iv(screen->win, SCREEN_PROPERTY_FORMAT, &format);

  int usage = SCREEN_USAGE_ROTATION | SCREEN_USAGE_WRITE;
  screen_set_window_property_iv(screen->win, SCREEN_PROPERTY_USAGE, &usage);

  int position[2] = {50, 30};
screen_set_window_property_iv(screen->win, SCREEN_PROPERTY_POSITION, position);

  rc = screen_create_window_buffers(screen->win, 1);
  if (rc) {
    rmp_log_error("screen", "Failed to create screen window buffers\n");
    screen_destroy_window(screen->win);
    screen_destroy_context(screen->ctx);
    return RMP_SCREEN_BAD_INIT;
  }

  screen_get_window_property_pv(screen->win, SCREEN_PROPERTY_RENDER_BUFFERS,
                                (void**)&screen->buf);

  screen->app = app;

  return RMP_SCREEN_OK;
}

rmp_screenRet_e rmp_screen_free(rmp_screen_t* screen) {
  if (!screen) {
    return RMP_SCREEN_BAD_ARGS;
  }

  screen_destroy_window(screen->win);
  screen_destroy_context(screen->ctx);

  return RMP_SCREEN_OK;
}

void* rmp_screen_run(void* args) {
  if (!args) {
    return NULL;
  }

  rmp_screen_t* screen = (rmp_screen_t*)args;
  rmp_app_t* app = screen->app;

  while (true) {
    pthread_mutex_lock(&app->mutex);
    if (!app->running) {
      break;
    }
    pthread_mutex_unlock(&app->mutex);

    time_t frame_start = rmp_time_get_us();
    render(screen, app);
    time_t frame_duration = rmp_time_get_us() - frame_start;

    if (frame_duration < RMP_SCREEN_FRAME_TIME_US) {
      usleep(RMP_SCREEN_FRAME_TIME_US - frame_duration);
    }
  }
  pthread_mutex_unlock(&app->mutex);

  return NULL;
}

static void render(rmp_screen_t* screen, rmp_app_t* app) {
  if (!screen || !app) {
    return;
  }

  int win_background[] = {SCREEN_BLIT_COLOR, BACKGROUND_COLOR, SCREEN_BLIT_END};
  screen_fill(screen->ctx, screen->buf, win_background);

  draw_rectangle(screen,
                 app->pad_a.pos.x,
                 app->pad_a.pos.y,
                 app->pad_a.size.x,
                 app->pad_a.size.y,
                 PAD_COLOR);

  draw_rectangle(screen,
                 app->pad_b.pos.x,
                 app->pad_b.pos.y,
                 app->pad_b.size.x,
                 app->pad_b.size.y,
                 PAD_COLOR);

  draw_rectangle(screen,
                 app->ball.pos.x,
                 app->ball.pos.y,
                 app->ball.size.x,
                 app->ball.size.y,
                 PAD_COLOR);

  draw_rectangle(screen,
                 RMP_SCREEN_START_VEC.x,
                 RMP_SCREEN_START_VEC.y,
                 5,
                 5,
                 0xffff0000);

  draw_rectangle(screen,
                 RMP_SCREEN_END_VEC.x,
                 RMP_SCREEN_END_VEC.y,
                 5,
                 5,
                 0xffff0000);

  screen_post_window(screen->win, screen->buf, 0, NULL, 0);
}

static void draw_rectangle(rmp_screen_t* screen, int x, int y, int width, int height, uint32_t color) {
  if (!screen) {
    return;
  }

  int attribs[] = {
    SCREEN_BLIT_DESTINATION_X, x,
    SCREEN_BLIT_DESTINATION_Y, y,
    SCREEN_BLIT_DESTINATION_WIDTH, width,
    SCREEN_BLIT_DESTINATION_HEIGHT, height,
    SCREEN_BLIT_COLOR, color,
    SCREEN_BLIT_END
  };

  screen_fill(screen->ctx, screen->buf, attribs);
}
