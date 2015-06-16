# Ring Buffer Manager

Disruptor-inspired graph computing.

`typedef struct buffmgr` type


`typedef void *(*consumer)(void *arguments)` type


`extern buffmgr *
buffmgr_make(size_t requested_slots, short n_consumers, consumer *consumers, bool consumer_deps[n_consumers][n_consumers])` function


`extern void buffmgr_start(buffmgr *bm)` function


`extern void buffmgr_pause(buffmgr *bm)` function


`extern void buffmgr_resume(buffmgr *bm)` function


`extern void buffmgr_destroy(buffmgr *bm)` function


`extern size_t buffmgr_getslot(buffmgr *bm)` function


`extern size_t buffmgr_getslot_spin(buffmgr *bm)` function


`extern void buffmgr_advanceslot(buffmgr *bm)` function


`extern bool buffmgr_is_empty(buffmgr *bm)` function


`extern void buffmgr_join_spin(buffmgr *bm)` function


`CONSUMER(CONSUMER_FUNCTION, SLOT)` macro


`END_CONSUMER` macro


`N_CONSUMERS(CS)` macro
