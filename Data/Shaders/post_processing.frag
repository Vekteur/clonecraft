#version 330 core

in VS_OUT
{
	vec2 tex;
} fs_in;

out vec4 color;

uniform sampler2D renderTexture;
uniform bool underwater;

const vec3 waterColor = vec3(0.1f, 0.3f, 0.5f);
const float underwaterTint = 0.45f;

void main() {
	color = texture(renderTexture, fs_in.tex);
	if (underwater) {
		color.rgb = mix(color.rgb, waterColor, underwaterTint);
	}
}