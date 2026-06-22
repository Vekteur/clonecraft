#include "ExplosionDrawer.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <vector>

#include "World/ChunkMap.h"

namespace {
	// How detailed the sphere is. It only needs to look round; the fire detail comes from the
	// shader's noise, not the geometry.
	constexpr int SPHERE_SLICES = 32; // longitude divisions
	constexpr int SPHERE_STACKS = 16; // latitude divisions
	constexpr float PI = glm::pi<float>();

	// Build a unit sphere. Each vertex also doubles as an outward direction the shader uses for noise.
	void buildUnitSphere(std::vector<vec3>& positions, std::vector<GLuint>& indices) {
		for (int stack = 0; stack <= SPHERE_STACKS; ++stack) {
			float phi = PI * float(stack) / float(SPHERE_STACKS); // 0 (top) .. PI (bottom)
			float sinPhi = std::sin(phi);
			float cosPhi = std::cos(phi);
			for (int slice = 0; slice <= SPHERE_SLICES; ++slice) {
				float theta = 2.f * PI * float(slice) / float(SPHERE_SLICES);
				positions.push_back({ sinPhi * std::cos(theta), cosPhi, sinPhi * std::sin(theta) });
			}
		}

		const int stride = SPHERE_SLICES + 1;
		for (int stack = 0; stack < SPHERE_STACKS; ++stack) {
			for (int slice = 0; slice < SPHERE_SLICES; ++slice) {
				GLuint a = GLuint(stack * stride + slice);
				GLuint b = GLuint(a + stride);
				indices.insert(indices.end(), { a, b, a + 1, a + 1, b, b + 1 });
			}
		}
	}
}

ExplosionDrawer::ExplosionDrawer() {
	m_shader.loadFromFile("Data/Shaders/explosion.vs", "Data/Shaders/explosion.frag");
	// Fog distance, matching the terrain shader.
	m_shader.use().set("distance", ChunkMap::SIDE);
	buildMesh();
}

ExplosionDrawer::~ExplosionDrawer() {
	if (m_EBO != 0)
		glDeleteBuffers(1, &m_EBO);
	if (m_VBO != 0)
		glDeleteBuffers(1, &m_VBO);
	if (m_VAO != 0)
		glDeleteVertexArrays(1, &m_VAO);
}

void ExplosionDrawer::buildMesh() {
	std::vector<vec3> positions;
	std::vector<GLuint> indices;
	buildUnitSphere(positions, indices);
	m_indexCount = static_cast<GLsizei>(indices.size());

	glGenVertexArrays(1, &m_VAO);
	glBindVertexArray(m_VAO);

	glGenBuffers(1, &m_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(vec3), positions.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &m_EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (GLvoid*)0);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void ExplosionDrawer::spawn(vec3 center, float radius) {
	Explosion e;
	e.center = center;
	// A bit bigger than the crater so it spills past the rim.
	e.size = radius * 1.5f;
	e.seed = float(std::rand()) / float(RAND_MAX) * 100.f;
	e.age = 0.f;
	// Bigger blasts last longer. Mostly proportional, so a small one stays quick and snappy.
	e.duration = 0.22f + 0.019f * radius;
	// Bigger blasts look denser and more opaque.
	e.density = 8.f + 0.14f * radius;
	// 0 for a small blast, 1 for a huge one. The shader uses this to go from a sharp burst to a slow
	// rolling fireball.
	e.bigness = std::clamp((radius - 15.f) / 85.f, 0.f, 1.f);
	m_explosions.push_back(e);
}

void ExplosionDrawer::update(float dt) {
	for (Explosion& e : m_explosions)
		e.age += dt;
	m_explosions.erase(
		std::remove_if(m_explosions.begin(), m_explosions.end(),
			[](const Explosion& e) { return e.age >= e.duration; }),
		m_explosions.end());
}

void ExplosionDrawer::render(const mat4& view, const mat4& projection, const vec4& clipPlane, const vec3& skyColor) {
	if (m_explosions.empty())
		return;

	// Remember the GL state we change so we can put it back afterwards.
	const GLboolean depthWasEnabled = glIsEnabled(GL_DEPTH_TEST);
	const GLboolean blendWasEnabled = glIsEnabled(GL_BLEND);
	const GLboolean cullWasEnabled = glIsEnabled(GL_CULL_FACE);
	const GLboolean clipWasEnabled = glIsEnabled(GL_CLIP_DISTANCE0);
	GLboolean depthMask;
	glGetBooleanv(GL_DEPTH_WRITEMASK, &depthMask);
	GLint blendSrc, blendDst;
	glGetIntegerv(GL_BLEND_SRC_RGB, &blendSrc);
	glGetIntegerv(GL_BLEND_DST_RGB, &blendDst);

	// Test against the world's depth so terrain in front hides the fireball, but don't write depth:
	// the shells overlap and shouldn't block each other; the draw order below handles that. Culling
	// is off so both the near and far side of each shell get drawn.
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glDisable(GL_CULL_FACE);
	// Keep clipping on so the water reflection pass can cut the fireball at sea level.
	glEnable(GL_CLIP_DISTANCE0);
	glEnable(GL_BLEND);
	// Premultiplied "over" blending. Each shell both glows and covers what's behind it, so the
	// fireball looks like a solid mass rather than a see-through glow.
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	m_shader.use();
	m_shader.set("view", view);
	m_shader.set("projection", projection);
	m_shader.set("clipPlane", clipPlane);
	m_shader.set("skyColor", skyColor);
	m_shader.set("shellCount", SHELL_COUNT);

	glBindVertexArray(m_VAO);
	for (const Explosion& e : m_explosions) {
		m_shader.set("center", e.center);
		m_shader.set("size", e.size);
		m_shader.set("seed", e.seed);
		m_shader.set("density", e.density);
		m_shader.set("bigness", e.bigness);
		m_shader.set("progress", e.age / e.duration);
		// Draw the shells outer to inner so the bright core layers on top of the dimmer outer haze.
		// The shader decides per pixel whether each shell is currently inside the fireball, which is
		// what makes the blast look like it expands over time.
		for (int shell = SHELL_COUNT; shell >= 1; --shell) {
			m_shader.set("shellRadius", float(shell) / float(SHELL_COUNT));
			glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, 0);
		}
	}
	glBindVertexArray(0);

	// Put the GL state back.
	glDepthMask(depthMask);
	glBlendFunc(blendSrc, blendDst);
	if (!blendWasEnabled)
		glDisable(GL_BLEND);
	if (cullWasEnabled)
		glEnable(GL_CULL_FACE);
	if (!clipWasEnabled)
		glDisable(GL_CLIP_DISTANCE0);
	if (!depthWasEnabled)
		glDisable(GL_DEPTH_TEST);
}
