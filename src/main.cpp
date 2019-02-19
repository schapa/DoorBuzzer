
#include <stdio.h>
#include <stdlib.h>

#include "bsp.h"
#include "Queue.h"
#include "timers.h"
#include "systemTimer.h"

static int s_timerHandle;

static void onStartupTimer(uint32_t id, void *data);
static void onTimerFire(uint32_t id, void *data);

static inline void onTimerPush(uint32_t id) {
	EventQueue_Push(EVENT_TIMCALL, (void*)id, NULL);
}


static void onStartupTimer(uint32_t id, void *data) {

	s_timerHandle = Timer_newArmed(BSP_TICKS_PER_SECOND /30, true, onTimerFire, data);
	System_setStatus(INFORM_ERROR);
}

static void onTimerFire(uint32_t id, void *data) {
	uint8_t *arg = (uint8_t*)data;
	static bool dir = false;
	if (*arg) {
		if (dir)
			*arg = *arg >> 1;
		else
			*arg = *arg << 1;
		BSP_SetPinPWM(*arg);
	} else {
		BSP_SetPinPWM(0);
		if (dir)
			*arg = 0x01;
		else
			*arg = 0x80;
		dir = !dir;
	}
}

int main(int argc, char* argv[]) {

	Timer_init(onTimerPush);
	BSP_Init();
	int arg = 1;

//	s_timerHandle = Timer_newArmed(6*BSP_TICKS_PER_SECOND, false, onStartupTimer, (void*)&arg);
	s_timerHandle = Timer_newArmed(BSP_TICKS_PER_SECOND/100, false, onStartupTimer, (void*)&arg);

	System_setStatus(INFORM_INIT);
	while (true) {
		Event_t event;
		EventQueue_Pend(&event);
		BSP_FeedWatchdog();
		uint32_t intVal = (uint32_t)event.data;
		switch (event.type) {
			case EVENT_SYSTICK:
				break;
			case EVENT_TIMCALL:
				Timer_onTimerCb(intVal);
				break;
			case EVENT_STOP:
				Timer_disarm(s_timerHandle);
				BSP_SetPinPWM(0);
				System_setStatus(INFORM_IDLE);
				break;
			default:
				break;
		}
		EventQueue_Dispose(&event);
	}
	return 0;
}
