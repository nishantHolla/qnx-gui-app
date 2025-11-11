#ifndef RMP_APP_H_
#define RMP_APP_H_

#include "rmp_vec2.h"

#include <stdbool.h>
#include <pthread.h>

extern const rmp_vec2_t RMP_SCREEN_START_VEC;
extern const rmp_vec2_t RMP_SCREEN_END_VEC;

typedef enum {
  RMP_APP_OK,
  RMP_APP_BAD_ARGS
} rmp_appRet_e;

typedef struct {
  rmp_vec2_t pos;
  rmp_vec2_t vel;
  rmp_vec2_t size;
} rmp_app_entity_t;

typedef struct {
  bool running;
  bool paused;
  pthread_mutex_t mutex;
  pthread_cond_t cond;

  int pad_speed;
  rmp_app_entity_t pad_a;
  rmp_app_entity_t pad_b;
  rmp_app_entity_t ball;
} rmp_app_t;

rmp_appRet_e rmp_app_init(rmp_app_t* app);
rmp_appRet_e rmp_app_free(rmp_app_t* app);
void* rmp_app_run(void* args);
void rmp_app_log_entity(const char* name, rmp_app_entity_t entity);

#endif // !RMP_APP_H_
