#include "shaderCommons.hs"

const int HASH_SIZE = 0xffff;
const int MaxNumberOfParticles = 0xff;

const ivec2 CellOffsets[9] = {
    ivec2(-1, -1),
    ivec2(0, -1),
    ivec2(1, -1),
    ivec2(-1, 0),
    ivec2(0, 0),
    ivec2(1, 0),
    ivec2(-1, 1),
    ivec2(0, 1),
    ivec2(1, 1)
};

uint hasha(ivec2 p) 
{
    float L = 0.2;
    int ix = int(((float(p.x) + 2.0) / L));
    int iy = int(((float(p.y) + 2.0) / L));
    return uint(((ix * 73856093) ^ (iy * 19349663)) % HASH_SIZE);
}

uint GenerateKeyFromCellPos(ivec2 cellCoord, uint listMaxLength)
{
    return hasha(cellCoord) % listMaxLength;
}

ivec2 PositionToCellCoord(vec2 positionToCheck, float hRadius)
{
    vec2 worldCenter = vec2(0.5, 0.5) * 100.0;
    vec2 adjustedCenter = positionToCheck - worldCenter;
    adjustedCenter = adjustedCenter / hRadius;
    return ivec2(int(adjustedCenter.x), int(adjustedCenter.y));
}

uint ComputeParticleKeyLookup(vec2 cloudParticlePos, uint listMaxLength, float hRadius)
{
    ivec2 cellCoord = PositionToCellCoord(cloudParticlePos, hRadius);
    return hasha(cellCoord) % listMaxLength;
}

float CubicSplineKernel(float r, float h)
{
    if (r >= h) return 0.0;
    float q = r / h;
    float sigma = 40.0 / (7.0 * PIf * h * h); // 2D normalization

    if (q >= 0.0 && q < 0.5)
    {
        return sigma * (1.0 + 6.0 * ((q * q * q) - (q * q)));
    }
    else if (q >= 0.5 && q < 1.0)
    {
        float term = 1.0 - q;
        return sigma * 2.0 * (term * term * term);
    }
}

float CubicSplineGradient(float r, float h)
{
    if (r >= h) return 0.0;
    float q = r / h;
    float sigma = (7.0 * PIf * h * h); // 2D normalization

    if (q >= 0.0 && q < 0.5)
    {
        sigma = 40.0 / sigma;
        return sigma * (18.0 * (q * q) - 12.0 * q);
    }
    else if (q >= 0.5 && q < 1.0)
    {
        sigma = -(240.0 / sigma);
        float term = 1.0 - q;
        return sigma * (term * term);
    }
}

float NearSmoothingKernel(float radius, float dist)
{
    if (dist >= radius) return 0.0;
    // Volume for (r-d)^3
    float volume = (PIf * pow(radius, 5.0)) * 0.1;
    float value = radius - dist;
    return value * value * value / volume;
}

vec2 VogelDirection(uint sampleIndex, uint samplesCount, float Offset)
{
    float r = sqrt(float(sampleIndex) + 0.5) / sqrt(float(samplesCount));
    float theta = float(sampleIndex) * GoldenAngle + Offset;
    return r * vec2(cos(theta), sin(theta));
}

layout(binding = 0, set = 1, std140) uniform ParticleBuffer 
{
	uint numberOfParticles;
	float particleSize;
	vec2 cloudTextureDimensions;

    float graviticForce;
    float cloudDampeningMultiplier;
    float mouseForceStrength;
    float mouseForceRadius;

    float mass;
    float stiffness;
    float restDensity;
    float nearPressureMultiplier;
    
    float viscosityModifier;
    float deltaTimeModifier;
    float particleKernelRadius;
    uint bSimulateCloud;
} g_particleVariables;

struct FluidParticle 
{
    vec2 pos;
    vec2 velocity;
    vec2 intermediateVelocity;
    vec2 pressureForce;
    vec2 pressureNearPressure;
    vec2 densityNearDensity;
};

layout(binding = 1, set = 1, std430) buffer GlobalParticleList
{
    FluidParticle globalParticleList[];
};

layout(binding = 2, set = 1, std430) buffer GlobalSpatialLookupStartIndices
{
    uint globalSpatialLookupStartIndices[];
};

layout(binding = 3, set = 1, std430) buffer GlobalSpatialLookupKeyIndexList
{
    uvec2 globalSpatialLookupKeyIndexList[];
};

layout(binding = 4, set = 1, std140) buffer DynamicParticleBuffer 
{
    float maxVelocity;
	uint numberOfParticles; // TODO when this is data driven hook it up
    vec2 padding;
} g_dynamicParticleVariables;