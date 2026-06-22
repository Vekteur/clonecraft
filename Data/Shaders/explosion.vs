#version 330 core
layout (location = 0) in vec3 aPos; // position on a unit sphere (also the outward direction)

uniform mat4 view;
uniform mat4 projection;
uniform vec3 center;       // world-space center of the blast
uniform float size;        // outer radius of the fireball in world units
uniform float shellRadius; // this shell's radius as a fraction of `size`, in (0, 1]
uniform vec4 clipPlane;    // world-space clip plane (used to clip at the sea level in the reflection pass)
uniform int distance;      // render distance, for the distance fog (matches the terrain shader)

out vec3 vDir;       // outward unit direction of the fragment from the blast center
out float visibility; // distance-fog factor: 1 = clear, 0 = fully hazed into the sky

const float fogDensity = 2.4f;
const float fogGradient = 15.0f;

void main()
{
	vDir = aPos; // aPos lies on the unit sphere, so it already is the outward direction
	vec3 world = center + aPos * (size * shellRadius);
	gl_ClipDistance[0] = dot(vec4(world, 1.0f), clipPlane);

	vec4 posRelativeToCam = view * vec4(world, 1.0f);
	float vDistance = length(posRelativeToCam.xyz);
	visibility = clamp(exp(-pow(vDistance * fogDensity / float(distance), fogGradient)), 0.0f, 1.0f);

	gl_Position = projection * posRelativeToCam;
}
