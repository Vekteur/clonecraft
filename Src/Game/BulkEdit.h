#pragma once

#include "Maths/GlmCommon.h"
#include "Block/Block.h"
#include "World/BlockEdit.h"

#include <vector>

namespace BulkEdit {
	std::vector<BlockEdit> smoothSphere(ivec3 center, int radius, Block block);
}
