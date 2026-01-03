#ifndef Utils
#define Utils

static const int MAX_OBJ = 100;
static const uint MAX_UINT = 4294967295;
static const float MAX_FLOAT = 3.402823466e+38;
static const float PI = 3.141592;
static const float EPSILON = 0.001;

uint PCG_Hash(uint hash)
{
    uint state = hash * 747796405u + 2891336453u;
    uint word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
    return (word >> 22u) ^ word;
}

float RandomFloat(inout uint seed)
{
    seed = PCG_Hash(seed);
    return float(seed) / MAX_UINT;
}

float RandomRange(inout uint seed, float min, float max)
{
    return lerp(min, max, RandomFloat(seed));
}

float3 RandomUnitVec(inout uint seed)
{
    return normalize(float3(
        RandomFloat(seed) * 2.0f - 1.0f,
        RandomFloat(seed) * 2.0f - 1.0f,
        RandomFloat(seed) * 2.0f - 1.0f));
}


float3 RandomVec3OnUnitHemiSphere(inout uint seed, float3 normal)
{
    float3 randomvec = RandomUnitVec(seed);

    if (dot(randomvec, normal) > 0.0f)
    {
        return randomvec;
    }
    else
    {
        return -randomvec;
    }
}

float3 RandomVec3(inout uint seed, float min, float max)
{
    return float3(
        RandomRange(seed, min, max),
        RandomRange(seed, min, max),
        RandomRange(seed, min, max)
    );
}


float3 FresnelSchlick(float cosThetha, float3 F0)
{
    return F0 + (1.0f - F0) * pow(max(1.0f - cosThetha, 0.0f), 5.0f);
}

#endif