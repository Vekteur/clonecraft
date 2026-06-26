#include "LightEngine.h"

#include "Chunk.h"
#include "WorldConstants.h"
#include "ResManager/ResManager.h"
#include "Block/BlockDatas.h"
#include "Maths/Converter.h"
#include "Maths/Dir3D.h"
#include "Util/DynamicArray3D.h"

#include <vector>
#include <queue>
#include <unordered_set>
#include <algorithm>
#include <utility>

namespace {
	constexpr int SX = Const::SECTION_SIDE;   // x extent of a section/chunk
	constexpr int SZ = Const::SECTION_SIDE;   // z extent
	constexpr int SY = Const::SECTION_HEIGHT; // y extent of one section
	constexpr unsigned char SKY_FULL = 15;

	// Sections are 32 wide (a power of two), so splitting a world coord into section + inner is a shift
	// and a mask instead of floorDiv/posMod, which each do a real division. The relight resolves cells
	// millions of times, so this adds up; the static_assert holds us to the assumption.
	constexpr int SEC_SHIFT = Const::SECTION_SIDE_POWER;
	constexpr int SEC_MASK = Const::SECTION_SIDE - 1;
	static_assert(SX == (1 << SEC_SHIFT) && SZ == (1 << SEC_SHIFT) && SY == (1 << SEC_SHIFT),
		"LightVolume assumes 32-wide power-of-two sections for its shift/mask cell resolution");

	// One scratch cell of the relight grid: whether the block stops light, plus the two light channels
	// being spread. y counts up from the bottom of the chunk's lowest section.
	struct LightCell {
		unsigned char opaque = 0;
		unsigned char sky = 0;
		unsigned char blk = 0;
	};

	// One scratch cell of the flat relight grid: the block's solid / backed-by-a-section / glowing flags
	// plus the two light channels being spread. Same idea as LightCell, with the extra flags the edit
	// relight needs. The flags are packed into one byte: bits 0-3 hold the emission level, bit 4 the
	// opaque flag, bit 5 the stored flag. Cells start sealed and unstored so an unpinned column reads as
	// solid.
	struct FlatCell {
		static constexpr unsigned char EMISSION_MASK = 0x0F;
		static constexpr unsigned char OPAQUE_BIT = 0x10;
		static constexpr unsigned char STORED_BIT = 0x20;

		unsigned char flags = OPAQUE_BIT;
		Light light;

		unsigned char emission() const { return flags & EMISSION_MASK; }
		bool opaque() const { return (flags & OPAQUE_BIT) != 0; }
		bool stored() const { return (flags & STORED_BIT) != 0; }

		void setEmission(int level) {
			flags = (flags & ~EMISSION_MASK) | (static_cast<unsigned char>(level) & EMISSION_MASK);
		}
		void setOpaque(bool v) { flags = v ? (flags | OPAQUE_BIT) : (flags & ~OPAQUE_BIT); }
		void setStored(bool v) { flags = v ? (flags | STORED_BIT) : (flags & ~STORED_BIT); }
	};

	using LightEdit = LightEngine::LightEdit;

	inline int channelLevel(const Block& b, bool sky) {
		return sky ? b.light.sky() : b.light.block();
	}
}

