#version 330 core

in VS_OUT
{
	vec2 tex;
} fs_in;

out vec4 color;

uniform sampler2D renderTexture;

void main() {
	color = texture(renderTexture, fs_in.tex);
}