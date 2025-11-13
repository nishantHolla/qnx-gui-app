#ifndef RMP_SCREEN_H_
#define RMP_SCREEN_H_

#include "rmp_app.h"

#include <screen/screen.h>

typedef enum {
  RMP_SCREEN_OK,
  RMP_SCREEN_BAD_ARGS,
  RMP_SCREEN_BAD_INIT
} rmp_screenRet_e;

typedef struct {
  screen_context_t ctx;
  screen_window_t win;
  screen_buffer_t buf;
  screen_event_t event;

  rmp_app_t* app;
} rmp_screen_t;

rmp_screenRet_e rmp_screen_init(rmp_screen_t* screen, rmp_app_t* app);
rmp_screenRet_e rmp_screen_free(rmp_screen_t* screen);
void* rmp_screen_run(void* args);

#endif // !RMP_SCREEN_H_
