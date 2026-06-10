
#include <string.h>

#include "utils/log.h"

// globals
FILE *__log_fp = NULL;
log_ring_buffer_t syn_log_buffer;

// 
char *prettify_fnc_signature(const char *f)
{
    memset(__tmp_fnc_buffer, '\0', 128);
    size_t i = 0;

    // chop type specifier
    while (i < strlen(f)) {
        if (f[i] == ' ')
            break;
        i++;
    }
    i++;

    size_t len = strlen(f);
    size_t j = 0;
    
    while (i < len) {
        if (f[i] == '(') {
            break;
        } else {
            __tmp_fnc_buffer[j++] = f[i];
        }

        i++;
    }

    return __tmp_fnc_buffer;
}

//
void zero_log_buffer() 
{
    memset(__tmp_log_buffer, 0, 512); 
}

// 
int syn_open_log()
{
    const char *file = "log.txt";
    __log_fp = fopen(file, "w");
    if (__log_fp == NULL) {
        SYN_WARNING("cannot open file '%s'.\n", file);
        return -1;
    }
    SYN_INFO("opened log file '%s'.\n", file);
    return 0;
}

// 
void syn_close_log()
{
    if (__log_fp) {
        SYN_INFO("closing log.\n");
        fclose(__log_fp);
        __log_fp = NULL;
    }
    
}

// 
void syn_log(const char *_color)
{
    fprintf(stdout, "%s%s\033[0m", _color, __tmp_log_buffer);
    if (__log_fp != NULL)
        fprintf(__log_fp, "%s", __tmp_log_buffer);

    log_level_t level = log_level_t::INFO;
    if (strcmp(_color, DEBUG_COLOR) == 0)           level = log_level_t::DEBUG;
    else if (strcmp(_color, WARNING_COLOR) == 0)    level = log_level_t::WARNING;
    else if (strcmp(_color, ERROR_COLOR) == 0)      level = log_level_t::ERROR;

    syn_log_buffer.push(__tmp_log_buffer, level);
    
}

// 
void fmt_debug_func(const char *_func)
{
	memset(__tmp_fnc_buffer, 0, 128);
	sprintf(__tmp_fnc_buffer, "%s", _func);
}

//
const char* ff(float _f) { return (_f < 0.0f ? "-" : " "); }
const char* fi(int _i) { return (_i < 0 ? "-" : " "); }

//
void debug_matrix(const char* _func, const char* _mat_name, const glm::mat4& _m4)
{
	fmt_debug_func(_func);
	memset(__tmp_debug_buffer, 0, 1024);
	int c = 0;
	c += std::sprintf(__tmp_debug_buffer, "%s\n", _mat_name);
	c += std::sprintf(__tmp_debug_buffer + c, "|  %s%.2f  %s%.2f  %s%.2f  %s%.2f  |\n", ff(_m4[0][0]), fabs(_m4[0][0]), ff(_m4[0][1]), fabs(_m4[0][1]), ff(_m4[0][2]), fabs(_m4[0][2]), ff(_m4[0][3]), fabs(_m4[0][3]));
	c += std::sprintf(__tmp_debug_buffer + c, "|  %s%.2f  %s%.2f  %s%.2f  %s%.2f  |\n", ff(_m4[1][0]), fabs(_m4[1][0]), ff(_m4[1][1]), fabs(_m4[1][1]), ff(_m4[1][2]), fabs(_m4[1][2]), ff(_m4[1][3]), fabs(_m4[1][3]));
	c += std::sprintf(__tmp_debug_buffer + c, "|  %s%.2f  %s%.2f  %s%.2f  %s%.2f  |\n", ff(_m4[2][0]), fabs(_m4[2][0]), ff(_m4[2][1]), fabs(_m4[2][1]), ff(_m4[2][2]), fabs(_m4[2][2]), ff(_m4[2][3]), fabs(_m4[2][3]));
	std::sprintf(__tmp_debug_buffer + c, "|  %s%.2f  %s%.2f  %s%.2f  %s%.2f  |", ff(_m4[3][0]), fabs(_m4[3][0]), ff(_m4[3][1]), fabs(_m4[3][1]), ff(_m4[3][2]), fabs(_m4[3][2]), ff(_m4[3][3]), fabs(_m4[3][3]));

	SYN_DEBUG_NO_FNC("%s: %s\n", __tmp_fnc_buffer, __tmp_debug_buffer);

}

