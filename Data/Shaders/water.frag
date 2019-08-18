#version 330 core

in VS_OUT
{
	vec4 clipSpace;
	flat ivec3 pos;
	vec2 tex;
	vec3 norm;
	float visibility;
	vec3 toCamera;
	float distanceFromPlayer;
} fs_in;

out vec4 color;

uniform vec3 skyColor;
uniform float moveOffset;
uniform bool simple;

uniform sampler2D reflectionTexture;
uniform sampler2D refractionTexture;
uniform sampler2D dudvMap;

const float waveStrength = 0.01f;
const float epsilon = 1e-6;

void main()
{
	vec2 ndc = (fs_in.clipSpace.xy / fs_in.clipSpace.w) / 2.f + 0.5f;
	vec2 tex = mod(mod(fs_in.tex, 1.f) + 1.f, 1.f);
	
	// Sample the dudv map with the texture coordinates and an x offset
	vec2 distortion = texture(dudvMap, tex + vec2(moveOffset, 0)).rg;
	/*distortion *= 0.1f; // Decrease the effect of the first sampling
	// Resample the dudv map with the distorted texture coordinate and a y offset
	distortion = texture(dudvMap, tex + distortion + vec2(0, moveOffset)).rg;*/
	distortion = distortion * 2.f - 1.f; // Allow negative distortion
	distortion = distortion * waveStrength;
	
	float distanceFactor = 20.f / fs_in.distanceFromPlayer; // TO IMPROVE
	//float distanceFactor = 1.f;

	vec2 refractTexCoords = ndc;
	refractTexCoords += distortion * distanceFactor;
	refractTexCoords = clamp(refractTexCoords, 0.f, 1.f);
	vec4 refractColor = texture(refractionTexture, refractTexCoords);
	
	vec4 reflectColor = vec4(skyColor, 1.f);
	// Face visible from top and at the sea level
	if (!simple && abs(fs_in.norm.y - 1.f) < epsilon && fs_in.pos.y == 64) {
		vec2 reflectTexCoords = vec2(ndc.x, 1.f - ndc.y);
		reflectTexCoords += distortion * distanceFactor;
		reflectTexCoords = clamp(reflectTexCoords, 0.f, 1.f);
		reflectColor = texture(reflectionTexture, reflectTexCoords);
	}
	
	vec3 viewVector = normalize(fs_in.toCamera);
	float refractiveFactor = dot(viewVector, fs_in.norm);
	color = vec4(1.f);
	color *= mix(reflectColor, refractColor, refractiveFactor);
	
    color = mix(vec4(skyColor, 1.f), color, fs_in.visibility);
}