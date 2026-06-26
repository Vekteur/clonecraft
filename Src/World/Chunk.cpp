#include "Chunk.h"

#include "Maths/GlmCommon.h"
#include "Maths/Converter.h"
#include "ChunkMap.h"
#include "LightEngine.h"
#include "Generator/WorldGenerator.h"

Chunk::Chunk(ChunkMap* const chunkMap, ivec2 position)
	: p_chunkMap{ chunkMap }, m_position{ position } {
	setState(TO_LOAD_BLOCKS);
}

Chunk::~Chunk() {
	setState(STATE_SIZE);
}

void Chunk::setBlock(ivec3 pos, Block block) {
	int sectionY = floorDiv(pos.y, Const::SECTION_HEIGHT);
	Section* section = nullptr;
	if (block.id == +BlockID::AIR) { // Avoid creating a section just to store air
		section = tryGetSection(sectionY);
		if (section == nullptr)
			return;
	} else {
		// Create the section if it doesn't exist yet
		section = &getOrCreateSection(sectionY);
	}
	section->setBlock({ pos.x, posMod(pos.y, Const::SECTION_HEIGHT), pos.z }, block);
}

Block Chunk::getBlock(ivec3 pos) const {
	int sectionY = floorDiv(pos.y, Const::SECTION_HEIGHT);
	const Section* section = tryGetSection(sectionY);
	if (section == nullptr)
		return { BlockID::AIR }; // outside the populated range, or a gap between sparse sections
	return section->getBlock({ pos.x, posMod(pos.y, Const::SECTION_HEIGHT), pos.z });
}

void Chunk::loadBlocks() {
	g_worldGenerator.loadChunk(*this);
	// Light this chunk on its own now, while we still own it.
	// Light that crosses chunk borders is added later, at mesh time.
	LightEngine::computeChunkLight(*this);
	setState(TO_LOAD_MESH);
}

void Chunk::loadMesh(const NeighborChunks& neighbors) {
	std::vector<Section*> sections;
	{
		std::lock_guard<std::mutex> lock(m_sectionsMutex);
		for (auto& [y, section] : m_sections)
			sections.push_back(&section);
	}
	// Build meshes without the lock.
	for (Section* section : sections)
		section->loadMesh(neighbors);
	setState(TO_RENDER);
}

void Chunk::uploadMesh() {
	std::lock_guard<std::mutex> lock(m_sectionsMutex);
	for (auto& [y, section] : m_sections)
		section.uploadMesh();
}

void Chunk::releaseMesh() {
	{
		std::lock_guard<std::mutex> lock(m_sectionsMutex);
		for (auto& [y, section] : m_sections)
			section.releaseMesh();
	}
	setState(TO_REMOVE);
}

Chunk::State Chunk::getState() const {
	return m_state.load(std::memory_order_acquire);
}

void Chunk::setState(State state) {
	State previous = m_state.exchange(state, std::memory_order_acq_rel);
	p_chunkMap->onChangeChunkState(previous, state);
}

bool Chunk::casState(State expected, State desired) {
	if (m_state.compare_exchange_strong(expected, desired, std::memory_order_acq_rel)) {
		p_chunkMap->onChangeChunkState(expected, desired);
		return true;
	}
	return false;
}

ivec2 Chunk::getPosition() const {
	return m_position;
}

void Chunk::render(const DefaultRenderer & defaultRenderer) const {
	std::lock_guard<std::mutex> lock(m_sectionsMutex);
	for (auto& [y, section] : m_sections)
		section.render(defaultRenderer);
}

void Chunk::render(const WaterRenderer& waterRenderer) const {
	std::lock_guard<std::mutex> lock(m_sectionsMutex);
	for (auto& [y, section] : m_sections)
		section.render(waterRenderer);
}

ChunkGenerationInfo& Chunk::chunkInfo() {
	return m_chunkGenerationInfo;
}

const ChunkGenerationInfo& Chunk::chunkInfo() const {
	return m_chunkGenerationInfo;
}

bool Chunk::hasSection(int sectionY) const {
	std::lock_guard<std::mutex> lock(m_sectionsMutex);
	return m_sections.find(sectionY) != m_sections.end();
}

Section& Chunk::getSection(int sectionY) {
	// Lock so the lookup can't race a section being inserted. The reference stays valid after
	// unlocking since the map never moves or removes sections.
	std::lock_guard<std::mutex> lock(m_sectionsMutex);
	return m_sections.at(sectionY);
}

const Section& Chunk::getSection(int sectionY) const {
	std::lock_guard<std::mutex> lock(m_sectionsMutex);
	return m_sections.at(sectionY);
}

std::map<int, Section>& Chunk::getSections() {
	return m_sections;
}

Section* Chunk::tryGetSection(int sectionY) {
	std::lock_guard<std::mutex> lock(m_sectionsMutex);
	auto it = m_sections.find(sectionY);
	return it == m_sections.end() ? nullptr : &it->second;
}

const Section* Chunk::tryGetSection(int sectionY) const {
	std::lock_guard<std::mutex> lock(m_sectionsMutex);
	auto it = m_sections.find(sectionY);
	return it == m_sections.end() ? nullptr : &it->second;
}

Section& Chunk::getOrCreateSection(int sectionY) {
	std::lock_guard<std::mutex> lock(m_sectionsMutex);
	return m_sections.try_emplace(sectionY, this, ivec3{ int(m_position.x), sectionY, int(m_position.y) })
		.first->second;
}

int Chunk::minSectionY() const {
	std::lock_guard<std::mutex> lock(m_sectionsMutex);
	return m_sections.empty() ? 0 : m_sections.begin()->first;
}

int Chunk::maxSectionY() const {
	std::lock_guard<std::mutex> lock(m_sectionsMutex);
	return m_sections.empty() ? 0 : m_sections.rbegin()->first;
}