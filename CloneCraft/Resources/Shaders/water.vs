#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 tex;
layout (location = 2) in vec3 norm;

out vec4 clipSpace;
out vec3 normal;
out float visibility;

uniform mat4 view;
uniform mat4 projection;
uniform int distance;

const float density = 2.15f;
const float gradient = 15.0f;

void main()
{
	vec4 posRelativeToCam = view * vec4(pos, 1.0f);
	
	float vDistance = length(posRelativeToCam.xz);
	visibility = exp(-pow((vDistance * density / distance), gradient));
	visibility = clamp(visibility, 0.0f, 1.0f);
	
	normal = norm;
	clipSpace = projection * posRelativeToCam;
	gl_Position = clipSpace;
}