void LightEngine::computeChunkLight(Chunk& chunk) {
	const int minY = chunk.minSectionY();
	const int maxY = chunk.maxSectionY();
	const int rows = maxY - minY + 1; // number of stacked sections, gaps included

	// Grab each existing section once so the per-cell loops below never re-lock the section map. A null
	// row is a gap and reads as see-through air.
	std::vector<Section*> secByRow(rows, nullptr);
	bool any = false;
	for (auto& [row, section] : chunk.getSections()) {
		secByRow[row - minY] = &section;
		any = true;
	}
	if (!any) // empty chunk (all air): nothing to light
		return;

	const int H = rows * SY;
	DynamicArray3D<LightCell> grid({ SX, H, SZ });

	const BlockDatas& datas = ResManager::blockDatas();

	// Read the blocks into the scratch grid once: note which cells are solid, and start the block-light
	// spread from every glowing cell.
	std::queue<ivec3> blkQueue;
	for (int r = 0; r < rows; ++r) {
		Section* section = secByRow[r];
		if (section == nullptr)
			continue;
		for (int ly = 0; ly < SY; ++ly) {
			const int y = r * SY + ly;
			for (int x = 0; x < SX; ++x)
				for (int z = 0; z < SZ; ++z) {
					const Block& block = section->getBlock({ x, ly, z });
					const BlockData& data = datas.get(block.id);
					LightCell& cell = grid.at({ x, y, z });
					if (data.isOpaque())
						cell.opaque = 1;
					const int emission = data.getEmission();
					if (emission > 0) {
						cell.blk = static_cast<unsigned char>(emission);
						blkQueue.push({ x, y, z }); // a glowing block can be solid (a lamp) and still be a source
					}
				}
		}
	}

	// Sky light: fall full-strength down each column from the open sky until the first solid block,
	// dark below it. These lit cells start the sideways spread below.
	std::queue<ivec3> skyQueue;
	for (int x = 0; x < SX; ++x)
		for (int z = 0; z < SZ; ++z) {
			bool exposed = true; // nothing above the chunk's top section but open sky
			for (int y = H - 1; y >= 0; --y) {
				LightCell& cell = grid.at({ x, y, z });
				if (cell.opaque) {
					exposed = false;
					cell.sky = 0;
				} else if (exposed) {
					cell.sky = SKY_FULL;
					skyQueue.push({ x, y, z });
				}
			}
		}

	// Spread sky light sideways and under overhangs: one level lost per step, except straight down at
	// full strength (so a shaft of sky stays bright down to its floor).
	while (!skyQueue.empty()) {
		const ivec3 c = skyQueue.front();
		skyQueue.pop();
		const int level = grid.at(c).sky;
		for (Dir3D::Dir d : Dir3D::all()) {
			const ivec3 n = c + Dir3D::to_ivec3(d);
			if (!grid.isValidIndex(n))
				continue;
			LightCell& cell = grid.at(n);
			if (cell.opaque)
				continue;
			const bool down = (d == Dir3D::DOWN);
			const int next = (down && level == SKY_FULL) ? SKY_FULL : level - 1;
			if (next > 0 && next > cell.sky) {
				cell.sky = static_cast<unsigned char>(next);
				skyQueue.push(n);
			}
		}
	}

	// Block light: spread out of the glowing cells, one level lost per step in every direction.
	while (!blkQueue.empty()) {
		const ivec3 c = blkQueue.front();
		blkQueue.pop();
		const int level = grid.at(c).blk;
		for (Dir3D::Dir d : Dir3D::all()) {
			const ivec3 n = c + Dir3D::to_ivec3(d);
			if (!grid.isValidIndex(n))
				continue;
			LightCell& cell = grid.at(n);
			if (cell.opaque)
				continue;
			const int next = level - 1;
			if (next > 0 && next > cell.blk) {
				cell.blk = static_cast<unsigned char>(next);
				blkQueue.push(n);
			}
		}
	}

	// Pack both channels back into each cell's light byte. Gap rows have no storage and keep the air
	// default (full sky), which is right for an empty column.
	for (int r = 0; r < rows; ++r) {
		Section* section = secByRow[r];
		if (section == nullptr)
			continue;
		for (int ly = 0; ly < SY; ++ly) {
			const int y = r * SY + ly;
			for (int x = 0; x < SX; ++x)
				for (int z = 0; z < SZ; ++z) {
					const LightCell& cell = grid.at({ x, y, z });
					Block& block = section->getBlock({ x, ly, z });
					block.light.setSky(cell.sky);
					block.light.setBlock(cell.blk);
				}
		}
	}
}

