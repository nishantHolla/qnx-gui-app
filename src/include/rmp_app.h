#ifndef RMP_APP_H_
#define RMP_APP_H_

#include "rmp_vec2.h"

#include <stdbool.h>
#include <pthread.h>

#define SCREEN_WIDTH_P(a) ((a)->SCREEN_END.x - (a)->SCREEN_START.x)
#define SCREEN_WIDTH(a) ((a).SCREEN_END.x - (a).SCREEN_START.x)
#define SCREEN_HEIGHT_P(a) ((a)->SCREEN_END.y - (a)->SCREEN_START.y)
#define SCREEN_HEIGHT(a) ((a).SCREEN_END.y - (a).SCREEN_START.y)

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
  bool recalibrating;
  bool ai_is_playing;

  pthread_mutex_t mutex;
  pthread_cond_t cond;

  rmp_vec2_t SCREEN_START;
  rmp_vec2_t SCREEN_END;

  int pad_speed;
  int pad_padding;
  rmp_vec2_t pad_size;
  rmp_app_entity_t pad_a;
  rmp_app_entity_t pad_b;

  int ball_size;
  rmp_app_entity_t ball;
} rmp_app_t;

rmp_appRet_e rmp_app_init(rmp_app_t* app);
rmp_appRet_e rmp_app_free(rmp_app_t* app);
void* rmp_app_run(void* args);
void rmp_app_log_entity(const char* name, rmp_app_entity_t entity);
void rmp_app_recalibrate(rmp_app_t* app);

#endif // !RMP_APP_H_
