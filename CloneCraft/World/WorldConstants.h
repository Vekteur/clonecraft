#pragma once

namespace Const
{
	const int SECTION_SIDE{ 32 }, SECTION_HEIGHT{ 32 };
	const int CHUNK_NB_SECTIONS{ 8 };
	const int CHUNK_SIDE{ SECTION_SIDE }, CHUNK_HEIGHT{ SECTION_HEIGHT * CHUNK_NB_SECTIONS };

	const int SEA_LEVEL = 64;
}