void LightEngine::spillBorderLight(Chunk& chunk, const std::array<Chunk*, Dir2D::SIZE>& neighbors,
		std::vector<ivec3>& dirtySections) {
	const ivec2 cpos = chunk.getPosition();
	const BlockDatas& datas = ResManager::blockDatas();

	// Find the chunk owning a world column, but only within the snapshot: this chunk or a direct
	// neighbor. A diagonal column (one chunk out on both axes) isn't pinned, so it reads as off-limits.
	auto chunkAt = [&](ivec3 gpos) -> Chunk* {
		const ivec2 relPos = Converter::globalToChunk(gpos) - cpos;
		if (relPos == ivec2{0, 0})
			return &chunk;
		for (Dir2D::Dir d : Dir2D::all()) {
			if (relPos == Dir2D::to_ivec2(d))
				return neighbors[d];
		}
		return nullptr;
	};
	auto blockAt = [&](Chunk* k, ivec3 pos) -> Block {
		return k->getBlock( Converter::globalToInnerChunk(pos) );
	};

	// A changed cell needs its own section remeshed, plus the six sections whose border faces look into
	// it (a neighbor block's face carries the light of the cell it faces).
	std::unordered_set<ivec3, Comp_ivec3, Comp_ivec3> dirty;
	auto markDirty = [&](ivec3 cell) {
		dirty.insert(Converter::globalToSection(cell));
		for (Dir3D::Dir d : Dir3D::all())
			dirty.insert(Converter::globalToSection(cell + Dir3D::to_ivec3(d)));
	};

	std::queue<ivec3> queue;
	// Brighten the target cell from a source at (srcSky, srcBlk). Only ever raises a channel, so the
	// whole spread settles no matter what order the chunks are lit in.
	auto tryRaise = [&](Chunk* tk, ivec3 cell, int srcSky, int srcBlk, bool down) {
		if (tk == nullptr)
			return;
		const Block tb = blockAt(tk, cell);
		if (datas.get(tb.id).isOpaque())
			return;
		const int skyNext = (down && srcSky == SKY_FULL) ? SKY_FULL : srcSky - 1;
		const int newSky = std::max<int>(tb.light.sky(), skyNext > 0 ? skyNext : 0);
		const int newBlk = std::max<int>(tb.light.block(), srcBlk - 1 > 0 ? srcBlk - 1 : 0);
		if (newSky <= tb.light.sky() && newBlk <= tb.light.block())
			return;
		// Air above the terrain has no stored section; it already reads as open sky, so there's nothing
		// to brighten or spread through.
		Section* s = tk->tryGetSection(floorDiv(cell.y, SY));
		if (s == nullptr)
			return;
		Block& block = s->getBlock({ Converter::globalToInnerSection(cell) });
		block.light.setSky(static_cast<unsigned char>(newSky));
		block.light.setBlock(static_cast<unsigned char>(newBlk));
		
		markDirty(cell);
		queue.push(cell);
	};

	// Seed: walk each of the four border planes and let the brighter side raise the dimmer, both ways,
	// so light both leaves this chunk and is drawn in from its neighbors.
	for (Dir2D::Dir d : Dir2D::all()) {
		Chunk* m = neighbors[d];
		if (m == nullptr)
			continue;
		const ivec2 v = Dir2D::to_ivec2(d);
		const int yLo = std::min(chunk.minSectionY(), m->minSectionY()) * SY;
		const int yHi = (std::max(chunk.maxSectionY(), m->maxSectionY()) + 1) * SY;
		for (int t = 0; t < Const::SECTION_SIDE; ++t)
			for (int y = yLo; y < yHi; ++y) {
				// This chunk's border column for direction d, and the neighbor cell one step across.
				ivec3 c_cell = (v.x != 0)
					? ivec3{ cpos.x * SX + (v.x > 0 ? SX - 1 : 0), y, cpos.y * SZ + t }
					: ivec3{ cpos.x * SX + t, y, cpos.y * SZ + (v.y > 0 ? SZ - 1 : 0) };
				ivec3 m_cell{ c_cell.x + v.x, y, c_cell.z + v.y };
				const Block cb = blockAt(&chunk, c_cell);
				const Block mb = blockAt(m, m_cell);
				tryRaise(m, m_cell, cb.light.sky(), cb.light.block(), false);      // this chunk -> neighbor
				tryRaise(&chunk, c_cell, mb.light.sky(), mb.light.block(), false); // neighbor -> this chunk
			}
	}

	// Spread the seeded light through the five-chunk neighborhood (diagonal corners excluded).
	while (!queue.empty()) {
		const ivec3 cell = queue.front();
		queue.pop();
		const Block b = blockAt(chunkAt(cell), cell); // owner pinned: it was enqueued
		const int srcSky = b.light.sky(), srcBlk = b.light.block();
		for (Dir3D::Dir d : Dir3D::all()) {
			const ivec3 n = cell + Dir3D::to_ivec3(d);
			tryRaise(chunkAt(n), n, srcSky, srcBlk, d == Dir3D::DOWN);
		}
	}

	// This chunk will be meshed in a moment with its final light, so only the neighbors' changed
	// sections need an extra rebuild.
	dirtySections.clear();
	for (const ivec3& s : dirty)
		if (s.x != cpos.x || s.z != cpos.y)
			dirtySections.push_back(s);
}

