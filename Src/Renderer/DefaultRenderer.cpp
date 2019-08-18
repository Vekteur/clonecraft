#include "DefaultRenderer.h"

#include "ResManager/ResManager.h"
#include "Util/Logger.h"
#include "World/ChunkMap.h"

#include <filesystem>

namespace fs = std::filesystem;

std::vector<fs::path> getPaths() {
	std::string blockTexturesPath = "Data/Textures/Blocks";
	std::vector<fs::path> paths;
	for (const fs::directory_entry& entry : fs::directory_iterator(blockTexturesPath)) {
		if (entry.path().extension().string() == ".png") {
			paths.push_back(entry.path());
		}
	}
	return paths;
}

DefaultRenderer::DefaultRenderer() : texArray{ getPaths(), ivec2{ 16, 16 }, GL_RGBA } {
	m_shader.loadFromFile("Data/Shaders/cube.vs", "Data/Shaders/cube.frag");
	
	getShader().use().set("distance", ChunkMap::SIDE);
}

void DefaultRenderer::render(const DefaultMesh& mesh) const {
	if (mesh.indicesNb != 0) {
		glActiveTexture(GL_TEXTURE0);
		texArray.bind();
		m_shader.use();
		mesh.draw();
		texArray.unbind();
	}
}

TextureArray & DefaultRenderer::getTextureArray() {
	return texArray;
}

const Shader & DefaultRenderer::getShader() const {
	return m_shader;
}
