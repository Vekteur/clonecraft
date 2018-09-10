#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 tex;
layout (location = 2) in vec3 norm;
layout (location = 3) in uint texID;

out vec3 fragTex;
out vec3 normal;
out float visibility;

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
	
	float vDistance = length(posRelativeToCam.xz);
	visibility = exp(-pow((vDistance * density / distance), gradient));
	visibility = clamp(visibility, 0.0f, 1.0f);
	
	fragTex = vec3(tex, texID);
	normal = norm;
	
	gl_Position = projection * posRelativeToCam;
}