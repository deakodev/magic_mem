#pragma once

#include <cassert>
#include <cstddef>

#include "SingleThreadArena.h"

namespace Magic {

	/// this cache will be accessed often so aligned to
	/// 32 bytes for a good balance between memory usage 
	/// and fitting within a single cache line (usually 64 bytes)
	struct alignas(32) ArenaThreadCache
	{
		// TODO: was called PreThreadIds set to 256
		static constexpr uint64_t ReservedLifetimeHandles = 32; // randomly chosen, reference had 256 for heavy multitreaded env need to test for optimal value

		uint64_t NextLifetimeHandle = 0;
		/// cache is valid if this is equvalent to LifetimeHandle of active arena
		uint64_t LastLifetimeHandle = -1;
		SingleThreadArena* LastArena = nullptr;
	};
	static_assert(sizeof(ArenaThreadCache) <= 32, "ArenaThreadCache is greater than alignment!");

	/// Implements and seperates concerns of arena memory allocation 
	class MultiThreadArena
	{
	public:
		MultiThreadArena();

	private:
		uint64_t GetLifetimeHandle();
		void CacheArena(SingleThreadArena* arena);

	private:
		SingleThreadArena m_FirstArena;

		void* m_FirstCache;

		/// unique for each arena, may change when reseting arena
		uint64_t m_LifetimeHandle = 0;
	};

} // namespace Magic