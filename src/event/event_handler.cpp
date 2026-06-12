
#include "event/event_handler.h"
#include "utils/log.h"

//
event_callback_fnc_t __event_callbacks[MAX_EVENT_TYPES][MAX_CALLBACKS_PER_TYPE];
uint8_t __event_callback_counts[MAX_EVENT_TYPES];

// 
void events_t::init()
{
	m_queue_head = 0;
	m_queue_tail = 0;

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
	assert((m_queue_tail + 1) % MAX_QUEUE_EVENTS != m_queue_head);

	m_queue[m_queue_tail] = _event;
	m_queue_tail = (m_queue_tail + 1) % MAX_QUEUE_EVENTS;
	
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
            }
        }
    }
}

// 
/* TODO: event_tually, different systems may register with the event handler to process events
	* of the event_type_t (event_t.h) corresponding to task of the registered class. Each class that
	* registers for an event type have to provide a function pointer to the function used to
	* process events of a certain event_type_t.
	*/
bool events_t::next_event(event_t &_out_event)
{
	if (m_queue_head == m_queue_tail) {
	    return false;
	}

	_out_event = m_queue[m_queue_head];
	m_queue_head = (m_queue_head + 1) % MAX_QUEUE_EVENTS;	
	return true;
	
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

// 
void events_t::flush_event_type(event_type_t _type)
{
    uint8_t type_idx = (uint8_t)_type;
    uint8_t count = __event_callback_counts[type_idx];

    // temp buffer to hold non-matching events
    event_t temp[MAX_QUEUE_EVENTS];
    uint32_t temp_count = 0;

    // drain the queue
    event_t e;
    while (next_event(e)) {
        if (e.type == _type) {
            // fire callbacks immediately
            for (uint8_t i = 0; i < count; i++) {
                if (__event_callbacks[type_idx][i])
                    __event_callbacks[type_idx][i](e);
            }
        } else {
            // keep for later
            temp[temp_count++] = e;
        }
    }

    // re-enqueue non-matching events
    for (uint32_t i = 0; i < temp_count; i++) {
        dispatch_event(temp[i]);
    }
}

