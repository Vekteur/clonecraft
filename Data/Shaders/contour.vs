#version 330 core
layout (location = 0) in vec3 pos;

uniform mat4 view;
uniform mat4 projection;
uniform vec3 blockPos;

// Inflate the unit cube slightly so the outline hugs the block faces
// without z-fighting against them.
const float scale = 1.002f;
const float offset = 0.001f;

void main()
{
	vec3 world = blockPos + pos * scale - vec3(offset);
	gl_Position = projection * view * vec4(world, 1.0f);
}
