#include "PickedBlockDrawer.h"

#include "ResManager/ResManager.h"
#include "Block/BlockDatas.h"
#include "World/CubeData.h"
#include "World/Mesh.h"
#include "View/Window.h"
#include "Renderer/DefaultRenderer.h"


DefaultMesh PickedBlockDrawer::buildHeldBlockMesh(Block block) {
	const BlockData& data = ResManager::blockDatas().get(block.id);
	std::vector<DefaultMesh::Vertex> vertices;
	std::vector<GLuint> indices;
	for (Dir3D::Dir dir : Dir3D::all()) {
		const GLuint texID = data.getTexture(dir);
		const vec3 normal = Dir3D::to_ivec3(dir);
		for (GLuint idx : CubeData::faceElementIndices)
			indices.push_back(4 * static_cast<GLuint>(dir) + idx);
		for (int vtx = 0; vtx < 4; ++vtx) {
			// Centre the cube on the origin so it rotates around its middle in the HUD view.
			const vec3 pos = CubeData::dirToFace[dir][vtx] - vec3(0.5f);
			const vec2 tex = CubeData::faceCoords[vtx];
			// Full block light so the HUD preview stays bright regardless of the day/night cycle.
			vertices.push_back({ pos, tex, normal, texID, 1.f, vec2(1.f, 1.f) });
		}
	}
	DefaultMesh mesh;
	mesh.loadBuffers(vertices, indices);
	mesh.loadVAOs();
    return mesh;
}

void PickedBlockDrawer::render(std::optional<Block> picked, Window* p_window, DefaultRenderer& renderer) {
	if (!picked.has_value())
		return;
	const Block block = picked.value();
	if (ResManager::blockDatas().get(block.id).getCategory() == BlockData::AIR)
		return;

	// The mesh only depends on the block type, so rebuild it solely when the pick changes.
	if (!m_heldBlockId.has_value() || m_heldBlockId.value() != block.id) {
		m_heldBlockMesh = buildHeldBlockMesh(block);
		m_heldBlockId = block.id;
	}

	const ivec2 size = p_window->size();
	const int side = std::max(96, int(size.y * 0.12f));
	const int margin = int(size.y * 0.025f);

	// Render into a small square in the bottom-left corner, on a freshly cleared depth buffer so the
	// cube sits on top of the scene yet still self-occludes correctly.
	glClear(GL_DEPTH_BUFFER_BIT);
	glViewport(margin, margin, side, side);

	const mat4 view = glm::lookAt(vec3(1.6f, 1.5f, 1.6f), vec3(0.f), vec3(0.f, 1.f, 0.f));
	const mat4 projection = glm::perspective(glm::radians(45.f), 1.f, 0.1f, 10.f);

	const Shader& shader = renderer.getShader();
	shader.use().set("view", view);
	shader.use().set("projection", projection);
	shader.use().set("skyColor", p_window->getClearColor());
	shader.use().set("clipPlane", vec4(0.f, 0.f, 0.f, 1.f)); // always positive: never clipped

	renderer.render(m_heldBlockMesh);

	glViewport(0, 0, size.x, size.y);
}