#include "WaterRenderer.h"

#include "ResManager.h"
#include "Logger.h"

#include <filesystem>

namespace fs = std::filesystem;

WaterRenderer::WaterRenderer() {
	m_shader.loadFromFile("Resources/Shaders/water.vs", "Resources/Shaders/water.frag");
	ResManager::setShader(m_shader, "water");
}

void WaterRenderer::render(const WaterMesh& mesh) const {
	if (mesh.indicesNb != 0) {
		m_shader.use();

		mesh.draw();
	}
}