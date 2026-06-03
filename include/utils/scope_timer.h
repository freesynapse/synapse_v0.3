#ifndef __SCOPE_TIMER_H
#define __SCOPE_TIMER_H

#include <chrono>

#include "utils/log.h"

//
class scope_timer_t
{
public:
	scope_timer_t(const char *_func="", bool _print_results=false) : // RAII
		m_print_results(_print_results)
	{
		if (m_print_results)
		{
			memset(m_caller, '\0', 128);
			std::sprintf(m_caller, "%s", _func);
		}

		m_start = std::chrono::high_resolution_clock::now();
	}

	~scope_timer_t()
	{
		stop();
	}

	void stop()
	{
		auto end_ = std::chrono::high_resolution_clock::now();
		auto start = std::chrono::time_point_cast<std::chrono::microseconds>(m_start).time_since_epoch().count();
		auto end = std::chrono::time_point_cast<std::chrono::microseconds>(end_).time_since_epoch().count();

		auto duration = end - start;
		double ms = duration * 0.001;

		if (m_print_results) {
			SYN_INFO_NO_FNC("%s: %lf ms.\n", m_caller, ms);
		}
	}

	/* elapsed time in microseconds. */
	long long get_dt()
	{
		auto end_ = std::chrono::high_resolution_clock::now();
		auto start = std::chrono::time_point_cast<std::chrono::microseconds>(m_start).time_since_epoch().count();
		auto end = std::chrono::time_point_cast<std::chrono::microseconds>(end_).time_since_epoch().count();

		auto duration = end - start;

		return duration;
	}

	float get_dt_ms() { return get_dt() * 0.001f; }

private:
	std::chrono::time_point<std::chrono::high_resolution_clock> m_start;
	char m_caller[128] = "";
	bool m_print_results = false;

};


#endif // __SCOPE_TIMER_H
