#version 330 core
in vec2 TexCoords;
in float visibility;
in float ambient;

out vec4 color;

uniform sampler2D texture1;
uniform vec3 skyColor;

void main()
{
	color = texture(texture1, TexCoords);
    color = mix(vec4(skyColor, 1.0f), color, visibility);
	color = vec4(color.xyz + ambient, color.w);
}