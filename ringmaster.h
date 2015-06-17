#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>

typedef void *(*consumer)(void *arguments);

typedef enum {
  ACTIVE, PAUSED, STOPPED
} ringmaster_state;

typedef struct ringmaster_t {
  pthread_t *threads;
  consumer *consumers;
  volatile size_t *consumer_slots;
  void *consumer_deps;
  size_t head;
  size_t tail;
  size_t n_slots;
  size_t n_slots_mask;
  short n_consumers;
  volatile ringmaster_state state;
} ringmaster_t;

typedef struct consumer_args {
  ringmaster_t *rm;
  unsigned short consumer;
} consumer_args;

extern ringmaster_t *
ringmaster_create(size_t requested_slots,
                  short n_consumers,
                  consumer *consumers,
                  short consumer_deps[n_consumers][n_consumers]);

extern size_t
ringmaster_slotavailable_priv(ringmaster_t *rm, unsigned short consumer);

extern void
ringmaster_finishslot_priv(ringmaster_t *rm, unsigned short consumer);

extern void ringmaster_start(ringmaster_t *rm);
extern void ringmaster_pause(ringmaster_t *rm);
extern void ringmaster_resume(ringmaster_t *rm);
extern void ringmaster_destroy(ringmaster_t *rm);
extern size_t ringmaster_getslot(ringmaster_t *rm);
extern size_t ringmaster_getslot_spin(ringmaster_t *rm);
extern void ringmaster_advanceslot(ringmaster_t *rm);
extern bool ringmaster_is_empty(ringmaster_t *rm);
extern void ringmaster_join_spin(ringmaster_t *rm);

#define CONSUMER(CONSUMER_FUNCTION, SLOT)                               \
  void *CONSUMER_FUNCTION(void *rm_arguments) {                         \
    __unused char rm_consumer_name[] = #CONSUMER_FUNCTION;              \
    consumer_args *rm_args = (consumer_args *)rm_arguments;             \
    ringmaster_t *rm_rm = rm_args->rm;                                  \
    short rm_consumer = rm_args->consumer;                              \
                                                                        \
    free(rm_arguments);                                                 \
                                                                        \
    for (;;) {                                                          \
      ringmaster_state rm_state = rm_rm->state;                         \
      if (rm_state == STOPPED) {                                        \
        pthread_exit(NULL);                                             \
        break;                                                          \
      } else if (rm_state == PAUSED)                                    \
        continue;                                                       \
                                                                        \
      size_t rm_i = ringmaster_slotavailable_priv(rm_rm, rm_consumer);  \
      if (rm_i == -1)                                                   \
        continue;                                                       \
                                                                        \
      __unused size_t SLOT = (size_t)rm_i;                              \

#define END_CONSUMER                                        \
      ringmaster_finishslot_priv(rm_rm, rm_consumer);       \
    }                                                       \
                                                            \
    return NULL;                                            \
  }

#define N_CONSUMERS(CS) (sizeof(CS)/sizeof(consumer))
