#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>

struct ringmgr;

typedef void *(*consumer)(void *arguments);

typedef enum {
  ACTIVE, PAUSED, STOPPED
} ringmgr_state;

typedef struct ringmgr_t {
  pthread_t *threads;
  consumer *consumers;
  volatile size_t *consumer_slots;
  void *consumer_deps;
  size_t head;
  size_t tail;
  size_t n_slots;
  size_t n_slots_mask;
  short n_consumers;
  volatile ringmgr_state state;
} ringmgr_t;

typedef struct consumer_args {
  ringmgr_t *rm;
  unsigned short consumer;
} consumer_args;

extern ringmgr_t *
ringmgr_make(size_t requested_slots,
             short n_consumers,
             consumer *consumers,
             bool consumer_deps[n_consumers][n_consumers]);

extern long long
ringmgr_slotavailable_priv(ringmgr_t *rm, unsigned short consumer);

extern void ringmgr_finishslot_priv(ringmgr_t *rm, unsigned short consumer);
extern void ringmgr_start(ringmgr_t *rm);
extern void ringmgr_pause(ringmgr_t *rm);
extern void ringmgr_resume(ringmgr_t *rm);
extern void ringmgr_destroy(ringmgr_t *rm);
extern size_t ringmgr_getslot(ringmgr_t *rm);
extern size_t ringmgr_getslot_spin(ringmgr_t *rm);
extern void ringmgr_advanceslot(ringmgr_t *rm);
extern bool ringmgr_is_empty(ringmgr_t *rm);
extern void ringmgr_join_spin(ringmgr_t *rm);

#define CONSUMER(CONSUMER_FUNCTION, SLOT)                               \
  void *CONSUMER_FUNCTION(void *rm_arguments) {                         \
    __unused char rm_consumer_name[] = #CONSUMER_FUNCTION;              \
    consumer_args *rm_args = (consumer_args *)rm_arguments;             \
    ringmgr_t *rm_rm = rm_args->rm;                                     \
    unsigned short rm_consumer = rm_args->consumer;                     \
                                                                        \
    free(rm_arguments);                                                 \
                                                                        \
    for (;;) {                                                          \
      ringmgr_state rm_state = rm_rm->state;                            \
      if (rm_state == STOPPED) {                                        \
        pthread_exit(NULL);                                             \
        break;                                                          \
      } else if (rm_state == PAUSED)                                    \
        continue;                                                       \
                                                                        \
      long long rm_i = ringmgr_slotavailable_priv(rm_rm, rm_consumer);  \
      if (rm_i == -1)                                                   \
        continue;                                                       \
                                                                        \
      __unused size_t SLOT = (size_t)rm_i;                              \

#define END_CONSUMER                                \
      ringmgr_finishslot_priv(rm_rm, rm_consumer);  \
     }                                              \
                                                    \
    return NULL;                                    \
  }

#define N_CONSUMERS(CS) (sizeof(CS)/sizeof(consumer))
