#include "rmp_keypad.h"
#include "rmp_log.h"
#include "rmp_time.h"
#include "external/rpi_gpio.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

#define RMP_KEYPAD_TARGET_FPS 60
#define RMP_KEYPAD_FRAME_TIME_US (1000000 / RMP_KEYPAD_TARGET_FPS)

static rmp_keypadRet_e init_gpio(int rows[4], int cols[4]);
static rmp_keypadRet_e scan_keypad(int rows[4], int cols[4], int keys[16]);

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

  return RMP_KEYPAD_OK;
}

void* rmp_keypad_run(void* args) {
  if (!args) {
    return NULL;
  }

  rmp_keypad_t* keypad = (rmp_keypad_t*)args;
  rmp_app_t* app = keypad->app;

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
      char c = (i > 9) ? (i - 10) + 'a' : i + '0';
      if (old_keys[i] != keypad->keys[i]) {
        if (keypad->keys[i]) {
          rmp_log_info("keypad", "KEYDOWN %c\n", c);
        }
        else {
          rmp_log_info("keypad", "KEYUP %c\n", c);
        }
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
