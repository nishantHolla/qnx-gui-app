#include "rmp_app.h"
#include "rmp_keypad.h"
#include "rmp_screen.h"
#include "rmp_log.h"

#include <stdlib.h>

int main(void) {
  rmp_app_t app;
  rmp_app_init(&app);

  // rmp_keypad_t keypad;
  // rmp_keypad_init(&keypad, &app);

  rmp_screen_t screen;
  rmp_screen_init(&screen, &app);

  pthread_t app_tid;
  if (pthread_create(&app_tid, NULL, rmp_app_run, (void*)&app) != 0) {
    rmp_log_error("main", "Failed to create app thread\n");
    return EXIT_FAILURE;
  }

  // pthread_t keypad_tid;
  // if (pthread_create(&keypad_tid, NULL, rmp_keypad_run, (void*)&keypad) != 0) {
  //   rmp_log_error("main", "Failed to create keypad thread\n");
  //   return EXIT_FAILURE;
  // }

  pthread_t screen_tid;
  if (pthread_create(&screen_tid, NULL, rmp_screen_run, (void*)&screen) != 0) {
    rmp_log_error("main", "Failed to create screen thread\n");
    return EXIT_FAILURE;
  }

  pthread_mutex_lock(&app.mutex);
  while (app.running) {
    pthread_cond_wait(&app.cond, &app.mutex);
  }
  pthread_mutex_unlock(&app.mutex);

  pthread_join(app_tid, NULL);
  // pthread_join(keypad_tid, NULL);
  pthread_join(screen_tid, NULL);

  rmp_app_free(&app);
  rmp_screen_free(&screen);
  return EXIT_SUCCESS;
}
