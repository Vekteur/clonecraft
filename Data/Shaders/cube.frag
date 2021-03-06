#version 330 core

in vec3 fragTex;
in vec3 normal;
in float visibility;

out vec4 color;

uniform sampler2DArray arrayTexture;
uniform vec3 skyColor;

void main()
{
	const vec3 sunDir = normalize(vec3(1, 3, 2));
	const float ambient = 0.4f;
	
	float diffuse = max(dot(normal, sunDir), 0.f);
	color = vec4(vec3(ambient + diffuse), 1.f);

	vec4 texColor = texture(arrayTexture, fragTex);
	if (texColor.a < 0.01f)
		discard;
	color *= texColor;
    color = mix(vec4(skyColor, 1.0f), color, visibility);
}