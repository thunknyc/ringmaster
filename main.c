#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

struct obstructor;

typedef void (*consumer)(struct obstructor *o, unsigned int consumer);

typedef enum {
  ACTIVE, PAUSED, KILLED
} obstructor_state;

typedef struct obstructor {
  consumer *consumers;
  void *slots;
  unsigned long *consumer_slot;
  unsigned long head;
  unsigned long tail;
  unsigned long n_slots;
  unsigned int n_consumers;
  unsigned short obstructor_state;
  obstructor_state state;
} obstructor;

unsigned long next_nat_pow_2(unsigned long n) {
  unsigned long x = 0;

  if(!n--) return 1;

  while(n) {
    n >>= 1;
    x = (x << 1) | 1;
  }
  return ++x;
}

unsigned long incr_index(obstructor *o, unsigned long i) {
  return 0;
}

obstructor *make_obstructor(unsigned long requested_slots,
                            unsigned int n_consumers,
                            consumer *consumers) {
  return NULL;
}

void start_obstructor(void) {

}

void stop_obstructor(void) {

}

void destroy_obstructor(obstructor *o) {

}

bool provide_obstructor(obstructor *o, void *v) {
  return false;
}

void provide_obstructor_block(obstructor *o, void *v) {
  while(!provide_obstructor(o, v));
}

int main(int argc, char **argv) {
  obstructor *o = make_obstructor(30, 0, NULL);
  provide_obstructor_block(o, (void *)42);
  return 0;
}
