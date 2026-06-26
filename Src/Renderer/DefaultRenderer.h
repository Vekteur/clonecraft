#pragma once

#include "Maths/GlmCommon.h"
#include "glad/glad.h"
#include "Renderer/Renderer.h"
#include "ResManager/TextureArray.h"
#include "World/Mesh.h"

class Section;

class DefaultRenderer : public Renderer {
public:
	DefaultRenderer(const Camera& camera, const DayCycle& dayCycle);

	void render(const Section& section) const override;
	void render(const DefaultMesh& mesh) const;
	void update(sf::Time dt) override;
	TextureArray& getTextureArray();

private:
	TextureArray texArray;
};
