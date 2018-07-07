#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 texCoords;

out vec2 TexCoords;
out float visibility;
out float ambient;

uniform mat4 view;
uniform mat4 projection;
uniform int distance;
uniform ivec3 target;

const float density = 2.15f;
const float gradient = 15.0f;

void main()
{
	vec4 posRelativeToCam = view * vec4(pos, 1.0f);
    gl_Position = projection * posRelativeToCam;
	
	float vDistance = length(posRelativeToCam.xz);
	visibility = exp(-pow((vDistance * density / distance), gradient));
	visibility = clamp(visibility, 0.0f, 1.0f);
	
	ambient = 0.f;
	if (target.x <= pos.x && pos.x <= target.x + 1 && target.y <= pos.y && pos.y <= target.y + 1 && target.z <= pos.z && pos.z <= target.z + 1)
		ambient = 0.2f;
	
    TexCoords = texCoords;
}