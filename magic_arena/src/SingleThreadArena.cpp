#include "magic_pch.h"
#include "SingleThreadArena.h"

#include "ArenaBlock.h"
#include "MultiThreadArena.h"

namespace Magic {

	SingleThreadArena::SingleThreadArena(MultiThreadArena& parent)
		: m_Blocks(nullptr), m_Parent(parent)
	{
		m_Blocks = GetFirstArenaBlock();

		std::cout << "SingleThreadArena::SingleThreadArena()" << std::endl;
	}

} // namespace Magic