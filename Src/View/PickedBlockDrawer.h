#include <optional>
#include <vector>

#include "Block/Block.h"
#include "World/Mesh.h"

class DefaultRenderer;
class Window;

class PickedBlockDrawer {
public:
    void render(std::optional<Block> picked, Window* p_window, DefaultRenderer& renderer);

private:
    DefaultMesh buildHeldBlockMesh(Block block);

	DefaultMesh m_heldBlockMesh;
	std::optional<BlockID> m_heldBlockId;
};
