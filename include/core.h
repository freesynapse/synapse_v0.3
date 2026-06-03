#ifndef __CORE_H
#define __CORE_H

#include <assert.h>
#include <inttypes.h>
#include <memory>
#include <stdlib.h>


// UTILITY //
#define RETURN_SUCCESS 	 0
#define RETURN_FAILURE 	-1


// 'SMART' POINTERS
template<typename T>
using Ref = std::shared_ptr<T>;
template<typename T>
inline Ref<T> MakeRef() { return std::make_shared<T>(); }
template<typename T, typename ...Args>
inline Ref<T> MakeRef(Args ...args) { return std::make_shared<T>(args...); }

// RENDERER //
//

// color packing/unpacking macros
#define RGBA8i(r, g, b, a) (r << 24 | g << 16 | b << 8 | a)
#define RGBA8f(r, g, b, a) ((uint32_t)(r * 255) << 24 | (uint32_t)(g * 255) << 16 | (uint32_t)(b * 255) << 8 | (uint32_t)(a * 255))
#define RGBA8_EXTRACT_FLOAT(value, offset) (((value >> offset) & 0xff) / 255.0f)

// TODO : test this!
#define SYN_ASSERT(condition, msg) do { if (!(condition)) { SYN_ERROR("Assertion failed: " msg); exit(1); } } while(0)


#endif // __CORE_H