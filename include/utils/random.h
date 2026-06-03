#ifndef __RANDOM_H
#define __RANDOM_H

#include <random>
#include <time.h>
#include <string.h>
#include <assert.h>

#include <glm/glm.hpp>


class Random
{
public:
    static void init()
    {
        // 'true' random -- physical based noise using std::random_device().
        s_randomEngine.seed(std::random_device()());
        srand(time(NULL));
    }
    
    //
    static int rand_i() { return rand(); }
    
    //
    static int rand_i(int max)
    { 
        if (max < 0)
            return -(rand() % (abs(max) + 1));
        return rand() % (max + 1); 
    }
    
    //
    static int rand_i_r(int _lo, int _hi) { return rand() % (_hi - _lo) + _lo; }
    
    //
    static uint32_t rand_ui() { return s_intDistribution(s_randomEngine); }
    
    //
    static uint32_t rand_ui_r(uint32_t _lo=0, uint32_t _hi=std::numeric_limits<uint32_t>::max())
    { return s_intDistribution(s_randomEngine) % (_hi - _lo) + _lo; }
    
    //
    static float rand_f() { return s_realDistribution(s_randomEngine) * s_invMaxFloat; }
    
    //
    static float rand_fC() { return (float)rand() * s_invMaxFloatC; }
    
    //
    static glm::vec2 rand2_f()
    { 
        return glm::vec2(s_realDistribution(s_randomEngine) * s_invMaxFloat,
                            s_realDistribution(s_randomEngine) * s_invMaxFloat);
    }
    
    //
    static glm::vec2 rand2_fC()
    {
        return glm::vec2((float)rand() * s_invMaxFloatC,
                            (float)rand() * s_invMaxFloatC);
    }
    
    //
    static glm::vec3 rand3_f()
    { 
        return glm::vec3(s_realDistribution(s_randomEngine) * s_invMaxFloat,
                            s_realDistribution(s_randomEngine) * s_invMaxFloat,
                            s_realDistribution(s_randomEngine) * s_invMaxFloat);
    }
    
    //
    static glm::vec3 rand3_fC()
    {
        return glm::vec3((float)rand() * s_invMaxFloatC,
                            (float)rand() * s_invMaxFloatC,
                            (float)rand() * s_invMaxFloatC);
    }
    
    //
    static glm::vec4 rand4_f()
    { 
        return glm::vec4(s_realDistribution(s_randomEngine) * s_invMaxFloat,
                            s_realDistribution(s_randomEngine) * s_invMaxFloat,
                            s_realDistribution(s_randomEngine) * s_invMaxFloat,
                            s_realDistribution(s_randomEngine) * s_invMaxFloat);
    }

    //
    static glm::vec4 rand4_fC()
    {
        return glm::vec4((float)rand() * s_invMaxFloatC,
                            (float)rand() * s_invMaxFloatC,
                            (float)rand() * s_invMaxFloatC,
                            (float)rand() * s_invMaxFloatC);
    }

    //
    static float rand_f_r(float _lo=0.0f, float _hi=std::numeric_limits<float>::max())
    {
        //float r = s_realDistribution(s_randomEngine);
        //float div = std::numeric_limits<float>::max() / (_hi - _lo);
        //r = r / div + _lo;
        return s_realDistribution(s_randomEngine) / (std::numeric_limits<float>::max() / (_hi-_lo)) + _lo;
    }

    //
    static float rand_fC_r(float _lo=0.0f, float _hi=(float)RAND_MAX)
    {
        return (float)rand() * s_invMaxFloatC / (_hi-_lo) + _lo;
    }

    //
    static glm::vec2 rand2_f_r(float _lo=0.0f, float _hi=std::numeric_limits<float>::max())
    {
        return glm::vec2(s_realDistribution(s_randomEngine) / (std::numeric_limits<float>::max() / (_hi-_lo)) + _lo,
                            s_realDistribution(s_randomEngine) / (std::numeric_limits<float>::max() / (_hi-_lo)) + _lo);
    }
    
    //
    static glm::vec2 rand2_fC_r(float _lo=0.0f, float _hi=(float)RAND_MAX)
    {
        return glm::vec2((float)rand() * s_invMaxFloatC / (_hi-_lo) + _lo,
                            (float)rand() * s_invMaxFloatC / (_hi-_lo) + _lo);
    }

    //
    static glm::vec3 rand3_f_r(float _lo=0.0f, float _hi=std::numeric_limits<float>::max())
    {
        return glm::vec3(s_realDistribution(s_randomEngine) / (std::numeric_limits<float>::max() / (_hi-_lo)) + _lo,
                            s_realDistribution(s_randomEngine) / (std::numeric_limits<float>::max() / (_hi-_lo)) + _lo,
                            s_realDistribution(s_randomEngine) / (std::numeric_limits<float>::max() / (_hi-_lo)) + _lo);
    }
    
    //
    static glm::vec3 rand3_fC_r(float _lo=0.0f, float _hi=(float)RAND_MAX)
    {
        return glm::vec3((float)rand() * s_invMaxFloatC / (_hi-_lo) + _lo,
                            (float)rand() * s_invMaxFloatC / (_hi-_lo) + _lo,
                            (float)rand() * s_invMaxFloatC / (_hi-_lo) + _lo);
    }

    //
    static glm::vec4 rand4_f_r(float _lo=0.0f, float _hi=std::numeric_limits<float>::max())
    {
        return glm::vec4(s_realDistribution(s_randomEngine) / (std::numeric_limits<float>::max() / (_hi-_lo)) + _lo,
                            s_realDistribution(s_randomEngine) / (std::numeric_limits<float>::max() / (_hi-_lo)) + _lo,
                            s_realDistribution(s_randomEngine) / (std::numeric_limits<float>::max() / (_hi-_lo)) + _lo,
                            s_realDistribution(s_randomEngine) / (std::numeric_limits<float>::max() / (_hi-_lo)) + _lo);
    }

    //
    static glm::vec4 rand4_fC_r(float _lo=0.0f, float _hi=(float)RAND_MAX)
    {
        return glm::vec4((float)rand() * s_invMaxFloatC / (_hi-_lo) + _lo,
                            (float)rand() * s_invMaxFloatC / (_hi-_lo) + _lo,
                            (float)rand() * s_invMaxFloatC / (_hi-_lo) + _lo,
                            (float)rand() * s_invMaxFloatC / (_hi-_lo) + _lo);
    }

    //
    static bool rand_b() { return (rand() % 2 == 0); }

    //
	static char* rand_str(size_t len)
	{
		static char buffer[256];
		memset(buffer, 0, 256);
		assert(len < 256);
		for (uint32_t i = 0; i < len; i++)
			buffer[i] = s_alphanum[rand() % s_alphanumSize];
		return buffer;
	}

private:
    static std::mt19937 s_randomEngine;
    static std::uniform_int_distribution<std::mt19937::result_type> s_intDistribution;
    static std::uniform_real_distribution<> s_realDistribution;
    static float s_invMaxFloat;
    static float s_invMaxFloatC;
    static const char* s_alphanum;
    static size_t s_alphanumSize;
};


#endif // __RANDOM_H
