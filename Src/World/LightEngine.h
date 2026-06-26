#pragma once

#include "Maths/GlmCommon.h"
#include "Maths/Dir2D.h"
#include "Block/Block.h"

#include <array>
#include <vector>
#include <utility>

class Chunk;

namespace LightEngine {
	// Computes sky and block light for every cell of a newly generated chunk, with no light
	// coming in from neighbors yet. Sky light falls down each open column until it hits a
	// solid block, then spreads sideways losing one level per step; block light spreads the same way
	// out of glowing blocks. Cheap and self-contained; cross-chunk light is added later by
	// spillBorderLight.
	void computeChunkLight(Chunk& chunk);

	// Settles light across a chunk's four borders just before it's meshed: pushes its light into the
	// (already lit) neighbors and pulls theirs back in. The spread only ever brightens
	// a cell, so order doesn't matter. The neighbor sections it changed go into dirtySections for the caller to remesh.
	//
	// The caller must hold the global light mutex: this writes several chunks' light bytes, so two at
	// once could lose updates. It can still briefly race a neighbor meshing its own light, but the
	// remesh it queues for that neighbor rebuilds it with the final light.
	void spillBorderLight(Chunk& chunk, const std::array<Chunk*, Dir2D::SIZE>& neighbors,
		std::vector<ivec3>& dirtySections);

	// One block change for the relighter: where it happened and the block before/after. oldBlock keeps
	// the cell's old light (so we can take it back out); newBlock gives the new opacity and glow. The
	// caller must have already written newBlock with its light byte zeroed.
	struct LightEdit {
		ivec3 pos;
		Block oldBlock;
		Block newBlock;
	};

	// Fixes light after a batch of edits. For each channel it first takes back the light the edited
	// cells used to give off (and everything that depended on it), then spreads light back in from the
	// sources that remain and any new glow, so a dug-out torch goes dark, a placed block casts a
	// shadow, and an opened hole fills with sky and neighbor light. chunks is the set the update may
	// touch (the edited chunks and their neighbors); light reaching a cell outside it is dropped, like
	// the diagonal corners already are. The sections are pinned up front, so the update needs no locks.
	// Sections whose light changed go into dirtySections for the caller to remesh.
	//
	// Like spillBorderLight, the caller must hold the global light mutex: this writes several chunks'
	// light bytes at once.
	void updateEditLight(const std::vector<std::pair<ivec2, Chunk*>>& chunks,
		const std::vector<LightEdit>& edits, std::vector<ivec3>& dirtySections);
}
