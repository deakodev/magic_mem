#include "magic_pch.h"
#include "MultiThreadArena.h"

namespace Magic {

	static std::atomic<uint64_t> s_LifetimeHandle;
	static ArenaThreadCache s_ThreadCache;

	MultiThreadArena::MultiThreadArena()
		: m_FirstArena(*this), m_FirstCache(&s_ThreadCache)
	{
		m_LifetimeHandle = GetLifetimeHandle();

		CacheArena(&m_FirstArena);

		std::cout << "MultiThreadArena::MultiThreadArena()" << std::endl;
	}

	uint64_t MultiThreadArena::GetLifetimeHandle()
	{
		uint64_t handle = s_ThreadCache.NextLifetimeHandle;
		uint64_t reservedHandles = ArenaThreadCache::ReservedLifetimeHandles;

		if ((handle & (reservedHandles - 1)) == 0)
		{
			handle = s_LifetimeHandle.fetch_add(1, std::memory_order_relaxed) * reservedHandles;
		}

		s_ThreadCache.NextLifetimeHandle = handle + 1;

		std::cout << "MultiThreadArena::GetLifetimeHandle()" << std::endl;

		return handle;
	}

	void MultiThreadArena::CacheArena(SingleThreadArena* arena)
	{
		s_ThreadCache.LastArena = arena;
		s_ThreadCache.LastLifetimeHandle = m_LifetimeHandle;

		std::cout << "MultiThreadArena::CacheArena()" << std::endl;
	}

} // namespace Magic

