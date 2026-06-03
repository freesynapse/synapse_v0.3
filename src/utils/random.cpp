
#include <string.h>

#include "utils/random.h"

std::mt19937 Random::s_randomEngine;
std::uniform_int_distribution<std::mt19937::result_type> Random::s_intDistribution;
std::uniform_real_distribution<> Random::s_realDistribution = std::uniform_real_distribution<>(0, std::numeric_limits<float>::max());
float Random::s_invMaxFloat = 1.0f / std::numeric_limits<float>::max();
float Random::s_invMaxFloatC = 1.0f / (float)RAND_MAX;
const char* Random::s_alphanum = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
size_t Random::s_alphanumSize = strlen(s_alphanum);

