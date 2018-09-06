#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 tex;
layout (location = 2) in vec3 norm;

out VS_OUT
{
	vec4 clipSpace;
	flat ivec3 pos;
	vec2 tex;
	vec3 norm;
	float visibility;
	vec3 toCamera;
} vs_out;

uniform mat4 view;
uniform mat4 projection;
uniform int distance;
uniform vec3 cameraPosition;

const float density = 2.3f;
const float gradient = 15.0f;

void main()
{
	vec4 posRelativeToCam = view * vec4(pos, 1.0f);
	
	float vDistance = length(posRelativeToCam.xz);
	vs_out.visibility = exp(-pow((vDistance * density / distance), gradient));
	vs_out.visibility = clamp(vs_out.visibility, 0.0f, 1.0f);

	vs_out.pos = ivec3(pos);
	vs_out.tex = vec2(pos.x, pos.z) / 32.f;
	vs_out.toCamera = cameraPosition - pos;
	vs_out.norm = norm;
	vs_out.clipSpace = projection * posRelativeToCam;
	gl_Position = vs_out.clipSpace;
}