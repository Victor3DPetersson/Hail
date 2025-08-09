#include "shaderCommons.hs"


layout(binding = 1, set = 1) uniform texture2D cloudCoverageSdfTexture;
layout(binding = 2, set = 1) uniform texture2D tileAbleCloudTexture;

const vec3 cloudColorTable[4] = {
    {115.0 / 255.0, 147.0 / 255.0, 196.0 / 255.0},
    {153.0 / 255.0, 153.0 / 255.0, 204.0 / 255.0},
    {179.0 / 255.0, 172.0 / 255.0, 205.0 / 255.0},
    {202.0 / 255.0, 189.0 / 255.0, 219.0 / 255.0}
};

#include "fluidParticleUniform.hs"

//  Mark Jarzynski and Marc Olano, Hash Functions for GPU Rendering, 
//  Journal of Computer Graphics Techniques (JCGT), vol. 9, no. 3, 21-38, 2020
//  Available online http://jcgt.org/published/0009/03/02/
float random (vec2 st) 
{
    return fract(sin(dot(st.xy, vec2(12.9898,78.233))) * 43758.5453123);
}

// Some useful functions
vec3 mod289(vec3 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec2 mod289(vec2 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec3 permute(vec3 x) { return mod289(((x*34.0)+1.0)*x); }

// Description : GLSL 2D simplex noise function
//      Author : Ian McEwan, Ashima Arts
//  Maintainer : ijm
//     Lastmod : 20110822 (ijm)
//     License :
//  Copyright (C) 2011 Ashima Arts. All rights reserved.
//  Distributed under the MIT License. See LICENSE file.
//  https://github.com/ashima/webgl-noise
// Generates values from -1 to 1
float snoise(vec2 v) 
{
    // Precompute values for skewed triangular grid
    const vec4 C = vec4(0.211324865405187,
                        // (3.0-sqrt(3.0))/6.0
                        0.366025403784439,
                        // 0.5*(sqrt(3.0)-1.0)
                        -0.577350269189626,
                        // -1.0 + 2.0 * C.x
                        0.024390243902439);
                        // 1.0 / 41.0

    // First corner (x0)
    vec2 i  = floor(v + dot(v, C.yy));
    vec2 x0 = v - i + dot(i, C.xx);

    // Other two corners (x1, x2)
    vec2 i1 = vec2(0.0);
    i1 = (x0.x > x0.y)? vec2(1.0, 0.0):vec2(0.0, 1.0);
    vec2 x1 = x0.xy + C.xx - i1;
    vec2 x2 = x0.xy + C.zz;

    // Do some permutations to avoid
    // truncation effects in permutation
    i = mod289(i);
    vec3 p = permute(permute( i.y + vec3(0.0, i1.y, 1.0)) + i.x + vec3(0.0, i1.x, 1.0 ));

    vec3 m = max(0.5 - vec3( dot(x0,x0), dot(x1,x1), dot(x2,x2)), 0.0);

    m = m*m ;
    m = m*m ;

    // Gradients:
    //  41 pts uniformly over a line, mapped onto a diamond
    //  The ring size 17*17 = 289 is close to a multiple
    //      of 41 (41*7 = 287)

    vec3 x = 2.0 * fract(p * C.www) - 1.0;
    vec3 h = abs(x) - 0.5;
    vec3 ox = floor(x + 0.5);
    vec3 a0 = x - ox;

    // Normalise gradients implicitly by scaling m Approximation of: m *= inversesqrt(a0*a0 + h*h);
    m *= 1.79284291400159 - 0.85373472095314 * (a0*a0+h*h);

    // Compute final noise value at P
    vec3 g = vec3(0.0);
    g.x  = a0.x  * x0.x  + h.x  * x0.y;
    g.yz = a0.yz * vec2(x1.x,x2.x) + h.yz * vec2(x1.y,x2.y);
    return 130.0 * dot(m, g);
}

vec2 hash2f(vec2 p)
{
	vec3 p3 = fract(vec3(p.xyx) * vec3(.1031, .1030, .0973));
    p3 += dot(p3, p3.yzx+33.33);
    return fract((p3.xx+p3.yz)*p3.zy);
}

float voronoi(vec2 x)
{
    ivec2 p = ivec2(floor(x));
    vec2  f = fract(x);

    float res = 8.0;
    for( int j=-1; j<=1; j++ )
		for( int i=-1; i<=1; i++ )
		{
			ivec2 b = ivec2( i, j );
			vec2  r = vec2( b ) - f + hash2f( vec2(p.x + b.x, p.y + b.y) );
			float d = dot( r, r );
			res = min( res, d );
		}
    return sqrt( res );
}

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec2 fragTexCoord;

float HenyeyGreensteinPhaseFunction(float g, float theta)
{
	return (1.0 / (4.0 * 3.1415926)) * (1.0 - g * g) / pow(1.0 + g * g - 2.0 * g * cos(theta), (1.5));
}

void main() 
{
	float cloudSample = texture(sampler2D(cloudCoverageSdfTexture, g_samplerBilinear), fragTexCoord).w;
	if (cloudSample <= 0.0)
		discard;

	float sunDirectionRad = g_particleVariables.effectSunDirectionRadian;
	vec2 sunDirection = normalize(vec2(cos(sunDirectionRad), sin(sunDirectionRad) * (-1.0)));

	vec2 sdfTexCoord = fragTexCoord;
	float sdfValue = texture(sampler2D(cloudCoverageSdfTexture, g_samplerBilinear), sdfTexCoord).r;

	float rayInCloudDistance = 0.0;
	vec2 currentSamplePoint = sdfTexCoord;
	vec2 toSunDirection = sunDirection * -1.0;
	float baseRayDistance = g_particleVariables.effectStepLength;
	// Change the bounds of the SDF
	sdfValue = sdfValue - 2.0;
	for (int i = 0; i < 8; i++)
	{
		if (sdfValue > 0.0)
			break;

		float rayDistance = sdfValue > 0.0 ? baseRayDistance : sdfValue * -1.0;
		rayInCloudDistance += rayDistance;
        vec2 noiseSamplePoint = currentSamplePoint;
        noiseSamplePoint.x += g_constantVariables.cameraPos.x;
        noiseSamplePoint.y -= g_constantVariables.cameraPos.y;

		float randomTurbulence = (voronoi(noiseSamplePoint * g_particleVariables.effectTurbulence) * 2.0 - 1.0) * 0.5;
		float noise = snoise(noiseSamplePoint * 2.0) * g_particleVariables.effectNoise;
		float randomDirectionShift = randomTurbulence + noise;
		vec2 rayToAdd = (rayDistance / vec2(g_constantVariables.renderTargetRes)) * (toSunDirection + randomDirectionShift);
		currentSamplePoint = rayToAdd + currentSamplePoint;
		sdfValue = texture(sampler2D(cloudCoverageSdfTexture, g_samplerBilinear), clamp(currentSamplePoint, vec2(0.0, 0.0), vec2(1.0, 1.0))).r - 2.0;
	}

	float hg = HenyeyGreensteinPhaseFunction(g_particleVariables.HenyeyGreensteinPhaseValue, sunDirectionRad);
	float beersLawValue = 1.0 - clamp(rayInCloudDistance / (g_particleVariables.BeersLawStepLengthMultiplier * baseRayDistance * 2.0), 0.0, 1.0); 
    
    vec2 tileableCloudCoord = vec2(fragTexCoord.x + g_constantVariables.cameraPos.x, fragTexCoord.y - g_constantVariables.cameraPos.y) * g_particleVariables.tileableCloudTilingFactor;
	float tileableCloudValue = texture(sampler2D(tileAbleCloudTexture, g_samplerLinear), tileableCloudCoord).r;

    int ditherY = (int(fragTexCoord.y * float(g_constantVariables.renderTargetRes.y)) % 2);
    int dither = (int(fragTexCoord.x * float(g_constantVariables.renderTargetRes.x)) + ditherY) % 2;

    float numberOfSteps = g_particleVariables.numberOfSteps;
    float beersLawRemainder = mod(beersLawValue * numberOfSteps, 1.0);
    int ditherPixel = 0;
    if (beersLawRemainder < g_particleVariables.ditherThreshold) 
    {
        ditherPixel = -1;
    }
    else if (beersLawRemainder > (1.0 - g_particleVariables.ditherThreshold))
    {
        ditherPixel = 1;
    }

    if (beersLawValue + g_particleVariables.tileableCloudThreshold > tileableCloudValue && beersLawValue - g_particleVariables.tileableCloudThreshold < tileableCloudValue)
        beersLawValue = tileableCloudValue;
        
	int quantizedBeersLawValue =  clamp(int(beersLawValue * numberOfSteps) + ditherPixel * dither, 0, int(numberOfSteps));

	vec3 color = vec3(hg, float(quantizedBeersLawValue) / numberOfSteps, sdfValue / 32.0);
    color = cloudColorTable[clamp(quantizedBeersLawValue, 0, 3)];
	//color.x = 0.0;
	//color.y = 0.0;
	//color.z = 0.0;
	outColor = vec4(color, 1.0);
}