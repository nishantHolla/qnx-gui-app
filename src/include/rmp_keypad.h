#ifndef RMP_KEYPAD_H_
#define RMP_KEYPAD_H_

#include "rmp_app.h"

typedef enum {
  RMP_KEYPAD_OK,
  RMP_KEYPAD_BAD_ARGS,
  RMP_KEYPAD_BAD_INIT
} rmp_keypadRet_e;

typedef struct {
  int keys[16];
  int row_pins[4];
  int col_pins[4];

  rmp_app_t* app;
} rmp_keypad_t;

rmp_keypadRet_e rmp_keypad_init(rmp_keypad_t* keypad, rmp_app_t* app);
void* rmp_keypad_run(void* args);

#endif // !RMP_KEYPAD_H_