// Flat-grid sweep: copy the pinned region into one dense grid and spread light with plain index steps,
// just like computeChunkLight does for a single chunk.
static void relightFlat(const std::vector<std::pair<ivec2, Chunk*>>& chunks,
		const std::vector<LightEdit>& edits,
		std::unordered_set<ivec3, Comp_ivec3, Comp_ivec3>& dirty) {
	const BlockDatas& datas = ResManager::blockDatas();

	// Box: chunk-aligned in x/z over the pinned chunks, full section range in y (sky can fall far below
	// an edit, so the height has to cover every stored section).
	ivec2 loC = chunks.front().first, hiC = loC;
	int minSecY = chunks.front().second->minSectionY();
	int maxSecY = chunks.front().second->maxSectionY();
	for (const auto& [pos, chunk] : chunks) {
		loC = { std::min(loC.x, pos.x), std::min(loC.y, pos.y) };
		hiC = { std::max(hiC.x, pos.x), std::max(hiC.y, pos.y) };
		minSecY = std::min(minSecY, chunk->minSectionY());
		maxSecY = std::max(maxSecY, chunk->maxSectionY());
	}
	const int nx = hiC.x - loC.x + 1, nz = hiC.y - loC.y + 1;
	const int rows = maxSecY - minSecY + 1;
	const int GX = nx * SX, GY = rows * SY, GZ = nz * SZ;

	// Chunk pointer per column of the box (null for an unpinned hole in the bounding rectangle).
	std::vector<Chunk*> chunkAt(static_cast<size_t>(nx) * nz, nullptr);
	for (const auto& [pos, chunk] : chunks)
		chunkAt[(pos.x - loC.x) * nz + (pos.y - loC.y)] = chunk;

	// One dense grid over the whole box.
	DynamicArray3D<FlatCell> grid({ GX, GY, GZ });

	// Initialize the grid
	// TODO: would it be better to integrate this into the next loop?
	ivec3 c; // Chunk coords on the grid
	for (c.x = 0; c.x < nx; ++c.x)
		for (c.z = 0; c.z < nz; ++c.z) {
			if (chunkAt[c.x * nz + c.z] == nullptr)
				continue;
			ivec3 l; // Local coords of the cell in its chunk
			for (l.x = 0; l.x < SX; ++l.x)
				for (l.y = 0; l.y < GY; ++l.y)
					for (l.z = 0; l.z < SZ; ++l.z) {
						FlatCell& cell = grid.at({ c.x * SX + l.x, l.y, c.z * SZ + l.z });
						cell.setOpaque(false);
						cell.light.setSky(SKY_FULL);
					}
		}
	for (c.x = 0; c.x < nx; ++c.x)
		for (c.z = 0; c.z < nz; ++c.z) {
			Chunk* chunk = chunkAt[c.x * nz + c.z];
			if (chunk == nullptr)
				continue;
			for (int secY = chunk->minSectionY(); secY <= chunk->maxSectionY(); ++secY) {
				Section* s = chunk->tryGetSection(secY);
				if (s == nullptr)
					continue;
				ivec3 l; // Local coords of the cell in its section
				ivec3 b = { c.x * SX, (secY - minSecY) * SY, c.z * SZ }; // Grid coords of the section's base corner
				for (l.x = 0; l.x < SX; ++l.x)
					for (l.y = 0; l.y < SY; ++l.y)
						for (l.z = 0; l.z < SZ; ++l.z) {
							const Block block = s->getBlock(l);
							const BlockData& bd = datas.get(block.id);
							FlatCell& cell = grid.at(b + l);
							cell.setOpaque(bd.isOpaque());
							cell.setStored(true);
							cell.setEmission(bd.getEmission());
							cell.light.setSky(block.light.sky());
							cell.light.setBlock(block.light.block());
						}
			}
		}

	// Grid coords of a world cell
	auto toRel = [&](ivec3 p) -> ivec3 {
		return { p.x - loC.x * SX, p.y - minSecY * SY, p.z - loC.y * SZ };
	};

	// Function to mark sections for remeshing.
	bool haveLast = false;
	ivec3 lastDirty{};
	auto markDirty = [&](ivec3 r) {
		// Section position of the cell
		const ivec3 sp{ loC.x + (r.x >> SEC_SHIFT), minSecY + (r.y >> SEC_SHIFT), loC.y + (r.z >> SEC_SHIFT) };
		if (!haveLast || lastDirty != sp) {
			dirty.insert(sp);
			lastDirty = sp;
			haveLast = true;
		}
		const int ix = r.x & SEC_MASK, iy = r.y & SEC_MASK, iz = r.z & SEC_MASK;
		if (ix == 0)           dirty.insert(sp + ivec3{ -1, 0, 0 });
		else if (ix == SX - 1) dirty.insert(sp + ivec3{ 1, 0, 0 });
		if (iy == 0)           dirty.insert(sp + ivec3{ 0, -1, 0 });
		else if (iy == SY - 1) dirty.insert(sp + ivec3{ 0, 1, 0 });
		if (iz == 0)           dirty.insert(sp + ivec3{ 0, 0, -1 });
		else if (iz == SZ - 1) dirty.insert(sp + ivec3{ 0, 0, 1 });
	};

	// Three phases: take light back, seed, and spread the refill.
	auto relightChannel = [&](bool isSky) {
		auto level = [&](const FlatCell& c) { return isSky ? c.light.sky() : c.light.block(); };
		auto setLevel = [&](FlatCell& c, int v) {
			if (isSky) c.light.setSky(static_cast<unsigned char>(v));
			else       c.light.setBlock(static_cast<unsigned char>(v));
		};
		std::queue<std::pair<ivec3, int>> removeQueue;
		std::queue<ivec3> addQueue;

		// --- Take light back ---
		// Find edited cells whose old light level is positive.
		for (const LightEdit& e : edits) {
			const ivec3 c = toRel(e.pos);
			if (!grid.isValidIndex(c))
				continue;
			const int old = channelLevel(e.oldBlock, isSky);
			if (old > 0)
				removeQueue.push({ c, old });
		}
		// Take the light back from any neighbors of the edited cells that were lit by them.
		// Mark all neighbors that are too bright to have been lit by it as sources to repropagate.
		// In other words, we look at cells which might have been lit by the edited cells.
		while (!removeQueue.empty()) {
			const auto [c, c_level] = removeQueue.front();
			removeQueue.pop();
			for (Dir3D::Dir d : Dir3D::all()) {
				const ivec3 n = c + Dir3D::to_ivec3(d);
				if (!grid.isValidIndex(n))
					continue;
				FlatCell& n_cell = grid.at(n);
				const int n_level = level(n_cell);
				if (n_level == 0)
					continue;
				const int floor = !isSky ? n_cell.emission() : 0; // glowing blocks never lose their own light
				const bool down = (d == Dir3D::DOWN);
				// n was lit by this cell if it's dimmer, or for full sky falling straight down an
				// equally-full cell right below.
				const bool litByCell = n_level < c_level || (isSky && down && c_level == SKY_FULL && n_level == SKY_FULL);
				if (litByCell) {
					if (n_level > floor && n_cell.stored()) {
						setLevel(n_cell, floor);
						markDirty(n);
						removeQueue.push({ n, n_level });
					}
					if (floor > 0)
        				addQueue.push(n);
				} else {
					addQueue.push(n); // source to repropagate
				}
			}
		}
		// --- Seed ---
		// Seed the light refill from new glowing blocks and any opened cells that can take light from a neighbor.
		for (const LightEdit& e : edits) {
			const ivec3 c = toRel(e.pos);
			if (!grid.isValidIndex(c))
				continue;
			FlatCell& c_cell = grid.at(c);
			if (!c_cell.stored())
				continue;
			if (!isSky) {
				// Queue all glowing blocks
				const int emission = datas.get(e.newBlock.id).getEmission();
				if (emission > 0) {
					setLevel(c_cell, emission);
					markDirty(c);
					addQueue.push(c);
				}
			}
			if (datas.get(e.newBlock.id).isOpaque())
				continue; // a solid cell takes no light from its neighbors
			int best = 0;
			for (Dir3D::Dir d : Dir3D::all()) {
				const ivec3 n = c + Dir3D::to_ivec3(d);
				if (!grid.isValidIndex(n))
					continue;
				const int n_level = level(grid.at(n));
				const bool up = (d == Dir3D::UP);
				const int contribution = (isSky && up && n_level == SKY_FULL) ? SKY_FULL : n_level - 1;
				best = std::max(best, contribution);
			}
			if (best > level(c_cell)) {
				setLevel(c_cell, best);
				markDirty(c);
				addQueue.push(c);
			}
		}
		// --- Spread the refill ---
		// Propagate light from all the potential sources.
		while (!addQueue.empty()) {
			const ivec3 c = addQueue.front();
			addQueue.pop();
			const int lvl = level(grid.at(c));
			if (lvl <= 0)
				continue;
			for (Dir3D::Dir d : Dir3D::all()) {
				const ivec3 n = c + Dir3D::to_ivec3(d);
				if (!grid.isValidIndex(n))
					continue;
				FlatCell& cell = grid.at(n);
				if (cell.opaque())
					continue;
				const bool down = (d == Dir3D::DOWN);
				const int next = (isSky && down && lvl == SKY_FULL) ? SKY_FULL : lvl - 1;
				if (next > 0 && next > level(cell) && cell.stored()) {
					setLevel(cell, next);
					markDirty(n);
					addQueue.push(n);
				}
			}
		}
	};

	relightChannel(false); // block light
	relightChannel(true);  // sky light

	// Write the new light back into the sections the sweep changed (the stored ones inside the box; a
	// neighbor section just outside it was never written, it only needs a remesh).
	for (const ivec3& sp : dirty) {
		ivec3 sg = {sp.x - loC.x, sp.y - minSecY, sp.z - loC.y}; // Section coords on the grid
		ivec3 b = {sg.x * SX, sg.y * SY, sg.z * SZ}; // Grid coords of the section's base corner
		if (!grid.isValidIndex(b))
			continue;
		Chunk* chunk = chunkAt[sg.x * nz + sg.z];
		if (chunk == nullptr)
			continue;
		Section* s = chunk->tryGetSection(sp.y);
		if (s == nullptr)
			continue;
		ivec3 l; // Local coords of the cell in its section
		for (l.x = 0; l.x < SX; ++l.x)
			for (l.y = 0; l.y < SY; ++l.y)
				for (l.z = 0; l.z < SZ; ++l.z) {
					const FlatCell& cell = grid.at(b + l);
					Block block = s->getBlock(l);
					block.light.setSky(cell.light.sky());
					block.light.setBlock(cell.light.block());
					s->setBlock(l, block);
				}
	}
}

void LightEngine::updateEditLight(const std::vector<std::pair<ivec2, Chunk*>>& chunks,
		const std::vector<LightEdit>& edits, std::vector<ivec3>& dirtySections) {
	std::unordered_set<ivec3, Comp_ivec3, Comp_ivec3> dirty;
	if (!chunks.empty()) {
		relightFlat(chunks, edits, dirty);
	}
	dirtySections.assign(dirty.begin(), dirty.end());
}
