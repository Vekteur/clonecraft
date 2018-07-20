#version 330 core

in VS_OUT
{
	vec4 clipSpace;
	vec2 tex;
	vec3 norm;
	float visibility;
	vec3 toCamera;
} fs_in;

out vec4 color;

uniform vec3 skyColor;
uniform float moveOffset;
uniform bool simple;

uniform sampler2D reflectionTexture;
uniform sampler2D refractionTexture;
uniform sampler2D dudvMap;

const float waveStrength = 0.01f;

void main()
{
	const vec3 sunDir = normalize(vec3(1, 3, 2));
	const float ambient = 0.4f;
	
	float diffuse = max(dot(fs_in.norm, sunDir), 0.f);
	color = vec4(vec3(ambient + diffuse), 1.f);
	
	vec4 reflectColor = vec4(skyColor, 1.f);
	vec4 refractColor = vec4(skyColor / skyColor.b, 0.2f);
	if (!simple) {
		vec2 ndc = (fs_in.clipSpace.xy / fs_in.clipSpace.w) / 2.f + 0.5f;
		vec2 tex = mod(mod(fs_in.tex, 1.f) + 1.f, 1.f);
		
		vec2 reflectTexCoords = vec2(ndc.x, 1.f - ndc.y);
		vec2 refractTexCoords = vec2(ndc.x, ndc.y);
		
		vec2 distortedTexCoords = texture(dudvMap, tex + vec2(moveOffset, 0)).rg * 0.1f;
		distortedTexCoords += tex + vec2(0, moveOffset);
		vec2 totalDistortion = (texture(dudvMap, distortedTexCoords).rg * 2.f - 1.f) * waveStrength;
		
		refractTexCoords += totalDistortion;
		refractTexCoords = clamp(refractTexCoords, 0.f, 1.f);
		reflectTexCoords += totalDistortion;
		reflectTexCoords = clamp(reflectTexCoords, 0.f, 1.f);
		
		reflectColor = texture(reflectionTexture, reflectTexCoords);
		refractColor = texture(refractionTexture, refractTexCoords);
	}
	
	vec3 viewVector = normalize(fs_in.toCamera);
	float refractiveFactor = dot(viewVector, vec3(0.f, 1.f, 0.f));
	color *= mix(reflectColor, refractColor, refractiveFactor);
	
    color = mix(vec4(skyColor, 1.f), color, fs_in.visibility);
}