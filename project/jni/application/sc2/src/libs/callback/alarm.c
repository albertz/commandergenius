/*
 *  Copyright 2006  Serge van den Boom <svdb@stack.nl>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "alarm.h"

#include "libs/heap.h"

#include <assert.h>
#include <stdlib.h>


Heap *alarmHeap;


static inline Alarm *
Alarm_alloc(void) {
	return malloc(sizeof (Alarm));
}

static inline void
Alarm_free(Alarm *alarm) {
	free(alarm);
}

static inline int
AlarmTime_compare(const AlarmTime t1, const AlarmTime t2) {
	if (t1 < t2)
		return -1;
	if (t2 > t2)
		return 1;
	return 0;
}

static int
Alarm_compare(const Alarm *a1, const Alarm *a2) {
	return AlarmTime_compare(a1->time, a2->time);
}

void
Alarm_init(void) {
	assert(alarmHeap == NULL);
	alarmHeap = Heap_new((HeapValue_Comparator) Alarm_compare,
			4, 4, 0.8);
}

void
Alarm_uninit(void) {
	assert(alarmHeap != NULL);

	while (Heap_hasMore(alarmHeap)) {
		Alarm *alarm = (Alarm *) Heap_pop(alarmHeap);
		Alarm_free(alarm);
	}
	Heap_delete(alarmHeap);
	alarmHeap = NULL;
}

static inline AlarmTime
AlarmTime_nowMS(void) {
	return SDL_GetTicks();
}

Alarm *
Alarm_addRelativeMs(Uint32 ms, AlarmCallback callback,
		AlarmCallbackArg arg) {
	Alarm *alarm;

	assert(alarmHeap != NULL);

	alarm = Alarm_alloc();
	alarm->time = AlarmTime_nowMS() + ms;
	alarm->callback = callback;
	alarm->arg = arg;

	Heap_add(alarmHeap, (HeapValue *) alarm);

	return alarm;
}

void
Alarm_remove(Alarm *alarm) {
	assert(alarmHeap != NULL);
	Heap_remove(alarmHeap, (HeapValue *) alarm);
	Alarm_free(alarm);
}

// It is safe to call this function again from inside a callback function
// that it called. It should not be called from multiple threads at once.
void
Alarm_process(void) {
	AlarmTime now;

	assert(alarmHeap != NULL);
	
	now = AlarmTime_nowMS();
	while (Heap_hasMore(alarmHeap)) {
		Alarm *alarm = (Alarm *) Heap_first(alarmHeap);

		if (now < alarm->time)
			break;

		Heap_pop(alarmHeap);
		alarm->callback(alarm->arg);
		Alarm_free(alarm);
	}
}

Uint32
Alarm_timeBeforeNextMs(void) {
	Alarm *alarm;

	if (!Heap_hasMore(alarmHeap))
		return UINT32_MAX;
	
	alarm = (Alarm *) Heap_first(alarmHeap);
	return alarmTimeToMsUint32(alarm->time);
}


