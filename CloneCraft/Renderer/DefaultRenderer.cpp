#include "DefaultRenderer.h"

#include "ResManager.h"

DefaultRenderer::DefaultRenderer(TextureArray* p_texArray) 
	: p_texArray{ p_texArray }, m_shader{ ResManager::getShader("cube") } 
{ }

void DefaultRenderer::render(const DefaultMesh& mesh) {
	if (mesh.indicesNb != 0) {
		glActiveTexture(GL_TEXTURE0);
		p_texArray->bind();
		m_shader.use();

		glBindVertexArray(mesh.VAO);
		glDrawElements(GL_TRIANGLES, mesh.indicesNb, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}
}
