layout(binding = 0, set = 1, std140) uniform ParticleBuffer 
{
	uint numberOfParticles;
	float cloudSizeMultiplier;
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
    float effectTurbulence;
    float effectStepLength;
    
    float effectNoise;
    float effectSunDirectionRadian;
    float HenyeyGreensteinPhaseValue;
    float BeersLawStepLengthMultiplier; 
    
    float tileableCloudThreshold;
    float tileableCloudTilingFactor;
    int numberOfSteps;
    float ditherThreshold;
} g_particleVariables;