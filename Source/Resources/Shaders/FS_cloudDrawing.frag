#version 450

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

//Add to include file later
layout(set = 0, binding = 0, std140) uniform UniformBufferObject 
{
    uvec2 renderResolution;
    uvec2 screenResolution;
	uvec2 renderTargetRes; // Main rendertargets resolution
	vec2 totalTime_HorizonPosition;
} constantVariables;

// TODO: move global set data to a global include
layout(set = 0, binding = 2) uniform sampler samplerLinear;
layout(set = 0, binding = 3) uniform sampler samplerBilinear;

layout(std430, set = 1, binding = 0) buffer PointBuffer 
{
   	vec2 g_cloudPoints[];
};
layout(set = 1, binding = 1) uniform sampler2D cloudSdfTexture;
layout(set = 1, binding = 2) uniform texture2D cloudSdfTextureNoSampler;

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec2 fragTexCoord;

layout( push_constant ) uniform constants
{
	uvec4 numberOfPoints_padding; // uint uvec3 padding
} PushConstants;

bool CircleInRect(vec2 circlePos, float circleRadius, vec2 aabbMin, vec2 aabbMax)
{
	float sqDist = 0.0;
	for (int i = 0; i < 2; i++) 
	{
		// For each axis count any excess distance outside box extents
		float v = circlePos[i];
		sqDist += v < aabbMin[i] ? (aabbMin[i] - v) * (aabbMin[i] - v) : 0.0;
		sqDist += v > aabbMax[i] ? (v - aabbMax[i]) * (v - aabbMax[i]) : 0.0;
	}
	return sqDist <= circleRadius * circleRadius;
}

float HenyeyGreensteinPhaseFunction(float g, float theta)
{
	return (1.0 / (4.0 * 3.1415926)) * (1.0 - g * g) / pow(1.0 + g * g - 2.0 * g * cos(theta), (1.5));
}

const float cloudTextureSdfDimensions = 128.0;

void main() 
{
	float renderTexelSizeX = 1.0 / float(constantVariables.renderTargetRes.x);
	float renderTexelSizeY = 1.0 / float(constantVariables.renderTargetRes.y);

	vec2 pixelPos = fragTexCoord * constantVariables.renderTargetRes + 0.5; 
	float sampleRadiusPixelSpace = 20.0;
	// Should be constants
	vec2 cloudPos = vec2(0.5, 0.5);
	vec2 cloudDimensions = vec2(240.0, 240.0);
	float cloudRatioY = cloudDimensions.x / cloudDimensions.y;

	vec2 cloudToPixelDimensions = cloudDimensions / constantVariables.renderTargetRes;
	float sampleRadiusCloudSpace = sampleRadiusPixelSpace / cloudDimensions.y;
	float sampleInnerRadiusCloudSpace = (sampleRadiusPixelSpace * 0.15) / cloudDimensions.y;

	vec2 fragCoordInCloudSpace = (fragTexCoord + vec2(renderTexelSizeX * 0.5, renderTexelSizeY * 0.5)) / cloudToPixelDimensions;

	vec2 cloudMinPos = (cloudPos - (cloudToPixelDimensions * 0.5)) / cloudToPixelDimensions;

	// Move the fragCoord in to the normalized space of the cloud so we can iterate over the normalized points and do the transformation once. 
	vec2 pixelCloudSpacePos = fragCoordInCloudSpace - cloudMinPos;

	bool bInRect = CircleInRect(pixelCloudSpacePos, sampleRadiusCloudSpace, vec2(0.0, 0.0), vec2(1.0, 1.0));

	if (!bInRect)
		discard;
		
	float accumulatedDistance = 0.0;
	uint numberOfPointsInRadius = 0;
	uint numberOfPointsInInnerRadius = 0;
	float sampleRadiusSq = sampleRadiusCloudSpace * sampleRadiusCloudSpace;
	float innerSampleRadiusSq = sampleInnerRadiusCloudSpace * sampleInnerRadiusCloudSpace;
	for (int i = 0; i < 256; i++)
	{
		if (i == PushConstants.numberOfPoints_padding.x)
			break;

		vec2 normalizedCloudPoint = g_cloudPoints[i];
		normalizedCloudPoint.y = 1.0 - normalizedCloudPoint.y;
		vec2 noise = vec2(0.0, 0.0); 
		//noise = (hash2f((normalizedCloudPoint + pixelPos)) * 2.0 - 1.0) * 0.02;
		vec2 cloudPointToSamplePoint = pixelCloudSpacePos - normalizedCloudPoint + noise;
		float distanceSquared = dot(cloudPointToSamplePoint, cloudPointToSamplePoint);

		if (distanceSquared <= innerSampleRadiusSq)
		{
			numberOfPointsInRadius++;
			numberOfPointsInInnerRadius++;
		}
		else if (distanceSquared <= sampleRadiusSq)
		{
			accumulatedDistance += 0.05;
			numberOfPointsInRadius++;
		}
	}
	bool bIsAValidSample = numberOfPointsInInnerRadius != 0 || numberOfPointsInRadius > 2;
	if (!bIsAValidSample)
		discard;

	float sunDirectionRad = mod(3.925 + constantVariables.totalTime_HorizonPosition.x * 0.3, 3.1415926 * 2.0);
	//float sunDirectionRad = 0.0;
	vec2 sunDirection = normalize(vec2(cos(sunDirectionRad), sin(sunDirectionRad) * (-1.0)));

	float sdfValue = 0.0;
	vec2 sdfTexCoord = vec2(clamp(pixelCloudSpacePos.x, 0.0, 1.0), clamp(1.0 - pixelCloudSpacePos.y, 0.0, 1.0));
	sdfValue = texture(sampler2D(cloudSdfTextureNoSampler, samplerBilinear), sdfTexCoord + snoise(fragTexCoord * 10.0) * 0.01).r;

	float rayInCloudDistance = 0.0;
	vec2 currentSamplePoint = sdfTexCoord;
	vec2 toSunDirection = sunDirection * -1.0;
	float baseRayDistance = 4.0;
	// Change the bounds of the SDF
	sdfValue = sdfValue - 2.0;

	for (int i = 0; i < 8; i++)
	{
		if (sdfValue > 0.0)
			break;

		float rayDistance = sdfValue > 0.0 ? baseRayDistance : sdfValue * -1.0;
		rayInCloudDistance += rayDistance;

		float randomDirectionShift = (voronoi(currentSamplePoint * 10.0) * 2.0 - 1.0) * 0.5 + snoise(currentSamplePoint * 2.0) * 0.2;
		vec2 rayToAdd = (rayDistance / cloudTextureSdfDimensions) * (toSunDirection + randomDirectionShift);
		currentSamplePoint = rayToAdd + currentSamplePoint;
		sdfValue = texture(sampler2D(cloudSdfTextureNoSampler, samplerBilinear), clamp(currentSamplePoint, vec2(0.0, 0.0), vec2(1.0, 1.0))).r - 2.0;
	}

	float hg = HenyeyGreensteinPhaseFunction(0.95, sunDirectionRad);
	float beersLawValue = 1.0 - clamp(rayInCloudDistance / (8.0 * baseRayDistance * 2.0), 0.0, 1.0); 
	int quantizedValue = int(beersLawValue * 5.0);

	vec3 color = vec3(beersLawValue, float(quantizedValue) * 0.2, 0.0);
	//color.x = 0.0;
	//color.y = 0.0;
	//color.z = 0.0;
	outColor = vec4(color, 1.0);
}