#version 330 core

layout (location = 0) in vec2 pos;
layout (location = 1) in vec2 tex;

out VS_OUT
{
	vec2 tex;
} vs_out;

void main()
{
	gl_Position = vec4(pos, 0.0f, 1.0f); 
	vs_out.tex = tex;
}