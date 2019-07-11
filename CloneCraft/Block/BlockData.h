#pragma once

#include "Maths/Dir3D.h"
#include "Util/json.hpp"

#include "ResManager/TextureArray.h"

#include <array>
#include <vector>

using json = nlohmann::json;

class BlockData {
public:
	enum Category {
		DEFAULT, SEMI_TRANSPARENT, WATER, AIR
	};

	BlockData();
	BlockData(const json& j, const std::vector<TextureArray*>& texArrays);

	bool isOpaque() const;
	bool isObstacle() const;
	int getResistance() const;
	Category getCategory() const;
	int getTexture(Dir3D::Dir dir) const;

private:
	bool m_opaque = false;
	bool m_obstacle = false;
	int m_resistance = 0;
	Category m_category = DEFAULT;
	std::array<int, Dir3D::SIZE> m_textures;
};
