#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>

struct obstructor;

typedef void *(*consumer)(void *arguments);

typedef enum {
  ACTIVE, PAUSED, STOPPED
} obstructor_state;

typedef struct obstructor {
  pthread_t *threads;
  consumer *consumers;
  void **slots;
  unsigned long *consumer_slots;
  void *consumer_deps;
  unsigned long head;
  unsigned long tail;
  unsigned long n_slots;
  unsigned long n_slots_mask;
  short n_consumers;
  obstructor_state state;
} obstructor;

typedef struct consumer_args {
  obstructor *o;
  unsigned short consumer;
} consumer_args;

extern obstructor *
make_obstructor(unsigned long requested_slots,
                short n_consumers,
                consumer *consumers,
                bool consumer_deps[n_consumers][n_consumers]);

extern long long datum_available_private(obstructor *o,
                                         unsigned short consumer);

extern void finish_datum_private(obstructor *o, unsigned short consumer);
extern void start_obstructor(obstructor *o);
extern void pause_obstructor(obstructor *o);
extern void resume_obstructor(obstructor *o);
extern void destroy_obstructor(obstructor *o);
extern bool provide_obstructor(obstructor *o, void *v);
extern void provide_obstructor_block(obstructor *o, void *v);
extern bool obstructor_is_empty(obstructor *o);
extern void join_obstructor(obstructor *o);

#define CONSUMER(CONSUMER_FUNCTION, DATUM_TYPE, DATUM)          \
  void *CONSUMER_FUNCTION(void *obs_arguments) {                \
    char obs_consumer_name[] = #CONSUMER_FUNCTION;              \
    consumer_args *obs_args = (consumer_args *)obs_arguments;   \
    obstructor *obs_o = obs_args->o;                            \
    unsigned short obs_consumer = obs_args->consumer;           \
                                                                \
    free(obs_arguments);                                        \
                                                                \
    for (;;) {                                                  \
      obstructor_state obs_state = obs_o->state;                \
      if (obs_state == STOPPED) {                               \
        pthread_exit(NULL);                                     \
        break;                                                  \
      } else if (obs_state == PAUSED)                           \
        continue;                                               \
                                                                \
      long long obs_i = datum_available_private(obs_o, obs_consumer);   \
      if (obs_i == -1)                                          \
        continue;                                               \
                                                                \
      DATUM_TYPE DATUM = (DATUM_TYPE)obs_o->slots[obs_i];       \

#define END_CONSUMER                                  \
      finish_datum_private(obs_o, obs_consumer);      \
    }                                                 \
                                                      \
    return NULL;                                      \
  }

#define N_CONSUMERS(CS) (sizeof(CS)/sizeof(consumer))
