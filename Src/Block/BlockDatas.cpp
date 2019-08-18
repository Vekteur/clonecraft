#include "BlockDatas.h"

#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

BlockDatas::BlockDatas(const std::vector<TextureArray*>& texArrays) {
	std::string blocksPath = "Data/Blocks";
	for (const fs::directory_entry& entry : fs::directory_iterator(blocksPath)) {
		std::ifstream ifs(entry.path());
		json j;
		ifs >> j;
		std::string filename = entry.path().stem().string();
		BlockID id = BlockID::_from_string_nocase(filename.c_str());
		blockDatas[id] = BlockData(j, texArrays);
	}
}

const BlockData& BlockDatas::get(BlockID id) const {
	return blockDatas[id];
}

BlockData & BlockDatas::get(BlockID id) {
	return blockDatas[id];
}
