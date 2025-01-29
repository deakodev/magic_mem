#define MAGIC_ARENA_IMPL
#include <magic_arena.h>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

TEST_CASE("Magic_CreateArena")
{
	size_t capacity = 1024;
	Magic_Arena arena = Magic_CreateArena(capacity);

	CHECK(arena.Capacity == capacity);
	CHECK(arena.Memory != nullptr);
	CHECK(arena.Next == 0);

	free(arena.Memory);
}

TEST_CASE("Magic_Push")
{
	size_t capacity = 1024;
	Magic_Arena arena = Magic_CreateArena(capacity);

	SUBCASE("Allocate single item")
	{
		int* num_ptr = Magic_Push(&arena, int);
		REQUIRE(num_ptr != nullptr);
		*num_ptr = 42;
		CHECK(*num_ptr == 42);
	}

	SUBCASE("Allocate array of items")
	{
		size_t count = 10;
		int* array_ptr = Magic_PushArray(&arena, int, count);
		REQUIRE(array_ptr != nullptr);
		for (size_t i = 0; i < count; ++i)
		{
			array_ptr[i] = static_cast<int>(i);
		}
		for (size_t i = 0; i < count; ++i)
		{
			CHECK(array_ptr[i] == static_cast<int>(i));
		}
	}

	SUBCASE("Handle out of memory")
	{
		size_t large_size = capacity * 2;
		void* ptr = Magic_PushToArena(&arena, large_size, 1);
		CHECK(ptr == nullptr);
	}
}