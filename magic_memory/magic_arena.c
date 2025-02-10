#include "magic_memory.h"

#include "magic_error.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int magic_arena_allocate(Magic_Arena* arena, const uint32_t size)
{
	CHECK_PTR(arena);
	*arena = (Magic_Arena){ 0 };

	STATUS(magic_allocate(&arena->memory, size));

	arena->write_marker = 0;
	arena->end_marker = size;
	arena->dynamic = false;
	arena->allocated = true;

	return MAGIC_STATUS_OK;
}

int magic_arena_validate(const Magic_Arena* arena)
{
	CHECK_PTR(arena);
	CHECK(arena->write_marker <= arena->end_marker, MAGIC_ERROR_ARENA_INVALID);
	CHECK(arena->end_marker <= arena->memory.size, MAGIC_ERROR_ARENA_INVALID);

	const Magic_Memory* memory = &arena->memory;

	CHECK_PTR(memory);
	CHECK(memory->data != NULL || memory->size == 0, MAGIC_ERROR_ARENA_INVALID);
	CHECK(memory->data != NULL || memory->allocated == 0, MAGIC_ERROR_ARENA_INVALID);
	CHECK(memory->dynamic || memory->allocated == 0, MAGIC_ERROR_ARENA_INVALID);
	CHECK(!memory->dynamic || memory->size <= memory->allocated, MAGIC_ERROR_ARENA_INVALID);

	return MAGIC_STATUS_OK;
}

int magic_arena_write(Magic_Arena* arena, void** data, uint32_t size)
{
	CHECK_PTR(arena);
	CHECK_PTR(data);
	STATUS(magic_arena_validate(arena));

	CHECK(size > 0, MAGIC_ERROR_WRITE_FAILED);
	CHECK(arena->write_marker + size <= arena->end_marker, MAGIC_ERROR_WRITE_FAILED);
	// TODO: handle overflow
	// TODO: alignment check?

	/*void* dest = arena->memory.data + arena->write_marker;*/
	//memcpy(dest, *data, size);
	*data = arena->memory.data + arena->write_marker;

	arena->write_marker += size;

	STATUS(magic_arena_validate(arena));
	return MAGIC_STATUS_OK;
}

int magic_arena_clear(Magic_Arena* arena)
{
	CHECK_PTR(arena);
	STATUS(magic_arena_validate(arena));

	arena->write_marker = 0;

	STATUS(magic_arena_validate(arena));

	return MAGIC_STATUS_OK;
}