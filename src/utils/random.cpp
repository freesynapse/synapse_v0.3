#include "utils/random.h"


static const char* k_alphanum =
    "0123456789"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz";

void random_t::init()
{
    m_random_engine.seed(std::random_device()());
    srand(time(NULL));

    m_inv_max_float  = 1.0f / (float)std::numeric_limits<std::mt19937::result_type>::max();
    m_inv_max_floatC = 1.0f / (float)RAND_MAX;
    m_alphanum     = k_alphanum;
    m_alphanum_size = strlen(m_alphanum);
}


// -- int --

int random_t::rand_i() { return rand(); }

int random_t::rand_i(int max)
{
    if (max < 0)
        return -(rand() % (abs(max) + 1));
    return rand() % (max + 1);
}

int random_t::rand_i_r(int _lo, int _hi) { return rand() % (_hi - _lo) + _lo; }


// -- uint32 --

uint32_t random_t::rand_ui() { return m_int_distribution(m_random_engine); }

uint32_t random_t::rand_ui_r(uint32_t _lo, uint32_t _hi)
{ return m_int_distribution(m_random_engine) % (_hi - _lo) + _lo; }


// -- float --

float random_t::rand_f()  { return m_real_distribution(m_random_engine) * m_inv_max_float; }
float random_t::rand_fC() { return (float)rand() * m_inv_max_floatC; }

float random_t::rand_f_r(float _lo, float _hi)
{ return m_real_distribution(m_random_engine) / (std::numeric_limits<float>::max() / (_hi - _lo)) + _lo; }

float random_t::rand_fC_r(float _lo, float _hi)
{ return (float)rand() * m_inv_max_floatC / (_hi - _lo) + _lo; }


// -- vec2 --

glm::vec2 random_t::rand2_f()
{
    return glm::vec2(m_real_distribution(m_random_engine) * m_inv_max_float,
                     m_real_distribution(m_random_engine) * m_inv_max_float);
}

glm::vec2 random_t::rand2_fC()
{
    return glm::vec2((float)rand() * m_inv_max_floatC,
                     (float)rand() * m_inv_max_floatC);
}

glm::vec2 random_t::rand2_f_r(float _lo, float _hi)
{
    float scale = 1.0f / (std::numeric_limits<float>::max() / (_hi - _lo));
    return glm::vec2(m_real_distribution(m_random_engine) * scale + _lo,
                     m_real_distribution(m_random_engine) * scale + _lo);
}

glm::vec2 random_t::rand2_fC_r(float _lo, float _hi)
{
    return glm::vec2((float)rand() * m_inv_max_floatC / (_hi - _lo) + _lo,
                     (float)rand() * m_inv_max_floatC / (_hi - _lo) + _lo);
}


// -- vec3 --

glm::vec3 random_t::rand3_f()
{
    return glm::vec3(m_real_distribution(m_random_engine) * m_inv_max_float,
                     m_real_distribution(m_random_engine) * m_inv_max_float,
                     m_real_distribution(m_random_engine) * m_inv_max_float);
}

glm::vec3 random_t::rand3_fC()
{
    return glm::vec3((float)rand() * m_inv_max_floatC,
                     (float)rand() * m_inv_max_floatC,
                     (float)rand() * m_inv_max_floatC);
}

glm::vec3 random_t::rand3_f_r(float _lo, float _hi)
{
    float scale = 1.0f / (std::numeric_limits<float>::max() / (_hi - _lo));
    return glm::vec3(m_real_distribution(m_random_engine) * scale + _lo,
                     m_real_distribution(m_random_engine) * scale + _lo,
                     m_real_distribution(m_random_engine) * scale + _lo);
}

glm::vec3 random_t::rand3_fC_r(float _lo, float _hi)
{
    return glm::vec3((float)rand() * m_inv_max_floatC / (_hi - _lo) + _lo,
                     (float)rand() * m_inv_max_floatC / (_hi - _lo) + _lo,
                     (float)rand() * m_inv_max_floatC / (_hi - _lo) + _lo);
}


// -- vec4 --

glm::vec4 random_t::rand4_f()
{
    return glm::vec4(m_real_distribution(m_random_engine) * m_inv_max_float,
                     m_real_distribution(m_random_engine) * m_inv_max_float,
                     m_real_distribution(m_random_engine) * m_inv_max_float,
                     m_real_distribution(m_random_engine) * m_inv_max_float);
}

glm::vec4 random_t::rand4_fC()
{
    return glm::vec4((float)rand() * m_inv_max_floatC,
                     (float)rand() * m_inv_max_floatC,
                     (float)rand() * m_inv_max_floatC,
                     (float)rand() * m_inv_max_floatC);
}

glm::vec4 random_t::rand4_f_r(float _lo, float _hi)
{
    float scale = 1.0f / (std::numeric_limits<float>::max() / (_hi - _lo));
    return glm::vec4(m_real_distribution(m_random_engine) * scale + _lo,
                     m_real_distribution(m_random_engine) * scale + _lo,
                     m_real_distribution(m_random_engine) * scale + _lo,
                     m_real_distribution(m_random_engine) * scale + _lo);
}

glm::vec4 random_t::rand4_fC_r(float _lo, float _hi)
{
    return glm::vec4((float)rand() * m_inv_max_floatC / (_hi - _lo) + _lo,
                     (float)rand() * m_inv_max_floatC / (_hi - _lo) + _lo,
                     (float)rand() * m_inv_max_floatC / (_hi - _lo) + _lo,
                     (float)rand() * m_inv_max_floatC / (_hi - _lo) + _lo);
}


// -- misc --

bool random_t::rand_b() { return (rand() % 2 == 0); }

char* random_t::rand_str(size_t len)
{
    memset(tmp_buffer, 0, 256);
    assert(len < 256);
    for (uint32_t i = 0; i < len; i++)
        tmp_buffer[i] = m_alphanum[rand() % m_alphanum_size];
    return tmp_buffer;
}

