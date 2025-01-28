#pragma once

#include "AllocatorType.h"

#include <cmath>

namespace Magic {

	// forward declare
	class ArenaBlock;
	class ArenaAllocator;

	struct FreeBlockList
	{
		struct FreeBlock
		{
			FreeBlock* Next = nullptr;
		};

		FreeBlock* Blocks = nullptr;
		uint32_t Size = 0;
	};

	/// child(ren) of ArenaAllocator
	/// not responsible for actual memory allocation
	/// instead owns memory blocks, manages cleanup list, 
	/// and delegates memory allocation
	class ArenaBlockPool
	{
	public:

	private:
		/// used in arena allocator construction to set first pool (child)
		ArenaBlockPool(ArenaAllocator& allocator);

		template <AllocatorType allocatorType = AllocatorType::Default>
		void* AllocateAligned(uint64_t size)
		{
			MC_CORE_ASSERT(IsSizeAligned(size), "Size does not meet alignment requirements!");
			MC_CORE_ASSERT(GetMarker() <= m_Capacity, "Not enough space in block!");

			void* allocation;

			if (allocatorType == AllocatorType::Array)
			{
				allocation = AttemptToAllocateOnFreeBlock(size);
				if (allocation)
				{
					return allocation;
				}
			}

			allocation = AttemptToAllocateOnCurrentBlock(size);
			if (allocation)
			{
				return allocation;
			}

			return AllocateOnNewBlock(size);
		}

		void* AttemptToAllocateOnFreeBlock(uint64_t size)
		{
			if (size < 16) return nullptr;

			const uint64_t freeBlockIndex = static_cast<uint64_t>(std::log2(size - 1)) - 4;

			if (freeBlockIndex >= m_FreeBlocks->Size) return nullptr;

			FreeBlockList::FreeBlock* freeBlockMarker = &m_FreeBlocks->Blocks[freeBlockIndex];
			if (freeBlockMarker->Next)
			{
				FreeBlockList::FreeBlock* freeBlock = freeBlockMarker->Next;
				freeBlockMarker->Next = freeBlock->Next;
				return freeBlock;
			}

			return nullptr;
		}

		void* AttemptToAllocateOnCurrentBlock(uint64_t size)
		{
			MC_CORE_ASSERT(IsSizeAligned(size), "Size does not meet alignment requirements!");
			MC_CORE_ASSERT(GetMarker() <= m_Capacity, "Not enough space in block!");

			// check for overflow, using uintptr_t to avoid undefined behavior
			uintptr_t overflow = (reinterpret_cast<uintptr_t>(GetMarker()) + size) > reinterpret_cast<uintptr_t>(m_Capacity);
			if (overflow)
			{
				return nullptr;
			}

			char* marker = GetMarker();
			SetMarker(marker + size);

			return marker;
		}

		void* AllocateOnNewBlock(uint64_t size)
		{
			AllocateBlock(size);
			void* allocation = AttemptToAllocateOnCurrentBlock(size);
			MC_CORE_ASSERT(allocation, "Failed to allocate on new block!");
			return nullptr;
		}

		void AllocateBlock(uint64_t size)
		{
			ArenaBlock* head = GetHead();

			auto memory = malloc(size);




		}


		ArenaBlock* GetHead() { return m_Head.load(std::memory_order_relaxed); }
		const ArenaBlock* GetHead() const { return m_Head.load(std::memory_order_relaxed); }

		char* GetMarker() { return m_Marker.load(std::memory_order_relaxed); }
		const char* GetMarker() const { return m_Marker.load(std::memory_order_relaxed); }
		void SetMarker(char* marker) { return m_Marker.store(marker, std::memory_order_relaxed); }

		static constexpr bool IsSizeAligned(uint64_t size)
		{
			return (size & (ArenaConfig::ALIGNMENT - 1)) == 0U;
		}


		friend class ArenaAllocator;

	private:
		std::atomic<ArenaBlock*> m_Head = nullptr;
		ArenaAllocator& m_Allocator;

		std::atomic<char*> m_Marker = nullptr; // next allocation point
		char* m_Capacity = nullptr; // allocate up to this address

		FreeBlockList* m_FreeBlocks = nullptr; // linked list of pointers to available blocks to reuse

	};

} // namespace Magic

