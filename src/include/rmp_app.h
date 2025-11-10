#ifndef RMP_APP_H_
#define RMP_APP_H_

#include <stdbool.h>
#include <pthread.h>

typedef enum {
  RMP_APP_OK,
  RMP_APP_BAD_ARGS
} rmp_appRet_e;

typedef struct {
  bool running;
  pthread_mutex_t mutex;
  pthread_cond_t cond;
} rmp_app_t;

rmp_appRet_e rmp_app_init(rmp_app_t* app);
rmp_appRet_e rmp_app_free(rmp_app_t* app);
void* rmp_app_run(void* args);

#endif // !RMP_APP_H_
