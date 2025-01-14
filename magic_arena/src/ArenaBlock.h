#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>

namespace Magic {

	/// represents a variable sized block of memory and associated header for malloc allocated objects,
	/// to be owned by a single thread arena
	class ArenaBlock
	{
	public:
		/// used for first block in arena
		ArenaBlock() = default;

		/// used for typical use cases
		ArenaBlock(const ArenaBlock* next, uint64_t size) : m_Next(next), m_Size(size) {}

		std::byte* GetMarker(uint64_t offset)
		{
			assert(offset <= m_Size);
			return reinterpret_cast<std::byte*>(reinterpret_cast<ArenaBlock*>(this)) + offset;
		}

		std::byte* GetEnd() { return GetMarker(m_Size & static_cast<uint64_t>(-8)); }

		const ArenaBlock* GetNext() const { return m_Next; }
		uint64_t GetSize() const { return m_Size; }

		bool IsFirst() const { return m_Size == 0; }

	private:
		const ArenaBlock* m_Next = nullptr;
		const uint64_t m_Size = 0;
	};


	/// returns the static dummie arena block for single thread arena creation
	ArenaBlock* GetFirstArenaBlock();

} // namespace Magic