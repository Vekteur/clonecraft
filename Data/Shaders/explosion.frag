#version 330 core
in vec3 vDir;
in float visibility; // distance-fog factor from the vertex shader: 1 = clear, 0 = fully hazed
out vec4 fragColor;

uniform float progress;    // normalized lifetime, 0 at spawn, 1 at death
uniform float seed;        // per-explosion offset so no two blasts look alike
uniform float shellRadius; // this shell's normalized radius from the center, in (0, 1]
uniform int   shellCount;  // number of nested shells (used to normalize the build-up)
uniform float density;     // optical thickness of the medium; larger = more opaque (scales with blast size)
uniform vec3  skyColor;    // fog colour the fireball fades into with distance (matches the terrain)
uniform float bigness;     // 0 for a small blast, 1 for a huge one: blends the small/big behaviour

// --- 3D value noise / fbm ----------------------------------------------------
float hash(vec3 p) {
	p = fract(p * 0.3183099f + 0.1f);
	p *= 17.0f;
	return fract(p.x * p.y * p.z * (p.x + p.y + p.z));
}

float noise(vec3 x) {
	vec3 i = floor(x);
	vec3 f = fract(x);
	f = f * f * (3.0f - 2.0f * f);
	return mix(mix(mix(hash(i + vec3(0, 0, 0)), hash(i + vec3(1, 0, 0)), f.x),
	               mix(hash(i + vec3(0, 1, 0)), hash(i + vec3(1, 1, 0)), f.x), f.y),
	           mix(mix(hash(i + vec3(0, 0, 1)), hash(i + vec3(1, 0, 1)), f.x),
	               mix(hash(i + vec3(0, 1, 1)), hash(i + vec3(1, 1, 1)), f.x), f.y), f.z);
}

float fbm(vec3 p) {
	float v = 0.0f;
	float amp = 0.5f;
	for (int i = 0; i < 5; ++i) {
		v += amp * noise(p);
		p *= 2.0f;
		amp *= 0.5f;
	}
	return v;
}

// Heat (0 = cold smoke, 1 = white-hot core) mapped to a fire gradient. `big` (0..1) shifts the
// palette with the blast size
vec3 fireColor(float h, float big) {
	vec3 smoke  = mix(vec3(0.06f, 0.05f, 0.05f), vec3(0.02f, 0.016f, 0.013f), big);
	vec3 red    = mix(vec3(0.75f, 0.12f, 0.02f), vec3(0.52f, 0.07f, 0.012f), big);
	vec3 orange = mix(vec3(1.00f, 0.45f, 0.07f), vec3(0.88f, 0.32f, 0.04f), big);
	vec3 yellow = mix(vec3(1.00f, 0.80f, 0.25f), vec3(0.98f, 0.64f, 0.14f), big);
	vec3 white  = mix(vec3(1.00f, 0.96f, 0.85f), vec3(1.00f, 0.92f, 0.76f), big);
	vec3 c = mix(smoke, red, smoothstep(0.0f, 0.30f, h));
	c = mix(c, orange, smoothstep(0.30f, 0.55f, h));
	c = mix(c, yellow, smoothstep(0.55f, 0.80f, h));
	c = mix(c, white,  smoothstep(0.80f, 1.00f, h));
	return c;
}

void main()
{
	float t = progress;
	float r = shellRadius;
	vec3 dir = normalize(vDir);

	vec3 seedVec = vec3(seed, seed * 1.7f, seed * 0.7f);

	float freqScale = mix(1.4f, 1.0f, bigness); // finer relative detail on small blasts

	// Turbulent, boiling noise scrolled over the lifetime (higher frequency = more violent).
	vec3 np = dir * (1.6f + 1.2f * r) * freqScale + seedVec;
	float turb = fbm(np - vec3(0.0f, t * 3.2f, 0.0f));
	float detail = fbm(dir * 5.5f * freqScale + seedVec - vec3(0.0f, 0.0f, t * 2.2f));

	// Burst outward fast then settle, with a ragged noise-driven rim.
	float expand = 1.0f - pow(1.0f - t, mix(3.4f, 2.4f, bigness));
	float fireR = mix(0.30f, 1.0f, expand);
	float edge = fireR * (0.72f + mix(0.45f, 0.55f, bigness) * turb);

	float ball = smoothstep(edge, edge - 0.40f, r);
	float core = smoothstep(fireR * 0.60f, 0.0f, r);
	float body = max(ball, core);

	// Hot in the middle, cooling toward the rim and as the blast ages.
	float heat = (1.0f - r / max(edge, 0.001f)) * body * (0.50f + 0.70f * detail);
	heat = max(heat, core);
	heat *= (1.0f - 0.7f * t);
	heat = clamp(heat, 0.0f, 1.0f);

	// Small blasts burn hotter and cleaner: lift the heat so the body glows orange-yellow instead of
	// settling into the deep reds, where the unbiased gradient otherwise spends most of its range. Big
	// blasts keep their cooler, redder gradient (exponent 1 leaves the heat untouched).
	heat = pow(heat, mix(0.55f, 1.0f, bigness));

	// Blinding white flash in the very first instants.
	float flash = smoothstep(mix(0.10f, 0.16f, bigness), 0.0f, t) * smoothstep(1.0f, 0.2f, r);

	// Bright shock ring that races out and is gone by mid-life.
	float ringR = mix(0.1f, 1.0f, 1.0f - pow(1.0f - t, mix(1.7f, 1.3f, bigness)));
	float ringWidth = mix(0.06f, 0.09f, bigness);
	float ringFade = 1.0f - smoothstep(mix(0.30f, 0.40f, bigness), mix(0.65f, 0.85f, bigness), t);
	float ring = smoothstep(ringWidth, 0.0f, abs(r - ringR)) * ringFade;

	// Overdrive the colour into HDR so the hot core blooms into a blown-out highlight.
	float intensity = 2.4f * (1.0f - 0.5f * t);
	vec3 color = fireColor(heat, bigness) * intensity;
	color += vec3(1.0f, 0.95f, 0.85f) * ring * mix(3.2f, 2.2f, bigness);
	color += vec3(1.0f, 0.98f, 0.92f) * flash * 4.0f;

	// Quick flash in, long fade out.
	float fadeIn = smoothstep(0.0f, 0.04f, t);
	float fadeOut = 1.0f - smoothstep(0.55f, 1.0f, t);

	// Local opacity of the medium.
	float coverage = clamp(body + flash + 0.6f * ring, 0.0f, 1.0f) * fadeIn * fadeOut;

	// A view ray crosses many shells, so each one carries only a fraction of the total opacity.
	float aShell = clamp(coverage * density / float(shellCount), 0.0f, 1.0f);

	if (aShell < 0.002f)
		discard;

	// Fog
	color = mix(skyColor, color, visibility);

	// Premultiplied-alpha output for "over" compositing (glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA)):
	// `color` is added as emissive light while `aShell` simultaneously covers the background, so the
	// fireball both glows and hides the terrain behind it.
	fragColor = vec4(color * aShell, aShell);
}
