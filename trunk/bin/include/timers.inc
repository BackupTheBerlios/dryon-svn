#pragma library Timers

native timer_id:timer_add(const name[], seconds, ...);
native timer_remove(timer_id:id);
native bool:timer_valid(timer_id:id);
