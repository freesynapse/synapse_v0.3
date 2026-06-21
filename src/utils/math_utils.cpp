
#include "utils/math_utils.h"

// 
ray_t ray_from_screen(const glm::vec2 &_screen_pos,
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
bool ray_aabb_intersect(const ray_t &_ray,
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
glm::vec2 world_to_screen_ui(const glm::vec3 &_world_pos,
                             const glm::vec2 &_viewport_pos,
                             const glm::vec2 &_viewport_size,
                             const glm::mat4 &_view,
                             const glm::mat4 &_projection)
{
    glm::vec4 clip = _projection * _view * glm::vec4(_world_pos, 1.0f);
    if (clip.w <= 0.0001f) return { -1.0f, -1.0f };

    glm::vec3 ndc = glm::vec3(clip) / clip.w;
    glm::vec2 screen;
    screen.x = _viewport_pos.x + (ndc.x * 0.5f + 0.5f) * _viewport_size.x;
    screen.y = _viewport_pos.y + (1.0f - (ndc.y * 0.5f + 0.5f)) * _viewport_size.y;
    return screen;
}

// 
glm::vec2 world_to_screen_fbo(const glm::vec3 &_world_pos,
                              const glm::vec2 &_viewport_pos,
                              const glm::vec2 &_viewport_size,
                              const glm::mat4 &_view,
                              const glm::mat4 &_projection)
{
    // SYN_DEBUG("viewport_pos passed in: %.2f %.2f\n", _viewport_pos.x, _viewport_pos.y);
    glm::vec4 clip = _projection * _view * glm::vec4(_world_pos, 1.0f);
    if (clip.w <= 0.0001f) return { -1.0f, -1.0f };

    glm::vec3 ndc = glm::vec3(clip) / clip.w;
    glm::vec2 screen;
    screen.x = _viewport_pos.x + (ndc.x * 0.5f + 0.5f) * _viewport_size.x;
    screen.y = _viewport_pos.y + (ndc.y * 0.5f + 0.5f) * _viewport_size.y;  // no flip
    return screen;
}

// 
glm::vec3 hsv_to_rgb(const glm::vec3 &_hsv)
{
    float h = _hsv.x, s = _hsv.y, v = _hsv.z;

    float c = v * s;
    float x = c * (1.0f - glm::abs(glm::mod(h * 6.0f, 2.0f) - 1.0f));
    float m = v - c;
    glm::vec3 rgb;
    if      (h < 1.0f/6.0f) rgb = { c, x, 0 };
    else if (h < 2.0f/6.0f) rgb = { x, c, 0 };
    else if (h < 3.0f/6.0f) rgb = { 0, c, x };
    else if (h < 4.0f/6.0f) rgb = { 0, x, c };
    else if (h < 5.0f/6.0f) rgb = { x, 0, c };
    else                     rgb = { c, 0, x };

    return rgb + m;
}

// 
glm::vec3 rgb_to_hsv(const glm::vec3 &_rgb)
{
    float r = _rgb.r, g = _rgb.g, b = _rgb.b;
    
    float max_c = glm::max(r, glm::max(g, b));
    float min_c = glm::min(r, glm::min(g, b));
    float delta = max_c - min_c;

    float h = 0.0f, s = 0.0f, v = max_c;

    if (delta > 0.0001f) {
        s = delta / max_c;
        if      (max_c == r) h = glm::mod((g - b) / delta, 6.0f) / 6.0f;
        else if (max_c == g) h = ((b - r) / delta + 2.0f)        / 6.0f;
        else                 h = ((r - g) / delta + 4.0f)        / 6.0f;
        if (h < 0.0f) h += 1.0f;
    }

    return { h, s, v };
    
}