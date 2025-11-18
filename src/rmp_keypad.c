#include "rmp_keypad.h"
#include "rmp_log.h"
#include "rmp_time.h"
#include "external/rpi_gpio.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

#define RMP_KEYPAD_TARGET_FPS 60
#define RMP_KEYPAD_FRAME_TIME_US (1000000 / RMP_KEYPAD_TARGET_FPS)

#define RMP_KEYDOWN    0x10
#define RMP_KEYUP      0x00

#define RMP_KEY0       0x00
#define RMP_KEY1       0x01
#define RMP_KEY2       0x02
#define RMP_KEY3       0x03
#define RMP_KEY4       0x04
#define RMP_KEY5       0x05
#define RMP_KEY6       0x06
#define RMP_KEY7       0x07
#define RMP_KEY8       0x08
#define RMP_KEY9       0x09
#define RMP_KEYA       0x0a
#define RMP_KEYB       0x0b
#define RMP_KEYC       0x0c
#define RMP_KEYD       0x0d
#define RMP_KEYE       0x0e
#define RMP_KEYF       0x0f

#define RMP_EVENT_QUIT             RMP_KEY3
#define RMP_EVENT_PLAY_PAUSE       RMP_KEYC
#define RMP_EVENT_PAD_A_UP         RMP_KEY0
#define RMP_EVENT_PAD_A_DOWN       RMP_KEY4
#define RMP_EVENT_PAD_B_UP         RMP_KEYB
#define RMP_EVENT_PAD_B_DOWN       RMP_KEYF
#define RMP_EVENT_TOGGLE_AI        RMP_KEYD

#define RMP_EVENT_TOGGLE_RECAL     RMP_KEYE
#define RMP_EVENT_RECAL_TL_LEFT    RMP_KEY0
#define RMP_EVENT_RECAL_TL_DOWN    RMP_KEY1
#define RMP_EVENT_RECAL_TL_UP      RMP_KEY2
#define RMP_EVENT_RECAL_TL_RIGHT   RMP_KEY3
#define RMP_EVENT_RECAL_BR_LEFT    RMP_KEY4
#define RMP_EVENT_RECAL_BR_DOWN    RMP_KEY5
#define RMP_EVENT_RECAL_BR_UP      RMP_KEY6
#define RMP_EVENT_RECAL_BR_RIGHT   RMP_KEY7

static rmp_keypadRet_e init_gpio(int rows[4], int cols[4]);
static rmp_keypadRet_e scan_keypad(int rows[4], int cols[4], int keys[16]);
static void handle_event(uint8_t event, rmp_app_t* app);
static void handle_game_event(uint8_t event, rmp_app_t* app);
static void handle_recal_event(uint8_t event, rmp_app_t* app);

rmp_keypadRet_e rmp_keypad_init(rmp_keypad_t* keypad, rmp_app_t* app) {
  if (!keypad || !app) {
    return RMP_KEYPAD_BAD_ARGS;
  }

  const int row_pins[4] = {18, 23, 24, 25};
  const int col_pins[4] = {12, 16, 20, 21};

  memset(keypad->keys, 0, sizeof(keypad->keys));
  memcpy(keypad->row_pins, row_pins, sizeof(keypad->row_pins));
  memcpy(keypad->col_pins, col_pins, sizeof(keypad->col_pins));

  rmp_keypadRet_e ret = init_gpio(keypad->row_pins, keypad->col_pins);
  if (ret != RMP_KEYPAD_OK) {
    rmp_log_error("keypad", "Failed to initialize gpio pins\n");
    return ret;
  }

  rmp_log_info("keypad", "Initialized row pins: ");
  for (int i = 0; i < 4; ++i) {
    printf("%d ", keypad->row_pins[i]);
  }
  printf("\n");

  rmp_log_info("keypad", "Initialized col pins: ");
  for (int i = 0; i < 4; ++i) {
    printf("%d ", keypad->col_pins[i]);
  }
  printf("\n");

  keypad->app = app;

  rmp_log_info("keypad", "Initialized keypad\n");
  return RMP_KEYPAD_OK;
}

void* rmp_keypad_run(void* args) {
  if (!args) {
    return NULL;
  }

  rmp_keypad_t* keypad = (rmp_keypad_t*)args;
  rmp_app_t* app = keypad->app;

  rmp_log_info("keypad", "Started keypad scan\n");
  while (true) {
    pthread_mutex_lock(&app->mutex);
    if (!app->running) {
      break;
    }
    pthread_mutex_unlock(&app->mutex);

    time_t frame_start = rmp_time_get_us();

    int old_keys[16];
    memcpy(old_keys, keypad->keys, sizeof(old_keys));
    scan_keypad(keypad->row_pins, keypad->col_pins, keypad->keys);

    for (int i = 0; i < 16; ++i) {
      if (old_keys[i] != keypad->keys[i]) {
        uint8_t event = (keypad->keys[i]) ? (RMP_KEYDOWN | i) : (RMP_KEYUP | i);
        handle_event(event, app);
      }
    }

    time_t frame_duration = rmp_time_get_us() - frame_start;
    if (frame_duration < RMP_KEYPAD_FRAME_TIME_US) {
      usleep(RMP_KEYPAD_FRAME_TIME_US - frame_duration);
    }
  }
  pthread_mutex_unlock(&app->mutex);

  return NULL;
}

static rmp_keypadRet_e init_gpio(int rows[4], int cols[4]) {
  for (int i = 0; i < 4; i++) {
    if (rpi_gpio_setup(rows[i], GPIO_OUT) || rpi_gpio_output(rows[i], GPIO_HIGH)) {
      rmp_log_error("keypad", "Failed to initialize row pins\n");
      return RMP_KEYPAD_BAD_INIT;
    }

    if (rpi_gpio_setup_pull(cols[i], GPIO_IN, GPIO_PUD_UP)) {
      rmp_log_error("keypad", "Failed to initialize column pins\n");
      return RMP_KEYPAD_BAD_INIT;
    }
  }

  return RMP_KEYPAD_OK;
}

