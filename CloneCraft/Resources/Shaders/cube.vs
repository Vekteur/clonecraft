#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in uint texNorm;
layout (location = 2) in ivec3 blockPos;

out vec2 frag_tex;
out vec3 normal;
out float visibility;
out float light;

uniform mat4 view;
uniform mat4 projection;
uniform int distance;
uniform ivec3 target;

const float density = 2.15f;
const float gradient = 15.0f;

void main()
{
	vec4 posRelativeToCam = view * vec4(pos, 1.0f);
	
	float vDistance = length(posRelativeToCam.xz);
	visibility = exp(-pow((vDistance * density / distance), gradient));
	visibility = clamp(visibility, 0.0f, 1.0f);
	
	light = 0.f;
	if (target == blockPos)
		light = 0.3f;
		
	ivec2 itex = ivec2(texNorm & 0xFFu, (texNorm >> 8) & 0xFFu);
	frag_tex = vec2(itex.x == 0 ? itex.x : 1.f / itex.x, itex.y == 0 ? itex.y : 1.f / itex.y);
	
	int normBin = int(texNorm >> 16);
	vec3 inormal = vec3(normBin & 3, (normBin >> 2) & 3, (normBin >> 4) & 3);
	normal = vec3(inormal - 1);
	
	gl_Position = projection * posRelativeToCam;
}