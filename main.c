#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "obstructor.h"

#define LOG(...) fprintf(stderr, __VA_ARGS__)

CONSUMER(echo_consumer, long, datum) {
  // LOG("ECHO (%hu): %ld.\n", consumer, datum);
} END_CONSUMER

#define RING_BUFFER_SIZE 10

int main(int argc, char **argv) {
  bool deps[3][3] = {{false, false, false},
                     {true, false, false},
                     {false, true, false}};
  consumer consumers[] = {echo_consumer, echo_consumer, echo_consumer};
  obstructor *o = make_obstructor(RING_BUFFER_SIZE, 3, consumers, deps);

  assert(argc == 2);
  assert(o != NULL);

  long iterations = atol(argv[1]);
  assert(iterations > 0);

  LOG("iterations: %ld\n", iterations);

  start_obstructor(o);

  for (long i = 0; i < iterations; i++)
    provide_obstructor_block(o, (void *)i);

  join_obstructor(o);
  destroy_obstructor(o);

  return 0;
}
