#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 2) in vec3 norm;

out VS_OUT
{
	vec3 worldPos;
	vec3 norm;
	float visibility;
} vs_out;

uniform mat4 view;
uniform mat4 projection;
uniform int distance;
uniform vec4 clipPlane;

const float density = 2.4f;
const float gradient = 15.0f;

void main()
{
	gl_ClipDistance[0] = dot(vec4(pos, 1.f), clipPlane);

	vec4 posRelativeToCam = view * vec4(pos, 1.0f);

	float vDistance = length(posRelativeToCam.xyz);
	vs_out.visibility = clamp(exp(-pow((vDistance * density / distance), gradient)), 0.0f, 1.0f);

	vs_out.worldPos = pos;
	vs_out.norm = norm;
	gl_Position = projection * posRelativeToCam;
}
