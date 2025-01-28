#pragma once

#include "ArenaAllocator.h"

namespace Magic {

	/// Client facing class for arena memory allocation
	/// aligned to 8 bytes for cache line alignment
	class alignas(ArenaConfig::ALIGNMENT) Arena final // prevent inheritance
	{
	public:
		Arena() : m_Allocator() {}

		void* Allocate(uint64_t size) { return m_Allocator.AllocateAligned(size); }

	private:
		ArenaAllocator m_Allocator; /// allocation implementing class

		std::mutex m_Mutex;
	};

} // namespace Magic

