#pragma once

#include <vector>

#include <glad/glad.h>

#include "Maths/GlmCommon.h"
#include "ResManager/Shader.h"

// Draws animated explosion fireballs. Each one is a stack of nested sphere shells, so it has real
// volume and is hidden correctly behind terrain instead of being a flat sprite. Spawned the moment
// an explosion happens and fades out on its own. All on the render thread, so no locking needed.
class ExplosionDrawer {
public:
	ExplosionDrawer();
	~ExplosionDrawer();

	ExplosionDrawer(const ExplosionDrawer&) = delete;
	ExplosionDrawer& operator=(const ExplosionDrawer&) = delete;

	// center is where the blast happens; radius is its size in blocks.
	void spawn(vec3 center, float radius);
	// Step every effect forward and remove the finished ones.
	void update(float dt);
	// Draw every active effect. clipPlane cuts the fireball at a plane (used to clip at sea level in
	// the water reflection pass). skyColor is the fog colour it fades into in the distance.
	void render(const mat4& view, const mat4& projection, const vec4& clipPlane, const vec3& skyColor);

private:
	struct Explosion {
		vec3 center;
		float size;
		float seed;
		float age;
		float duration;
		float density;
		float bigness;
	};

	// How many shells make up each fireball. More shells look smoother but cost more to draw.
	static constexpr int SHELL_COUNT = 100;

	// Build the unit sphere that every shell is a scaled copy of.
	void buildMesh();

	Shader m_shader;
	GLuint m_VAO = 0, m_VBO = 0, m_EBO = 0;
	GLsizei m_indexCount = 0;
	std::vector<Explosion> m_explosions;
};
