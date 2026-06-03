
#include "event/event_handler.h"
#include "utils/log.h"

//
event_callback_fnc_t __event_callbacks[MAX_EVENT_TYPES][MAX_CALLBACKS_PER_TYPE];
uint8_t __event_callback_counts[MAX_EVENT_TYPES];

// 
void events_t::init()
{
	queue_head = 0;
	queue_tail = 0;

	// Clear out the callback arrays explicitly
    for (int i = 0; i < MAX_EVENT_TYPES; ++i) {
        __event_callback_counts[i] = 0;
        for(int j = 0; j < MAX_CALLBACKS_PER_TYPE; ++j) {
            __event_callbacks[i][j] = nullptr;
        }
    }
    
   	SYN_INFO("initialized.\n");
}

// 
void events_t::shutdown()
{
	SYN_INFO("clearing event queue.\n");

}

// 
void events_t::dispatch_event(const event_t &_event)
{
	assert((queue_tail + 1) % MAX_QUEUE_EVENTS != queue_head);

	event_queue[queue_tail] = _event;
	queue_tail = (queue_tail + 1) % MAX_QUEUE_EVENTS;
	
}

// 
/* TODO: event_tually, different systems may register with the event handler to process events
	* of the event_type_t (event_t.h) corresponding to task of the registered class. Each class that
	* registers for an event type have to provide a function pointer to the function used to
	* process events of a certain event_type_t.
	*/
bool events_t::next_event(event_t &_out_event)
{
	if (queue_head == queue_tail) {
	    return false;
	}

	_out_event = event_queue[queue_head];
	queue_head = (queue_head + 1) % MAX_QUEUE_EVENTS;	
	return true;
	
}

// 
void events_t::process_events()
{
    event_t e;
    while (events_t::next_event(e)) {
        uint8_t type_idx = (uint8_t)e.type;
        uint8_t count = __event_callback_counts[type_idx];

        // call all callback functions
        for (uint8_t i = 0; i < count; i++) {
            if (__event_callbacks[type_idx][i]) {
                __event_callbacks[type_idx][i](e);
                if (e.type == event_type_t::WINDOW_TOGGLE_FULLSCREEN) {
                }
            }
        }
    }
}

// 
void events_t::register_callback(event_type_t _event_type, event_callback_fnc_t _handler_fnc)
{
    uint8_t type_idx = (uint8_t)_event_type;
    assert(type_idx < MAX_EVENT_TYPES);

    if (__event_callback_counts[type_idx] < MAX_CALLBACKS_PER_TYPE) {
        __event_callbacks[type_idx][__event_callback_counts[type_idx]] = _handler_fnc;
        __event_callback_counts[type_idx]++;
    }

}

