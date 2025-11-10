#include "rmp_screen.h"
#include "rmp_app.h"
#include "rmp_time.h"
#include "rmp_log.h"

#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

#define RMP_SCREEN_TARGET_FPS 3
#define RMP_SCREEN_FRAME_TIME_US (1000000 / RMP_SCREEN_TARGET_FPS)
#define BACKGROUND_COLOR 0xff0000ff

static void render(rmp_screen_t* screen, rmp_app_t* app);

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
  int win_background[] = {SCREEN_BLIT_COLOR, BACKGROUND_COLOR, SCREEN_BLIT_END};
  screen_fill(screen->ctx, screen->buf, win_background);

  screen_post_window(screen->win, screen->buf, 0, NULL, 0);
}
