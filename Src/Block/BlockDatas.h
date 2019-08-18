#pragma once

#include "BlockData.h"
#include "BlockID.h"

#include <array>

class BlockDatas {
public:
	BlockDatas() = default;
	BlockDatas(const std::vector<TextureArray*>& texArrays);

	const BlockData& get(BlockID id) const;
	BlockData& get(BlockID id);

private:
	std::array<BlockData, BlockID::SIZE> blockDatas;
};

