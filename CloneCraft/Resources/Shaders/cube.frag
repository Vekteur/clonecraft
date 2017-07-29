#version 330 core
in vec2 TexCoords;

out vec4 color;
in float visibility;

uniform sampler2D texture1;
uniform vec3 skyColor;

void main()
{          
	color = texture(texture1, TexCoords);
    color = mix(vec4(skyColor, 1.0f), color, visibility);
}