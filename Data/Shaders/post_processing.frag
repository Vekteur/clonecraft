#version 330 core

in VS_OUT
{
	vec2 tex;
} fs_in;

out vec4 color;

uniform sampler2D renderTexture;
uniform bool underwater;
uniform bool inLava;
uniform float time;

const vec3 waterColor = vec3(0.1f, 0.3f, 0.5f);
const float underwaterTint = 0.45f;
const vec3 lavaColor = vec3(0.72f, 0.16f, 0.02f);

void main() {
	color = texture(renderTexture, fs_in.tex);
	if (inLava) {
		float flicker = 0.88f + 0.12f * sin(time * 6.0f) * sin(time * 2.3f + fs_in.tex.x * 5.0f);
		vec2 d = fs_in.tex - 0.5f;
		float vignette = 1.0f - 0.7f * dot(d, d);
		color = vec4(lavaColor * flicker * vignette, 1.0f);
		return;
	}
	if (underwater) {
		color.rgb = mix(color.rgb, waterColor, underwaterTint);
	}
}
