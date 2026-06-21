#include "BlockContourDrawer.h"

#include <array>

namespace {
	// The 8 corners of a unit cube, indexed by (x | y << 1 | z << 2).
	const std::array<vec3, 8> cubeCorners{ {
		{ 0, 0, 0 }, { 1, 0, 0 }, { 0, 1, 0 }, { 1, 1, 0 },
		{ 0, 0, 1 }, { 1, 0, 1 }, { 0, 1, 1 }, { 1, 1, 1 }
	} };

	// The 12 edges of the cube, as pairs of corner indices, drawn with GL_LINES.
	const std::array<GLuint, 24> edgeIndices{
		0, 1, 1, 3, 3, 2, 2, 0, // bottom face (z = 0)
		4, 5, 5, 7, 7, 6, 6, 4, // top face (z = 1)
		0, 4, 1, 5, 2, 6, 3, 7  // vertical edges
	};
}

BlockContourDrawer::BlockContourDrawer() {
	m_shader.loadFromFile("Data/Shaders/contour.vs", "Data/Shaders/contour.frag");
	buildMesh();
}

BlockContourDrawer::~BlockContourDrawer() {
	if (m_VAO != 0)
		glDeleteVertexArrays(1, &m_VAO);
	if (m_VBO != 0)
		glDeleteBuffers(1, &m_VBO);
	if (m_EBO != 0)
		glDeleteBuffers(1, &m_EBO);
}

void BlockContourDrawer::buildMesh() {
	glGenVertexArrays(1, &m_VAO);
	glBindVertexArray(m_VAO);

	glGenBuffers(1, &m_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeCorners), cubeCorners.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &m_EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(edgeIndices), edgeIndices.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (GLvoid*)0);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void BlockContourDrawer::render(std::optional<ivec3> target, const mat4& view, const mat4& projection) {
	if (!target.has_value())
		return;

	m_shader.use();
	m_shader.set("view", view);
	m_shader.set("projection", projection);
	m_shader.set("blockPos", vec3(target.value()));
	m_shader.set("lineColor", vec4(0.f, 0.f, 0.f, 1.f));

	// Depth-test against the world so the outline is occluded by closer geometry and
	// its back edges are hidden by the targeted block. The slight inflation applied in
	// the shader keeps the visible front edges from z-fighting the block's own faces.
	const GLboolean depthWasEnabled = glIsEnabled(GL_DEPTH_TEST);
	glEnable(GL_DEPTH_TEST);
	glLineWidth(2.f);

	glBindVertexArray(m_VAO);
	glDrawElements(GL_LINES, static_cast<GLsizei>(edgeIndices.size()), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	if (!depthWasEnabled)
		glDisable(GL_DEPTH_TEST);
}
