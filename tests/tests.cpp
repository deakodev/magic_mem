#include <magic_memory.h>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <iostream>

TEST_SUITE("magic_arena")
{
	int status;

	Magic_Arena arena;
	Magic_Arena* arena_ptr = &arena;

	int data = 7;
	int* data_ptr = &data;
	int oversized_data_array[PAGE_SIZE + 1] = { 0 };

	TEST_CASE("magic_arena_allocate")
	{
		status = magic_arena_allocate(NULL, PAGE_SIZE);
		CHECK(status != MAGIC_STATUS_OK);
		status = magic_arena_allocate(arena_ptr, 0); /* alloc size sets to min PAGE_SIZE */
		CHECK(status == MAGIC_STATUS_OK);
		status = magic_arena_allocate(arena_ptr, PAGE_SIZE);
		CHECK(status == MAGIC_STATUS_OK);
	}

	TEST_CASE("magic_arena_write")
	{
		status = magic_arena_write(arena_ptr, NULL, sizeof(data));
		CHECK(status != MAGIC_STATUS_OK);
		status = magic_arena_write(arena_ptr, (void**)data_ptr, 0);
		CHECK(status != MAGIC_STATUS_OK);
		status = magic_arena_write(arena_ptr, (void**)oversized_data_array, sizeof(oversized_data_array));
		CHECK(status != MAGIC_STATUS_OK);
		status = magic_arena_write(arena_ptr, (void**)&data_ptr, sizeof(data));
		CHECK(status == MAGIC_STATUS_OK);
		CHECK(*data_ptr == 7);
	}
}

TEST_SUITE("magic_ring")
{
	int status;

	Magic_Ring ring;
	Magic_Ring* ring_ptr = &ring;

	int write_data = 7;
	int* write_data_ptr = &write_data;

	int read_data;
	int* read_data_ptr = &read_data;

	int oversized_data_array[PAGE_SIZE + 1] = { 0 };

	TEST_CASE("magic_ring_allocate")
	{
		status = magic_ring_allocate(NULL, PAGE_SIZE);
		CHECK(status != MAGIC_STATUS_OK);
		status = magic_ring_allocate(ring_ptr, 0); /* alloc size sets to min PAGE_SIZE */
		CHECK(status == MAGIC_STATUS_OK);
		status = magic_ring_allocate(ring_ptr, PAGE_SIZE);
		CHECK(status == MAGIC_STATUS_OK);
	}
	TEST_CASE("magic_ring_write")
	{
		status = magic_ring_write(NULL, (void*)write_data_ptr, sizeof(write_data));
		CHECK(status != MAGIC_STATUS_OK);
		status = magic_ring_write(ring_ptr, NULL, sizeof(write_data));
		CHECK(status != MAGIC_STATUS_OK);
		status = magic_ring_write(ring_ptr, (void*)write_data_ptr, 0);
		CHECK(status != MAGIC_STATUS_OK);
		status = magic_ring_write(ring_ptr, (void*)oversized_data_array, sizeof(oversized_data_array));
		CHECK(status != MAGIC_STATUS_OK);
		status = magic_ring_write(ring_ptr, (void*)write_data_ptr, sizeof(write_data));
		CHECK(status == MAGIC_STATUS_OK);
		CHECK(*write_data_ptr == 7);
	}

	TEST_CASE("magic_ring_read")
	{
		status = magic_ring_read(NULL, (void*)read_data_ptr, sizeof(read_data));
		CHECK(status != MAGIC_STATUS_OK);
		status = magic_ring_read(ring_ptr, NULL, sizeof(read_data));
		CHECK(status != MAGIC_STATUS_OK);
		status = magic_ring_read(ring_ptr, (void*)read_data_ptr, 0);
		CHECK(status != MAGIC_STATUS_OK);
		status = magic_ring_read(ring_ptr, (void*)oversized_data_array, sizeof(oversized_data_array));
		CHECK(status != MAGIC_STATUS_OK);
		status = magic_ring_read(ring_ptr, (void*)read_data_ptr, sizeof(read_data));
		CHECK(status == MAGIC_STATUS_OK);
		CHECK(*read_data_ptr == 7);
	}
}