static rmp_keypadRet_e scan_keypad(int rows[4], int cols[4], int keys[16]) {
  unsigned level;

  for (int r = 0; r < 4; r++) {
    rpi_gpio_output(rows[r], GPIO_LOW);
    usleep(10000);

    for (int c = 0; c < 4; c++) {
      if (rpi_gpio_input(cols[c], &level) != 0) {
        continue;
      }

      keys[c * 4 + r] = (level == GPIO_LOW) ? 1 : 0;
    }

    rpi_gpio_output(rows[r], GPIO_HIGH);
  }

  return RMP_KEYPAD_OK;
}


static void handle_event(uint8_t event, rmp_app_t* app) {
  pthread_mutex_lock(&app->mutex);

  if (app->recalibrating) {
    handle_recal_event(event, app);
  }
  else {
    handle_game_event(event, app);
  }

  pthread_mutex_unlock(&app->mutex);
}

static void handle_game_event(uint8_t event, rmp_app_t* app) {
  rmp_vec2_t v;

  switch (event) {
    case RMP_KEYUP | RMP_EVENT_QUIT:
      app->running = false;
      pthread_cond_signal(&app->cond);
      break;

    case RMP_KEYUP | RMP_EVENT_PLAY_PAUSE:
      app->paused = !app->paused;
      break;

    case RMP_KEYUP | RMP_EVENT_TOGGLE_AI:
      app->ai_is_playing = !app->ai_is_playing;
      if (!app->ai_is_playing) {
        rmp_vec2_set(&app->pad_b.vel, 0, 0);
      }
      break;

    case RMP_KEYUP | RMP_EVENT_TOGGLE_RECAL:
      app->recalibrating = !app->recalibrating;
      break;

    case RMP_KEYDOWN | RMP_EVENT_PAD_A_UP:
    case RMP_KEYUP | RMP_EVENT_PAD_A_DOWN:
      rmp_vec2_set(&v, 0, -app->pad_speed);
      rmp_vec2_add(&app->pad_a.vel, app->pad_a.vel, v);
      break;

    case RMP_KEYUP | RMP_EVENT_PAD_A_UP:
    case RMP_KEYDOWN | RMP_EVENT_PAD_A_DOWN:
      rmp_vec2_set(&v, 0, app->pad_speed);
      rmp_vec2_add(&app->pad_a.vel, app->pad_a.vel, v);
      break;

    case RMP_KEYDOWN | RMP_EVENT_PAD_B_UP:
    case RMP_KEYUP | RMP_EVENT_PAD_B_DOWN:
      if (app->ai_is_playing) {
        break;
      };
      rmp_vec2_set(&v, 0, -app->pad_speed);
      rmp_vec2_add(&app->pad_b.vel, app->pad_b.vel, v);
      break;

    case RMP_KEYUP | RMP_EVENT_PAD_B_UP:
    case RMP_KEYDOWN | RMP_EVENT_PAD_B_DOWN:
      if (app->ai_is_playing) {
        break;
      };
      rmp_vec2_set(&v, 0, app->pad_speed);
      rmp_vec2_add(&app->pad_b.vel, app->pad_b.vel, v);
      break;
  }
}

static void handle_recal_event(uint8_t event, rmp_app_t* app) {
  rmp_vec2_t v;
  const int step = 5;

  switch (event) {
    case RMP_KEYUP | RMP_EVENT_RECAL_TL_LEFT:
      rmp_vec2_set(&v, -step, 0);
      rmp_vec2_add(&app->SCREEN_START, app->SCREEN_START, v);
      break;

    case RMP_KEYUP | RMP_EVENT_RECAL_TL_DOWN:
      rmp_vec2_set(&v, 0, step);
      rmp_vec2_add(&app->SCREEN_START, app->SCREEN_START, v);
      break;

    case RMP_KEYUP | RMP_EVENT_RECAL_TL_UP:
      rmp_vec2_set(&v, 0, -step);
      rmp_vec2_add(&app->SCREEN_START, app->SCREEN_START, v);
      break;

    case RMP_KEYUP | RMP_EVENT_RECAL_TL_RIGHT:
      rmp_vec2_set(&v, step, 0);
      rmp_vec2_add(&app->SCREEN_START, app->SCREEN_START, v);
      break;

    case RMP_KEYUP | RMP_EVENT_RECAL_BR_LEFT:
      rmp_vec2_set(&v, -step, 0);
      rmp_vec2_add(&app->SCREEN_END, app->SCREEN_END, v);
      break;

    case RMP_KEYUP | RMP_EVENT_RECAL_BR_DOWN:
      rmp_vec2_set(&v, 0, step);
      rmp_vec2_add(&app->SCREEN_END, app->SCREEN_END, v);
      break;

    case RMP_KEYUP | RMP_EVENT_RECAL_BR_UP:
      rmp_vec2_set(&v, 0, -step);
      rmp_vec2_add(&app->SCREEN_END, app->SCREEN_END, v);
      break;

    case RMP_KEYUP | RMP_EVENT_RECAL_BR_RIGHT:
      rmp_vec2_set(&v, step, 0);
      rmp_vec2_add(&app->SCREEN_END, app->SCREEN_END, v);
      break;

    case RMP_KEYUP | RMP_EVENT_TOGGLE_RECAL:
      app->recalibrating = !app->recalibrating;
      rmp_app_recalibrate(app);
      break;
  }
}
