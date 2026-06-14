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
inline ray_t ray_from_screen(const glm::vec2 &_screen_pos,
                             const glm::vec2 &_viewport_pos,
                             const glm::vec2 &_viewport_size,
                             const glm::mat4 &_view_matrix,
                             const glm::mat4 &_proj_matrix)
{
    glm::vec2 ndc = {
        ((_screen_pos.x - _viewport_pos.x) / _viewport_size.x) * 2.0f - 1.0f,
        1.0f - ((_screen_pos.y - _viewport_pos.y) / _viewport_size.y) * 2.0f
    };

    glm::mat4 inv_vp = glm::inverse(_proj_matrix * _view_matrix);
    glm::vec4 near_p = inv_vp * glm::vec4(ndc.x, ndc.y, -1.0f, 1.0f);
    glm::vec4 far_p  = inv_vp * glm::vec4(ndc.x, ndc.y, 1.0f, 1.0f);
    near_p /= near_p.w;
    far_p  /= far_p.w;

    ray_t ray;
    ray.origin    = glm::vec3(near_p);
    ray.direction = glm::normalize(glm::vec3(far_p) - ray.origin);
    return ray;
    
}

// 
inline bool ray_aabb_intersect(const ray_t &_ray,
                               const glm::vec3 &_min,
                               const glm::vec3 &_max,
                               float &_t_out)
{
    glm::vec3 inv_dir = 1.0f / _ray.direction;
    glm::vec3 t0 = (_min - _ray.origin) * inv_dir;
    glm::vec3 t1 = (_max - _ray.origin) * inv_dir;

    glm::vec3 tmin = glm::min(t0, t1);
    glm::vec3 tmax = glm::max(t0, t1);

    float t_enter = glm::max(glm::max(tmin.x, tmin.y), tmin.z);
    float t_exit  = glm::min(glm::min(tmax.x, tmax.y), tmax.z);

    if (t_exit < 0.0f || t_enter > t_exit) return false;
    _t_out = t_enter;
    return true;
    
}

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
