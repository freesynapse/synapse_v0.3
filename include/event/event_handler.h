#ifndef __EVENT_HANDLER_H
#define __EVENT_HANDLER_H

#include "event/event.h"

//
#define MAX_QUEUE_EVENTS        256
#define MAX_EVENT_TYPES          32
#define MAX_CALLBACKS_PER_TYPE  128

// function pointer for event callback functions
typedef void (*event_callback_fnc_t)(const event_t &);


// 
struct events_t
{
	//
	void init();

	/* Release all event pointers. This is the job of the event_tHandler, even after dispatch to
	registered handlers. */
	void shutdown();

	/* Put a new event on the queue. */
	void dispatch_event(const event_t &_event);

	/* Process all events in the queue. */
	void process_events();

	// Registration of function pointers that will be alerted by the event handler
	// when an event of a certain event_type_t is dispatched.
	void register_callback(event_type_t _event_type, event_callback_fnc_t _handler_fnc);

	// DEBUG
	int queue_length();

	/* Get next event from the queue. */
	bool next_event(event_t &_out_event);

	
	// member variables
	uint32_t queue_head;
	uint32_t queue_tail;
	event_t event_queue[MAX_QUEUE_EVENTS];

};

extern event_callback_fnc_t __event_callbacks[MAX_EVENT_TYPES][MAX_CALLBACKS_PER_TYPE];
extern uint8_t __event_callback_counts[MAX_EVENT_TYPES];
/*
	A multimap where the registered handlers are paired to a specific
	event_type_t (eg. ::TUTORIAL etc.). Upon event handling, process_events()
	will dispatch the event to the respective handlers for that event_type_t.
*/
//extern std::multimap<event_type_t, std::function<void(event_t *)> > event_handler_fncs;
//typedef std::multimap<event_type_t, std::function<void(event_t *)> >::iterator mapIterator;



#endif // __EVENT_HANDLER_H
