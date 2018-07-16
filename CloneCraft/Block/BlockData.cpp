#include "BlockData.h"

#include "Logger.h"
#include "ID.h"

BlockData::BlockData() {}

int findID(std::string name, const std::vector<TextureArray*>& texArrays) {
	for (TextureArray* texArray : texArrays) {
		if (texArray->containsTexture(name)) {
			return texArray->getTextureID(name);
		}
	}
	throw "Bad texture name : " + name;
}

BlockData::BlockData(const json& j, const std::vector<TextureArray*>& texArrays) {
	m_opaque = j.value("opaque", false);
	m_obstacle = j.value("obstacle", false);
	m_resistance = j.value("resistance", 0);
	std::string category = j.value("category", "");
	if (category == "default")
		m_category = DEFAULT;
	else if (category == "liquid")
		m_category = LIQUID;
	else if (category == "air")
		m_category = AIR;
		
	json::const_iterator it;
	it = j.find("textures");
	if (it != j.end()) {
		auto texJ = *it;
		it = texJ.find("all");
		if (it != texJ.end()) {
			for (Dir3D::Dir dir : Dir3D::all()) {
				m_textures[dir] = findID(*it, texArrays);
			}
		}
		it = texJ.find("side");
		if (it != texJ.end()) {
			for (Dir3D::Dir dir : Dir3D::horizontal()) {
				m_textures[dir] = findID(*it, texArrays);
			}
		}
		it = texJ.find("up");
		if (it != texJ.end())
			m_textures[Dir3D::UP] = findID(*it, texArrays);
		it = texJ.find("front");
		if (it != texJ.end())
			m_textures[Dir3D::FRONT] = findID(*it, texArrays);
		it = texJ.find("right");
		if (it != texJ.end())
			m_textures[Dir3D::RIGHT] = findID(*it, texArrays);
		it = texJ.find("down");
		if (it != texJ.end())
			m_textures[Dir3D::DOWN] = findID(*it, texArrays);
		it = texJ.find("back");
		if (it != texJ.end())
			m_textures[Dir3D::BACK] = findID(*it, texArrays);
		it = texJ.find("left");
		if (it != texJ.end())
			m_textures[Dir3D::LEFT] = findID(*it, texArrays);
	}
}

bool BlockData::isOpaque() const {
	return m_opaque;
}

bool BlockData::isObstacle() const {
	return m_obstacle;
}

int BlockData::getResistance() const {
	return m_resistance;
}

BlockData::Category BlockData::getCategory() const {
	return m_category;
}

int BlockData::getTexture(Dir3D::Dir dir) const {
	return m_textures[dir];
}