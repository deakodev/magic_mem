#pragma once

namespace Magic {

	// forward declare
	class ArenaBlockPool;

	/// this cache will be accessed often so aligned to
	/// 32 bytes for a good balance between memory usage 
	/// and fitting within a single cache line (usually 64 bytes)
	struct alignas(32) ArenaThreadCache
	{
		static constexpr uint64_t PerThreadLifecycles = 256; // TODO: test

		uint64_t NextLifecycle = 0;
		// cache is valid if this is equvalent to m_Lifecycle of active arena
		uint64_t LastLifecycle = -1;
		ArenaBlockPool* LastPool = nullptr;
	};
	static_assert(sizeof(ArenaThreadCache) <= 32, "ArenaThreadCache is greater than alignment!");

} // namespace Magic

