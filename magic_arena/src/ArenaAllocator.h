#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <atomic>

#include "ArenaThreadCache.h"
#include "ArenaBlockPool.h"
#include "AllocatorType.h"

namespace Magic {

	/// TODO: Thread-safe
	/// Seperates concerns of arena memory allocation 
	class ArenaAllocator
	{
	public:
		ArenaAllocator();

		template <AllocatorType allocatorType = AllocatorType::Default>
		void* AllocateAligned(uint64_t size);

	private:
		ArenaBlockPool* GetArenaBlockPool();
		uint64_t GetLifecycle();
		void CacheArenaBlockPool(ArenaBlockPool* pool);

	private:
		ArenaBlockPool m_FirstArenaBlockPool;

		void* m_FirstThreadCache;

		/// unique for each arena, may change when resetting arena
		uint64_t m_Lifecycle = 0;
	};

	template <AllocatorType allocatorType>
	void* ArenaAllocator::AllocateAligned(uint64_t size)
	{
		ArenaBlockPool* pool = GetArenaBlockPool();

		if (pool)
		{
			MC_CORE_INFO("pool is not null");
			return pool->AllocateAligned<allocatorType>(size);
		}
		else
		{
			MC_CORE_INFO("pool is null");
			return nullptr;
			/*		pool = GetPoolFallback(size);
					return pool->AllocateAligned<allocatorType>(size);*/
		}
	}


	alignas(alignof(max_align_t))
		static inline std::atomic<uint64_t> s_Lifecycles{ 0 }; // total lifecycle count across threads

	static inline ArenaThreadCache s_ThreadCache; // thread local cache

} // namespace Magic