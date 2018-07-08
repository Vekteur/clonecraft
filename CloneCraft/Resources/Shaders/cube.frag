#version 330 core

in vec2 frag_tex;
in vec3 normal;
in float visibility;
in float light;

out vec4 color;

uniform sampler2D texture1;
uniform vec3 skyColor;

void main()
{
	const vec3 sunDir = normalize(vec3(1, 3, 2));
	const float ambient = 0.4f;
	
	float diffuse = max(dot(normal, sunDir), 0.f);
	color = vec4(vec3(ambient + diffuse + light), 1.f);

	color *= texture(texture1, frag_tex);
    color = mix(vec4(skyColor, 1.0f), color, visibility);
}