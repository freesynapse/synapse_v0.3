#ifndef __LOG_H
#define __LOG_H

#include <stdio.h>
#include <string.h>
#include <glm/glm.hpp>

#include "utils/time.h"


// macro for definition of the function signature used in Log::Log.
#ifdef _MSC_VER
#define __func__ __FUNCSIG__
#else
#define __func__ __PRETTY_FUNCTION__
// #define __func__ __FUNCTION__
#endif

//
#define INFO_COLOR      ""
#define DEBUG_COLOR     "\033[32m"
#define WARNING_COLOR   "\x1b[33m"
#define ERROR_COLOR     "\x1b[31m"

//
#define SYN_INFO(msg, ...)        do { zero_log_buffer(); sprintf(__tmp_log_buffer, "[%s] [INFO] %s(): " msg "", get_timestamp(), prettify_fnc_signature(__func__), ##__VA_ARGS__); syn_log(INFO_COLOR); } while (0)
#define SYN_DEBUG(msg, ...)       do { zero_log_buffer(); sprintf(__tmp_log_buffer, "[%s] [DEBUG] %s(): " msg "", get_timestamp(), prettify_fnc_signature(__func__), ##__VA_ARGS__); syn_log(DEBUG_COLOR); } while (0)
#define SYN_WARNING(msg, ...)     do { zero_log_buffer(); sprintf(__tmp_log_buffer, "[%s] [WARNING] %s(): " msg "", get_timestamp(), prettify_fnc_signature(__func__), ##__VA_ARGS__); syn_log(WARNING_COLOR); } while (0)
#define SYN_ERROR(msg, ...)       do { zero_log_buffer(); sprintf(__tmp_log_buffer, "[%s] [ERROR] %s(): " msg "", get_timestamp(), prettify_fnc_signature(__func__), ##__VA_ARGS__); syn_log(ERROR_COLOR); } while (0)
#define SYN_FATAL_ERROR(msg, ...) do { zero_log_buffer(); sprintf(__tmp_log_buffer, "[%s] [ERROR] %s(): " msg "", get_timestamp(), prettify_fnc_signature(__func__), ##__VA_ARGS__); syn_log(ERROR_COLOR); syn_close_log(); exit(1); } while (0)

#define SYN_INFO_NO_FNC(msg, ...)        do { zero_log_buffer(); sprintf(__tmp_log_buffer, "[%s] [INFO]: " msg "", get_timestamp(), ##__VA_ARGS__); syn_log(INFO_COLOR); } while (0)
#define SYN_DEBUG_NO_FNC(msg, ...)       do { zero_log_buffer(); sprintf(__tmp_log_buffer, "[%s] [DEBUG]: " msg "", get_timestamp(), ##__VA_ARGS__); syn_log(DEBUG_COLOR); } while (0)
#define SYN_WARNING_NO_FNC(msg, ...)     do { zero_log_buffer(); sprintf(__tmp_log_buffer, "[%s] [WARNING]: " msg "", get_timestamp(), ##__VA_ARGS__); syn_log(WARNING_COLOR); } while (0)
#define SYN_ERROR_NO_FNC(msg, ...)       do { zero_log_buffer(); sprintf(__tmp_log_buffer, "[%s] [ERROR]: " msg "", get_timestamp(), ##__VA_ARGS__); syn_log(ERROR_COLOR); } while(0)
#define SYN_FATAL_ERROR_NO_FNC(msg, ...) do { zero_log_buffer(); sprintf(__tmp_log_buffer, "[%s] [ERROR]: " msg "", get_timestamp(), ##__VA_ARGS__); syn_log(ERROR_COLOR); syn_close_log(); exit(1); } while(0)

// just a string logged, no sugar added
#define SYN_MSG(msg, ...) do { zero_log_buffer(); sprintf(__tmp_log_buffer, "" msg "", ##__VA_ARGS__); syn_log(INFO_COLOR); } while(0)

//
inline char __tmp_fnc_buffer[128];
char *prettify_fnc_signature(const char *f);

// output log to file
inline char __tmp_log_buffer[512];
extern FILE *__log_fp;
void zero_log_buffer();
int syn_open_log();
void syn_close_log();
void syn_log(const char *_color);

// TODO : test these functions!
//
// Debug matrices and vectors
inline char __tmp_debug_buffer[1024];
void fmt_debug_func(const char *_func);
const char* ff(float _f);
const char* fi(int _i);
void debug_matrix(const char* _func, const char *_mat_name, const glm::mat4& _m4);
void debug_matrix(const char* _func, const char* _mat_name, const glm::mat3& _m3);
void debug_vector(const char* _func, const char* _vec_name, const glm::vec4& _v4);
void debug_vector(const char* _func, const char* _vec_name, const glm::vec3& _v3);
void debug_vector(const char* _func, const char* _vec_name, const glm::vec2& _v2);
void debug_vector(const char* _func, const char* _vec_name, const glm::ivec4& _iv4);
void debug_vector(const char* _func, const char* _vec_name, const glm::ivec3& _iv3);
void debug_vector(const char* _func, const char* _vec_name, const glm::ivec2& _iv2);


#endif // __LOG_H
