#version 330 core

in vec4 clipSpace;
in vec3 normal;
in float visibility;

out vec4 color;

uniform vec3 skyColor;
uniform sampler2D reflectionTexture;
uniform sampler2D refractionTexture;

void main()
{
	const vec3 sunDir = normalize(vec3(1, 3, 2));
	const float ambient = 0.4f;
	
	float diffuse = max(dot(normal, sunDir), 0.f);
	color = vec4(vec3(ambient + diffuse), 1.f);
	
	vec2 ndc = (clipSpace.xy / clipSpace.w) / 2.f + 0.5f;
	vec4 reflectColor = texture(reflectionTexture, vec2(ndc.x, 1.f - ndc.y));
	vec4 refractColor = texture(refractionTexture, vec2(ndc.x, ndc.y));
	color *= mix(reflectColor, refractColor, 0.5f);
	
    color = mix(vec4(skyColor, 1.0f), color, visibility);
}