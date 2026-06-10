#ifndef __TIME_H
#define __TIME_H

#include <string>

//
inline char __tmp_time_buffer[16];
char *get_timestamp();
// static 
std::string current_time();
std::string current_date();


#endif // __TIME_H
