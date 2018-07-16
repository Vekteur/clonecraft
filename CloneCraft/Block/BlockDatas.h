#pragma once

#include "BlockData.h"
#include "ID.h"

#include <array>

class BlockDatas {
public:
	BlockDatas() = default;
	BlockDatas(const std::vector<TextureArray>& texArrays);

	const BlockData& get(ID id) const;
	BlockData& get(ID id);

private:
	std::array<BlockData, ID::SIZE> blockDatas;
};

