#version 330 core

in VS_OUT
{
	vec3 worldPos;
	vec3 norm;
	float visibility;
} fs_in;

out vec4 color;

uniform vec3 skyColor;
uniform float time; // seconds, drives the flow animation

// --- 2D value noise / fbm ----------------------------------------------------
float hash(vec2 p) {
	p = fract(p * vec2(123.34f, 345.45f));
	p += dot(p, p + 34.345f);
	return fract(p.x * p.y);
}

float noise(vec2 p) {
	vec2 i = floor(p);
	vec2 f = fract(p);
	f = f * f * (3.0f - 2.0f * f);
	float a = hash(i);
	float b = hash(i + vec2(1, 0));
	float c = hash(i + vec2(0, 1));
	float d = hash(i + vec2(1, 1));
	return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
}

float fbm(vec2 p) {
	float v = 0.0f;
	float amp = 0.5f;
	const mat2 m = mat2(1.6f, 1.2f, -1.2f, 1.6f);
	for (int i = 0; i < 5; ++i) {
		v += amp * noise(p);
		p = m * p;
		amp *= 0.5f;
	}
	return v;
}

// Crust-to-white-hot gradient.
vec3 lavaPalette(float h) {
	vec3 black  = vec3(0.05f, 0.018f, 0.012f);
	vec3 dark   = vec3(0.30f, 0.05f, 0.015f);
	vec3 red    = vec3(0.75f, 0.13f, 0.02f);
	vec3 orange = vec3(1.00f, 0.42f, 0.06f);
	vec3 yellow = vec3(1.00f, 0.78f, 0.25f);
	vec3 white  = vec3(1.00f, 0.95f, 0.80f);
	vec3 c = mix(black, dark, smoothstep(0.0f, 0.28f, h));
	c = mix(c, red, smoothstep(0.28f, 0.48f, h));
	c = mix(c, orange, smoothstep(0.48f, 0.66f, h));
	c = mix(c, yellow, smoothstep(0.66f, 0.84f, h));
	c = mix(c, white, smoothstep(0.84f, 1.0f, h));
	return c;
}

// Map a world position onto the plane of its face so the pattern doesn't smear across orientations.
vec2 surfaceUV(vec3 p, vec3 n) {
	vec3 a = abs(n);
	if (a.y > 0.5f) return p.xz;
	if (a.x > 0.5f) return p.zy;
	return p.xy;
}

void main()
{
	vec2 uv = surfaceUV(fs_in.worldPos, fs_in.norm) * 0.22f;
	float t = time * 0.07f;

	// Domain-warp the field so the crust swirls and creeps like molten rock.
	vec2 warp = vec2(fbm(uv * 0.6f + vec2(0.0f, t)),
	                 fbm(uv * 0.6f + vec2(3.1f, -t)));
	float h = fbm(uv + 1.6f * warp + vec2(t * 0.4f, t * 0.15f));
	h = smoothstep(0.25f, 0.85f, h);

	// Thin, bright seams of exposed lava running between the cooler crust plates.
	float seam = 1.0f - smoothstep(0.0f, 0.06f, abs(h - 0.5f));
	h = clamp(h + 0.4f * seam, 0.0f, 1.0f);

	// Slow glow pulse so the surface looks like it's breathing heat.
	h *= 0.9f + 0.1f * sin(time * 0.8f + dot(uv, vec2(1.3f, 0.7f)) + warp.x * 6.2831f);

	vec3 lava = lavaPalette(h);
	// Push the hot parts into HDR so they read as emissive against the dark caves.
	lava *= 1.35f + 0.5f * smoothstep(0.6f, 1.0f, h);

	lava = mix(skyColor, lava, fs_in.visibility);
	color = vec4(lava, 1.0f);
}
