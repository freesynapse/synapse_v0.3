
#include <iomanip>
#include <string.h>
#include <chrono>

#include "utils/time.h"

// 
std::string current_time()
{
    std::chrono::time_point<std::chrono::system_clock> chronoTime = std::chrono::system_clock::now();
	std::time_t now_t = std::chrono::system_clock::to_time_t(chronoTime);

    std::tm tm_now = *std::localtime(&now_t);

	// extract the time in HH:MM:SS and return.
	std::string time = "";
	std::stringstream ss;
	ss << std::put_time(&tm_now, "%H:%M:%S");

	return ss.str();
}

//
std::string current_date()
{
    std::chrono::time_point<std::chrono::system_clock> chronoTime = std::chrono::system_clock::now();
	std::time_t now_t = std::chrono::system_clock::to_time_t(chronoTime);

    std::tm tm_now = *std::localtime(&now_t);

	// extract the time in YYYY:MM:DD and return.
	std::string time = "";
	std::stringstream ss;
	ss << std::put_time(&tm_now, "%Y-%m-%d");

	return ss.str();
}

//
char *get_timestamp()
{
    memset(__tmp_time_buffer, 0, 16);
    time_t t;
    struct tm *info;
    time(&t);
    info = localtime(&t);
    strftime(__tmp_time_buffer, sizeof(__tmp_time_buffer), "%H:%M:%S", info);
    return __tmp_time_buffer;
}
