#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoords;

out vec2 TexCoords;
out float visibility;

uniform mat4 view;
uniform mat4 projection;
uniform float distance;

const float density = 0.06f;
const float gradient = 15.0f;

void main()
{
	vec4 posRelativeToCam = view * vec4(position, 1.0f);
    gl_Position = projection * posRelativeToCam;
	
	float vDistance = length(posRelativeToCam.xyz);
	visibility = exp(-pow((vDistance * density / distance), gradient));
	visibility = clamp(visibility, 0.0f, 1.0f);
	
    TexCoords = texCoords;
}