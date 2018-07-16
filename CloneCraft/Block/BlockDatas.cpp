#include "BlockDatas.h"

#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

BlockDatas::BlockDatas(const std::vector<TextureArray>& texArrays) {
	std::string blocksPath = "Resources/Blocks";
	for (const fs::directory_entry& entry : fs::directory_iterator(blocksPath)) {
		std::ifstream ifs(entry.path());
		json j;
		ifs >> j;
		std::string filename = entry.path().stem().string();
		ID id = ID::_from_string_nocase(filename.c_str());
		blockDatas[id] = BlockData(j, texArrays);
	}
}

const BlockData& BlockDatas::get(ID id) const {
	return blockDatas[id];
}

BlockData & BlockDatas::get(ID id) {
	return blockDatas[id];
}