// 
void debug_matrix(const char* _func, const char* _mat_name, const glm::mat3& _m3)
{
	fmt_debug_func(_func);
	memset(__tmp_debug_buffer, 0, 1024);
	int c = 0;

	c += std::sprintf(__tmp_debug_buffer, "%s\n", _mat_name);
	c += std::sprintf(__tmp_debug_buffer + c, "|  %s%.2f  %s%.2f  %s%.2f  |\n", ff(_m3[0][0]), fabs(_m3[0][0]), ff(_m3[0][1]), fabs(_m3[0][1]), ff(_m3[0][2]), fabs(_m3[0][2]));
	c += std::sprintf(__tmp_debug_buffer + c, "|  %s%.2f  %s%.2f  %s%.2f  |\n", ff(_m3[1][0]), fabs(_m3[1][0]), ff(_m3[1][1]), fabs(_m3[1][1]), ff(_m3[1][2]), fabs(_m3[1][2]));
	std::sprintf(__tmp_debug_buffer + c, "|  %s%.2f  %s%.2f  %s%.2f  |", ff(_m3[2][0]), fabs(_m3[2][0]), ff(_m3[2][1]), fabs(_m3[2][1]), ff(_m3[2][2]), fabs(_m3[2][2]));
	SYN_DEBUG_NO_FNC("%s: %s\n", __tmp_fnc_buffer, __tmp_debug_buffer);

}

// 
void debug_vector(const char* _func, const char* _vec_name, const glm::vec4& _v4)
{
	fmt_debug_func(_func);
	memset(__tmp_debug_buffer, 0, 1024);
	int c = 0;

	c += std::sprintf(__tmp_debug_buffer, "%s  ", _vec_name);
	std::sprintf(__tmp_debug_buffer + c, "|  %s%.2f  %s%.2f  %s%.2f  %s%.2f  |", ff(_v4.x), fabs(_v4.x), ff(_v4.y), fabs(_v4.y), ff(_v4.z), fabs(_v4.z), ff(_v4.w), fabs(_v4.w));
	SYN_DEBUG_NO_FNC("%s: %s\n", __tmp_fnc_buffer, __tmp_debug_buffer);

}

// 
void debug_vector(const char* _func, const char* _vec_name, const glm::vec3& _v3)
{
	fmt_debug_func(_func);
	memset(__tmp_debug_buffer, 0, 1024);
	int c = 0;

	c += std::sprintf(__tmp_debug_buffer, "%s  ", _vec_name);
	std::sprintf(__tmp_debug_buffer + c, "|  %s%.2f  %s%.2f  %s%.2f  |", ff(_v3.x), fabs(_v3.x), ff(_v3.y), fabs(_v3.y), ff(_v3.z), fabs(_v3.z));
	SYN_DEBUG_NO_FNC("%s: %s\n", __tmp_fnc_buffer, __tmp_debug_buffer);

}

// 
void debug_vector(const char* _func, const char* _vec_name, const glm::vec2& _v2)
{
	fmt_debug_func(_func);
	memset(__tmp_debug_buffer, 0, 1024);
	int c = 0;

	c += std::sprintf(__tmp_debug_buffer, "%s  ", _vec_name);
	std::sprintf(__tmp_debug_buffer + c, "|  %s%.2f  %s%.2f  |", ff(_v2.x), fabs(_v2.x), ff(_v2.y), fabs(_v2.y));
	SYN_DEBUG_NO_FNC("%s: %s\n", __tmp_fnc_buffer, __tmp_debug_buffer);

}
// 
void debug_vector(const char* _func, const char* _vec_name, const glm::ivec4& _iv4)
{
	fmt_debug_func(_func);
	memset(__tmp_debug_buffer, 0, 1024);
	int c = 0;

	c += std::sprintf(__tmp_debug_buffer, "%s  ", _vec_name);
	std::sprintf(__tmp_debug_buffer + c, "|  %s%d  %s%d  %s%d  %s%d  |", fi(_iv4.x), abs(_iv4.x), fi(_iv4.y), abs(_iv4.y), fi(_iv4.z), abs(_iv4.z), fi(_iv4.w), abs(_iv4.w));
	SYN_DEBUG_NO_FNC("%s: %s\n", __tmp_fnc_buffer, __tmp_debug_buffer);

}

// 
void debug_vector(const char* _func, const char* _vec_name, const glm::ivec3& _iv3)
{
	fmt_debug_func(_func);
	memset(__tmp_debug_buffer, 0, 1024);
	int c = 0;

	c += std::sprintf(__tmp_debug_buffer, "%s  ", _vec_name);
	std::sprintf(__tmp_debug_buffer + c, "|  %s%d  %s%d  %s%d  |", fi(_iv3.x), abs(_iv3.x), fi(_iv3.y), abs(_iv3.y), fi(_iv3.z), abs(_iv3.z));
	SYN_DEBUG_NO_FNC("%s: %s\n", __tmp_fnc_buffer, __tmp_debug_buffer);

}

// 
void debug_vector(const char* _func, const char* _vec_name, const glm::ivec2& _iv2)
{
	fmt_debug_func(_func);
	memset(__tmp_debug_buffer, 0, 1024);
	int c = 0;

	c += std::sprintf(__tmp_debug_buffer, "%s  ", _vec_name);
	std::sprintf(__tmp_debug_buffer + c, "|  %s%d  %s%d  |", fi(_iv2.x), abs(_iv2.x), fi(_iv2.y), abs(_iv2.y));
	SYN_DEBUG_NO_FNC("%s: %s\n", __tmp_fnc_buffer, __tmp_debug_buffer);
}

