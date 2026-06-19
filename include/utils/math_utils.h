#ifndef __MATH_UTILS_H
#define __MATH_UTILS_H


#include <glm/glm.hpp>

#ifndef M_PI
	#define M_PI 3.14159265358979f
#endif

#define _1_OVER_PI		0.31830988618379f
#define _1_OVER_180		0.00555555555555f
#define PI_OVER_180		M_PI * _1_OVER_180
#define _180_OVER_PI	180.0f * _1_OVER_PI
#define BIT_SHIFT(x) (1 << (x))

// 
struct ray_t {
    glm::vec3 origin;
    glm::vec3 direction;
};

// 
ray_t ray_from_screen(const glm::vec2 &_screen_pos,
                      const glm::vec2 &_viewport_pos,
                      const glm::vec2 &_viewport_size,
                      const glm::mat4 &_view_matrix,
                      const glm::mat4 &_proj_matrix);
// 
bool ray_aabb_intersect(const ray_t &_ray,
                        const glm::vec3 &_min,
                        const glm::vec3 &_max,
                        float &_t_out);

// 
glm::vec2 world_to_screen_ui(const glm::vec3 &_world_pos,
                             const glm::vec2 &_viewport_pos,
                             const glm::vec2 &_viewport_size,
                             const glm::mat4 &_view,
                             const glm::mat4 &_projection);

glm::vec2 world_to_screen_fbo(const glm::vec3 &_world_pos,
                              const glm::vec2 &_viewport_pos,
                              const glm::vec2 &_viewport_size,
                              const glm::mat4 &_view,
                              const glm::mat4 &_projection);


// 
template<class T>
inline const T& min(const T& _a, const T& _b) { return _a < _b ? _a : _b; }
template<class T>
inline const T& max(const T& _a, const T& _b) { return _a > _b ? _a : _b; }
template<class T>
inline const T& clamp(const T& _x, const T& _lo, const T& _hi) { return min(max(_x, _lo), _hi); }

inline float deg_to_rad(float _theta_deg) { return _theta_deg * PI_OVER_180;  }
inline float rad_to_deg(float _theta_rad) { return _theta_rad * _180_OVER_PI; }

// Linearly maps a range [_in0 ... _in1] to another range [_out0 ... _out1].
// _x is expected to be in range [_in0 ... _in1].
inline float lmap(float _x, float _in0, float _in1, float _out0, float _out1)
{
    float slope = (_out1 - _out0) / (_in1 - _in0);
    return _out0 + slope * (_x - _in0);
}

inline glm::vec2 lmap(const glm::vec2& _v, const glm::vec2& _in0, const glm::vec2& _in1, 
                        const glm::vec2& _out0, const glm::vec2& _out1)
{
    float slope_x = (_out1.x - _out0.x) / (_in1.x - _in0.x);
    float slope_y = (_out1.y - _out0.y) / (_in1.y - _in0.y);
    float x = _out0.x + slope_x * (_v.x - _in0.x);
    float y = _out0.y + slope_y * (_v.y - _in0.y);
    return glm::vec2(x, y);
}

inline glm::vec3 lmap(const glm::vec3& _v, const glm::vec3& _in0, const glm::vec3& _in1, 
                        const glm::vec3& _out0, const glm::vec3& _out1)
{
    float slope_x = (_out1.x - _out0.x) / (_in1.x - _in0.x);
    float slope_y = (_out1.y - _out0.y) / (_in1.y - _in0.y);
    float slope_z = (_out1.z - _out0.z) / (_in1.z - _in0.z);
    float x = _out0.x + slope_x * (_v.x - _in0.x);
    float y = _out0.y + slope_y * (_v.y - _in0.y);
    float z = _out0.z + slope_z * (_v.z - _in0.z);
    return glm::vec3(x, y, z);
}


#endif // __MATH_UTILS_H
