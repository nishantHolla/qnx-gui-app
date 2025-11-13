#include "rmp_screen.h"
#include "rmp_app.h"
#include "rmp_time.h"
#include "rmp_log.h"
#include "rmp_config.h"

#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <sys/keycodes.h>

#define RMP_SCREEN_TARGET_FPS 120
#define RMP_SCREEN_FRAME_TIME_US (1000000 / RMP_SCREEN_TARGET_FPS)
#define BACKGROUND_COLOR 0xff000000
#define PAD_COLOR        0xffffffff
#define AI_PAD_COLOR     0xff222222
#define BALL_COLOR       0xffffffff

#if RMP_CONFIG_USE_KEYBOARD == 1
static void poll_events(rmp_screen_t* screen, rmp_app_t* app);
static void handle_keyboard_events(rmp_screen_t* screen, rmp_app_t* app, int pad_movements[2]);
#endif // RMP_CONFIG_USE_KEYBOARD == 1

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

  rc = screen_create_window_buffers(screen->win, 1);
  if (rc) {
    rmp_log_error("screen", "Failed to create screen window buffers\n");
    screen_destroy_window(screen->win);
    screen_destroy_context(screen->ctx);
    return RMP_SCREEN_BAD_INIT;
  }

  screen_get_window_property_pv(screen->win, SCREEN_PROPERTY_RENDER_BUFFERS,
                                (void**)&screen->buf);

  rc = screen_create_event(&screen->event);
  if (rc) {
    rmp_log_error("screen", "Failed to create event\n");
    screen_destroy_window(screen->win);
    screen_destroy_context(screen->ctx);
    return RMP_SCREEN_BAD_INIT;
  }

  int sensitivity = SCREEN_SENSITIVITY_ALWAYS;
  screen_set_window_property_iv(screen->win, SCREEN_PROPERTY_SENSITIVITY, &sensitivity);

  int foucs = 1;
  screen_set_window_property_iv(screen->win, SCREEN_PROPERTY_FOCUS, &foucs);

  screen->app = app;

  rmp_log_info("screen", "Initialized screen\n");
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

  rmp_log_info("screen", "Started screen render\n");
  while (true) {
    pthread_mutex_lock(&app->mutex);
    if (!app->running) {
      break;
    }
    pthread_mutex_unlock(&app->mutex);

    time_t frame_start = rmp_time_get_us();
#if RMP_CONFIG_USE_KEYBOARD == 1
    poll_events(screen, app);
#endif // RMP_CONFIG_USE_KEYBOARD == 1
    render(screen, app);
    time_t frame_duration = rmp_time_get_us() - frame_start;

    if (frame_duration < RMP_SCREEN_FRAME_TIME_US) {
      usleep(RMP_SCREEN_FRAME_TIME_US - frame_duration);
    }
  }
  pthread_mutex_unlock(&app->mutex);

  return NULL;
}

#if RMP_CONFIG_USE_KEYBOARD == 1

static void poll_events(rmp_screen_t* screen, rmp_app_t* app) {
  if (!screen || !app) {
    return;
  }

  int rc = screen_get_event(screen->ctx, screen->event, 0);
  int pad_movements[2] = {0};

  if (rc == 0) {
    pthread_mutex_lock(&app->mutex);

    int event_type;
    screen_get_event_property_iv(screen->event, SCREEN_PROPERTY_TYPE, &event_type);

    switch (event_type) {
      case SCREEN_EVENT_KEYBOARD:
        handle_keyboard_events(screen, app, pad_movements);
        break;

      case SCREEN_EVENT_CLOSE:
        app->running = false;
        pthread_cond_signal(&app->cond);
        break;
    }

    pthread_mutex_unlock(&app->mutex);
  }

  rmp_vec2_set(&app->pad_a.vel, 0, app->pad_speed * pad_movements[0]);
  rmp_vec2_set(&app->pad_b.vel, 0, app->pad_speed * pad_movements[1]);
}

static void handle_keyboard_events(rmp_screen_t* screen, rmp_app_t* app, int pad_movements[2]) {
  if (!screen || !app) {
    return;
  }

  int flags, modifiers, key_sym, key_cap;

  screen_get_event_property_iv(screen->event, SCREEN_PROPERTY_FLAGS, &flags);
  screen_get_event_property_iv(screen->event, SCREEN_PROPERTY_MODIFIERS, &modifiers);
  screen_get_event_property_iv(screen->event, SCREEN_PROPERTY_SYM, &key_sym);
  screen_get_event_property_iv(screen->event, SCREEN_PROPERTY_KEY_CAP, &key_cap);

  if ((flags & KEY_DOWN) && key_sym == KEYCODE_P) {
    app->paused = !app->paused;
  }
  else if ((flags & KEY_DOWN) && key_sym == KEYCODE_Q) {
    app->running = false;
    pthread_cond_signal(&app->cond);
  }
  else if ((flags & KEY_DOWN) && key_sym == KEYCODE_I) {
    app->ai_is_playing = !app->ai_is_playing;
  }

  pad_movements[0] = pad_movements[1] = 0;
  bool is_pressed = (flags & KEY_DOWN) || (flags & KEY_REPEAT);

  if (is_pressed && key_sym == KEYCODE_S) {
    pad_movements[0] = 1;
  }
  else if (is_pressed && key_sym == KEYCODE_W) {
    pad_movements[0] = -1;
  }
  else if (is_pressed && key_sym == KEYCODE_DOWN && !app->ai_is_playing) {
    pad_movements[1] = 1;
  }
  else if (is_pressed && key_sym == KEYCODE_UP && !app->ai_is_playing) {
    pad_movements[1] = -1;
  }
}

#endif // RMP_CONFIG_USE_KEYBOARD == 1

static void render(rmp_screen_t* screen, rmp_app_t* app) {
  if (!screen || !app) {
    return;
  }

  int win_background[] = {SCREEN_BLIT_COLOR, BACKGROUND_COLOR, SCREEN_BLIT_END};
  screen_fill(screen->ctx, screen->buf, win_background);

  /// Pad A
  draw_rectangle(screen,
                 app->pad_a.pos.x,
                 app->pad_a.pos.y,
                 app->pad_a.size.x,
                 app->pad_a.size.y,
                 PAD_COLOR);

  /// Pad B
  draw_rectangle(screen,
                 app->pad_b.pos.x,
                 app->pad_b.pos.y,
                 app->pad_b.size.x,
                 app->pad_b.size.y,
                 app->ai_is_playing ? AI_PAD_COLOR : PAD_COLOR);

  /// Ball
  draw_rectangle(screen,
                 app->ball.pos.x,
                 app->ball.pos.y,
                 app->ball.size.x,
                 app->ball.size.y,
                 PAD_COLOR);

  /// Top left corner
  draw_rectangle(screen,
                 RMP_SCREEN_START_VEC.x,
                 RMP_SCREEN_START_VEC.y,
                 5,
                 5,
                 0xffff0000);

  /// Bottom right corner
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
