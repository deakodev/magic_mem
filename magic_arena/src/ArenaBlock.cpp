#include "magic_pch.h"
#include "ArenaBlock.h"

namespace Magic {

	static constexpr ArenaBlock s_FirstArenaBlock;

	ArenaBlock* GetFirstArenaBlock()
	{
		std::cout << "GetFirstArenaBlock()" << std::endl;

		return const_cast<ArenaBlock*>(&s_FirstArenaBlock);
	}

} // namespace Magic