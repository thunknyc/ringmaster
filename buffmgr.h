#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>

struct buffmgr;

typedef void *(*consumer)(void *arguments);

typedef enum {
  ACTIVE, PAUSED, STOPPED
} buffmgr_state;

typedef struct buffmgr {
  pthread_t *threads;
  consumer *consumers;
  volatile size_t *consumer_slots;
  void *consumer_deps;
  size_t head;
  size_t tail;
  size_t n_slots;
  size_t n_slots_mask;
  short n_consumers;
  volatile buffmgr_state state;
} buffmgr;

typedef struct consumer_args {
  buffmgr *bm;
  unsigned short consumer;
} consumer_args;

extern buffmgr *
buffmgr_make(size_t requested_slots,
             short n_consumers,
             consumer *consumers,
             bool consumer_deps[n_consumers][n_consumers]);

extern long long
buffmgr_slotavailable_priv(buffmgr *bm, unsigned short consumer);

extern void buffmgr_finishslot_priv(buffmgr *bm, unsigned short consumer);
extern void buffmgr_start(buffmgr *bm);
extern void buffmgr_pause(buffmgr *bm);
extern void buffmgr_resume(buffmgr *bm);
extern void buffmgr_destroy(buffmgr *bm);
extern size_t buffmgr_getslot(buffmgr *bm);
extern size_t buffmgr_getslot_spin(buffmgr *bm);
extern void buffmgr_advanceslot(buffmgr *bm);
extern bool buffmgr_is_empty(buffmgr *bm);
extern void buffmgr_join_spin(buffmgr *bm);

#define CONSUMER(CONSUMER_FUNCTION, SLOT)                               \
  void *CONSUMER_FUNCTION(void *bm_arguments) {                         \
    __unused char bm_consumer_name[] = #CONSUMER_FUNCTION;              \
    consumer_args *bm_args = (consumer_args *)bm_arguments;             \
    buffmgr *bm_bm = bm_args->bm;                                       \
    unsigned short bm_consumer = bm_args->consumer;                     \
                                                                        \
    free(bm_arguments);                                                 \
                                                                        \
    for (;;) {                                                          \
      buffmgr_state bm_state = bm_bm->state;                            \
      if (bm_state == STOPPED) {                                        \
        pthread_exit(NULL);                                             \
        break;                                                          \
      } else if (bm_state == PAUSED)                                    \
        continue;                                                       \
                                                                        \
      long long bm_i = buffmgr_slotavailable_priv(bm_bm, bm_consumer);  \
      if (bm_i == -1)                                                   \
        continue;                                                       \
                                                                        \
      __unused size_t SLOT = (size_t)bm_i;                              \

#define END_CONSUMER                                \
      buffmgr_finishslot_priv(bm_bm, bm_consumer);  \
     }                                              \
                                                    \
    return NULL;                                    \
  }

#define N_CONSUMERS(CS) (sizeof(CS)/sizeof(consumer))
