#pragma once

namespace Const {
	const int SECTION_SIDE_POWER = 5;
	const int SECTION_HEIGHT_POWER = 5;
	const int SECTION_SIDE = 1 << SECTION_SIDE_POWER;
	const int SECTION_HEIGHT = 1 << SECTION_HEIGHT_POWER;
	const int INIT_CHUNK_NB_SECTIONS = 4;
	const int INIT_CHUNK_HEIGHT = INIT_CHUNK_NB_SECTIONS * SECTION_HEIGHT;

	const int SEA_LEVEL = 64;
}