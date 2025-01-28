#include "magic_pch.h"
#include "ArenaAllocator.h"

#include "ArenaBlockPool.h"

namespace Magic {

	ArenaAllocator::ArenaAllocator()
		: m_FirstArenaBlockPool(*this), m_FirstThreadCache(&s_ThreadCache)
	{
		m_Lifecycle = GetLifecycle();

		CacheArenaBlockPool(&m_FirstArenaBlockPool);
	}

	// when multiple threads allocate from the same arena, 
	// this ensures that if a thread already owns a pool, it will use it
	ArenaBlockPool* ArenaAllocator::GetArenaBlockPool()
	{
		if (s_ThreadCache.LastLifecycle == m_Lifecycle)
		{
			return s_ThreadCache.LastPool;
		}

		return nullptr;
	}

	uint64_t ArenaAllocator::GetLifecycle()
	{
		uint64_t lifecycle = s_ThreadCache.NextLifecycle;
		uint64_t perThreadLifecycles = ArenaThreadCache::PerThreadLifecycles;

		if ((lifecycle & (perThreadLifecycles - 1)) == 0)
		{
			lifecycle = s_Lifecycles.fetch_add(1, std::memory_order_relaxed) * perThreadLifecycles;
		}

		s_ThreadCache.NextLifecycle = lifecycle + 1;

		return lifecycle;
	}

	void ArenaAllocator::CacheArenaBlockPool(ArenaBlockPool* pool)
	{
		s_ThreadCache.LastPool = pool;
		s_ThreadCache.LastLifecycle = m_Lifecycle;
	}

} // namespace Magic

