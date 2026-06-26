#version 330 core

in vec3 fragTex;
in vec3 normal;
in float visibility;
in float ambientOcclusion;
in vec2 lightLevel; // x = sky light, y = block light, each 0..1

out vec4 color;

uniform sampler2DArray arrayTexture;
uniform vec3 skyColor;
uniform float dayFactor; // 1 = noon, 0 = night; dims sky light only

void main()
{
	// Sky light follows the day/night cycle; block light does not.
	float level = max(lightLevel.x * dayFactor, lightLevel.y);
	// Faint floor so unlit caves are not pure black.
	float brightness = max(level, 0.12f);
	// Subtle directional tint so terrain reads as 3D (top brighter than bottom).
	float facing = 0.8f + 0.2f * (normal.y * 0.5f + 0.5f);
	color = vec4(vec3(brightness * facing * ambientOcclusion), 1.f);

	vec4 texColor = texture(arrayTexture, fragTex);
	if (texColor.a < 0.01f)
		discard;
	color *= texColor;
    color = mix(vec4(skyColor, 1.0f), color, visibility);
}