#include "PostProcessingRenderer.h"

#include "CubeData.h"
#include "ResManager.h"
#include "Logger.h"

PostProcessingRenderer::PostProcessingRenderer(ivec2 windowSize) {
	m_shader.loadFromFile("Resources/Shaders/post_processing.vs", "Resources/Shaders/post_processing.frag");

	getShader().use().set("renderTexture", 0);

	std::vector<PostProcessingMesh::Vertex> vertices;
	std::vector<GLuint> indices;
	auto& face = CubeData::dirToFace[static_cast<int>(Dir3D::DOWN)];
	for (int i = 0; i < 4; ++i) {
		vec2 pos = vec2{ face[i].x, face[i].z } * 2.f - 1.f;
		vec2 tex = CubeData::faceCoords[(i + 1) % 4];
		vertices.push_back({ pos, tex });
	}

	for (int i = 0; i < 6; ++i) {
		indices.push_back(CubeData::faceElementIndices[i]);
	}

	m_mesh.loadBuffers(vertices, indices);
	m_mesh.loadVAOs();

	onChangedSize(windowSize);
}

PostProcessingRenderer::~PostProcessingRenderer() {
	m_mesh.unloadVAOs();
}

void PostProcessingRenderer::prepare(std::function<void()> renderFunc, std::function<void()> clearFunc) {
	renderTexture.setActive(true);
	clearFunc();
	renderFunc();
	renderTexture.display();
}

void PostProcessingRenderer::render() {
	m_shader.use();
	glActiveTexture(GL_TEXTURE0);
	sf::Texture::bind(&renderTexture.getTexture());
	m_mesh.draw();
}

void PostProcessingRenderer::onChangedSize(ivec2 windowSize) {
	sf::ContextSettings settings;
	settings.majorVersion = 4;
	settings.minorVersion = 3;
	settings.depthBits = 24;
	settings.stencilBits = 8;
	if (!renderTexture.create(windowSize.x, windowSize.y, settings)) {
		LOG(Level::ERROR) << "Could not create render texture" << std::endl;
	}
}

const Shader& PostProcessingRenderer::getShader() const {
	return m_shader;
}


