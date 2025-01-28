#include "magic_pch.h"
#include "ArenaBlockPool.h"

#include "Arena.h"
#include "ArenaBlock.h"
#include "ArenaAllocator.h"

namespace Magic {

	ArenaBlockPool::ArenaBlockPool(ArenaAllocator& allocator)
		: m_Head(nullptr), m_Allocator(allocator)
	{
		m_Head = GetFirstArenaBlock();
	}

} // namespace Magic