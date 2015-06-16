# Ring Buffer Manager

Disruptor-inspired graph computing.

`typedef struct ringmgr_t` type


`typedef void *(*consumer)(void *arguments)` type


`extern ringmgr_t *
ringmgr_make(size_t requested_slots, short n_consumers, consumer *consumers, bool consumer_deps[n_consumers][n_consumers])` function


`extern void ringmgr_start(ringmgr_t *rm)` function


`extern void ringmgr_pause(ringmgr_t *rm)` function


`extern void ringmgr_resume(ringmgr_t *rm)` function


`extern void ringmgr_destroy(ringmgr_t *rm)` function


`extern size_t ringmgr_getslot(ringmgr_t *rm)` function


`extern size_t ringmgr_getslot_spin(ringmgr_t *rm)` function


`extern void ringmgr_advanceslot(ringmgr_t *rm)` function


`extern bool ringmgr_is_empty(ringmgr_t *rm)` function


`extern void ringmgr_join_spin(ringmgr_t *rm)` function


`CONSUMER(CONSUMER_FUNCTION, SLOT)` macro


`END_CONSUMER` macro


`N_CONSUMERS(CS)` macro
