# Ringmaster: Ring Buffer Manager

Disruptor-inspired graph computing.

type  
`typedef struct ringmaster_t`

type  
`typedef void *(*consumer)(void *arguments)`

function  
`extern ringmaster_t *
ringmaster_make(size_t requested_slots, short n_consumers, consumer *consumers, bool consumer_deps[n_consumers][n_consumers])`

function  
`extern void ringmaster_start(ringmaster_t *rm)`

function  
`extern void ringmaster_pause(ringmaster_t *rm)`

function  
`extern void ringmaster_resume(ringmaster_t *rm)`

function  
`extern void ringmaster_destroy(ringmaster_t *rm)`

function  
`extern size_t ringmaster_getslot(ringmaster_t *rm)`

function  
`extern size_t ringmaster_getslot_spin(ringmaster_t *rm)`

function  
`extern void ringmaster_advanceslot(ringmaster_t *rm)`

function  
`extern bool ringmaster_is_empty(ringmaster_t *rm)`

function  
`extern void ringmaster_join_spin(ringmaster_t *rm)`

macro  
`CONSUMER(CONSUMER_FUNCTION, SLOT)`

macro  
`END_CONSUMER`

macro  
`N_CONSUMERS(CS)`
