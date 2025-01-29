#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "magic_pch.h"

#ifndef MAGIC_ARENA_HEADER
#define MAGIC_ARENA_HEADER

#if !( \
    (defined(__STDC__) && __STDC__ == 1 && defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) || \
    (defined(__cplusplus) && __cplusplus == 199711L) || \
    (defined(_MSC_VER) && _MSC_VER >= 1600) \
)
#error "Magic arena library requires either C99, C++20, or MSVC!"
#endif

#if __cplusplus
extern "C" {
#endif

	const size_t ALIGNMENT = 8; // default arena alignment

	// Public Utility Structs
	typedef struct {
		size_t Capacity;
		char* Memory;
		uintptr_t Next; // offset / next push destination, aka arena "fill line"
	} Magic_Arena;

	// Public Utility Functions
	Magic_Arena Magic_CreateArena(size_t alloc_capacity);
	void* Magic_PushToArena(Magic_Arena* arena, size_t item_size, size_t item_alignment, size_t item_count);
	void Magic_PopFromArena(Magic_Arena* arena, size_t size);

	// Internal Utility Functions
	void Magic_PrintError(const char* message);

	// Public Helper Macros
#define Magic_Push(arena, type) (type*)Magic_PushToArena(arena, sizeof(type), 1)
#define Magic_PushArray(arena, type, count) (type*)Magic_PushToArena(arena, sizeof(type), count)
#define Magic_Pop(arena, type) Magic_PopFromArena(arena, sizeof(type), 1)
#define Magic_PopArray(arena, type, count) Magic_PopFromArena(arena, sizeof(type), count)

#if __cplusplus
}
#endif

#endif // MAGIC_ARENA_HEADER

/*************************************
*** Implementation *******************
**************************************/

#ifdef MAGIC_ARENA_IMPL
#undef MAGIC_ARENA_IMPL

Magic_Arena Magic_CreateArena(size_t alloc_capacity)
{
	void* memory = malloc(alloc_capacity);
	if (!memory)
	{
		Magic_PrintError("Failed to allocate memory for arena!");
	}

	Magic_Arena arena = {
		.Capacity = alloc_capacity,
		.Memory = (char*)memory,
		.Next = 0
	};

	return arena;
}

void* Magic_PushToArena(Magic_Arena* arena, size_t item_size, size_t item_count)
{
	size_t total_size = item_size * item_count;
	uintptr_t next_address = (uintptr_t)arena->Memory + arena->Next;
	uintptr_t next_address_aligned = (next_address + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1);

	if (next_address_aligned + total_size > (uintptr_t)arena->Memory + arena->Capacity)
	{
		Magic_PrintError("Failed to push, arena is out of memory!");
		return NULL;
	}

	arena->Next = (next_address_aligned + total_size) - (uintptr_t)arena->Memory;

	return (void*)((uintptr_t)arena->Memory + (arena->Next - total_size));
}

void Magic_PopFromArena(Magic_Arena* arena, size_t item_size, size_t item_count)
{
	size_t total_size = item_size * item_count;

	if (arena->Next < total_size)
	{
		Magic_PrintError("Failed to pop, size deduction is greater than arena's current volume!");
		return; // TODO: handle underflow
	}

	arena->Next = (arena->Next - total_size) & ~(ALIGNMENT - 1);
}

void Magic_PrintError(const char* message)
{
	printf("%s\n", message);
}

#endif // MAGIC_ARENA_IMPL
