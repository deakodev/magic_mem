#include "magic_memory.h"

#include "magic_error.h"

#include <malloc.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int magic_ring_allocate(Magic_Ring* ring, const uint32_t size)
{
	CHECK_PTR(ring);
	*ring = (Magic_Ring){ 0 };

	STATUS(magic_allocate(&ring->memory, size));

	ring->read_marker = 0;
	ring->write_marker = 0;
	ring->dynamic = false;
	ring->allocated = true;

	return MAGIC_STATUS_OK;
}

int magic_ring_validate(const Magic_Ring* ring)
{
	CHECK_PTR(ring);
	const Magic_Memory* memory = &ring->memory;

	CHECK_PTR(memory);
	CHECK(memory->data != NULL || memory->size == 0, MAGIC_ERROR_RING_INVALID);
	CHECK(memory->data != NULL || memory->allocated == 0, MAGIC_ERROR_RING_INVALID);
	CHECK(memory->dynamic || memory->allocated == 0, MAGIC_ERROR_RING_INVALID);
	CHECK(!memory->dynamic || memory->size <= memory->allocated, MAGIC_ERROR_RING_INVALID);

	return MAGIC_STATUS_OK;
}

int magic_ring_write(Magic_Ring* ring, const void* data, uint32_t size)
{
	CHECK_PTR(ring);
	CHECK_PTR(data);
	STATUS(magic_ring_validate(ring));

	// TODO: handle overflow
	// TODO: alignment check?

	CHECK(size > 0, MAGIC_ERROR_WRITE_FAILED);

	uint32_t available_space = (ring->write_marker >= ring->read_marker)
		? (ring->memory.size - (ring->write_marker - ring->read_marker))
		: (ring->read_marker - ring->write_marker);

	CHECK(size < available_space, MAGIC_ERROR_WRITE_FAILED);

	uint32_t first_chunk = (ring->memory.size - ring->write_marker);
	if (first_chunk > size) first_chunk = size;
	uint32_t second_chunk = size - first_chunk;

	memcpy(ring->memory.data + ring->write_marker, data, first_chunk);
	if (second_chunk > 0)
	{
		memcpy(ring->memory.data, (uint8_t*)data + first_chunk, second_chunk);
	}

	STATUS(magic_ring_write_next(ring, size));

	STATUS(magic_ring_validate(ring));

	return MAGIC_STATUS_OK;
}


int magic_ring_write_next(Magic_Ring* ring, const uint32_t size)
{
	STATUS(magic_ring_validate(ring));

	ring->write_marker = (ring->write_marker + size) % ring->memory.size;

	STATUS(magic_ring_validate(ring));

	return MAGIC_STATUS_OK;
}

int magic_ring_read(Magic_Ring* ring, void* out, uint32_t size)
{
	CHECK_PTR(ring);
	CHECK_PTR(out);
	STATUS(magic_ring_validate(ring));

	CHECK(size > 0, MAGIC_ERROR_READ_FAILED);

	uint32_t unread_data = (ring->write_marker >= ring->read_marker)
		? (ring->write_marker - ring->read_marker)
		: (ring->memory.size - ring->read_marker + ring->write_marker);

	CHECK(size <= unread_data, MAGIC_ERROR_READ_FAILED);

	uint32_t first_chunk = (ring->memory.size - ring->read_marker);
	if (first_chunk > size) first_chunk = size;
	uint32_t second_chunk = size - first_chunk;

	memcpy(out, ring->memory.data + ring->read_marker, first_chunk);
	if (second_chunk > 0)
	{
		memcpy((uint8_t*)out + first_chunk, ring->memory.data, second_chunk);
	}

	STATUS(magic_ring_read_next(ring, size));

	STATUS(magic_ring_validate(ring));

	return MAGIC_STATUS_OK;
}


int magic_ring_read_next(Magic_Ring* ring, const uint32_t size)
{
	STATUS(magic_ring_validate(ring));

	ring->read_marker = (ring->read_marker + size) % ring->memory.size;

	STATUS(magic_ring_validate(ring));

	return MAGIC_STATUS_OK;
}