#ifndef __RANDOM_H
#define __RANDOM_H

#include <random>
#include <time.h>
#include <string.h>
#include <assert.h>

#include <glm/glm.hpp>


class random_t
{
public:
    void               init();

    int                rand_i();
    int                rand_i(int max);
    int                rand_i_r(int _lo, int _hi);

    uint32_t           rand_ui();
    uint32_t           rand_ui_r(uint32_t _lo=0, uint32_t _hi=std::numeric_limits<uint32_t>::max());

    float              rand_f();
    float              rand_fC();
    float              rand_f_r(float _lo=0.0f, float _hi=std::numeric_limits<float>::max());
    float              rand_fC_r(float _lo=0.0f, float _hi=(float)RAND_MAX);

    glm::vec2          rand2_f();
    glm::vec2          rand2_fC();
    glm::vec2          rand2_f_r(float _lo=0.0f, float _hi=std::numeric_limits<float>::max());
    glm::vec2          rand2_fC_r(float _lo=0.0f, float _hi=(float)RAND_MAX);

    glm::vec3          rand3_f();
    glm::vec3          rand3_fC();
    glm::vec3          rand3_f_r(float _lo=0.0f, float _hi=std::numeric_limits<float>::max());
    glm::vec3          rand3_fC_r(float _lo=0.0f, float _hi=(float)RAND_MAX);

    glm::vec4          rand4_f();
    glm::vec4          rand4_fC();
    glm::vec4          rand4_f_r(float _lo=0.0f, float _hi=std::numeric_limits<float>::max());
    glm::vec4          rand4_fC_r(float _lo=0.0f, float _hi=(float)RAND_MAX);

    std::vector<float> rand_normal(size_t _n, float _mean=0.0f, float _sd=1.0f);
    
    bool               rand_b();
    char*              rand_str(size_t len);

private:
    std::mt19937 m_random_engine;
    std::uniform_int_distribution<std::mt19937::result_type> m_int_distribution;
    std::uniform_real_distribution<> m_real_distribution;
    float m_inv_max_float;
    float m_inv_max_floatC;
    const char* m_alphanum;
    size_t m_alphanum_size;
    char tmp_buffer[256];
};


#endif // __RANDOM_H
