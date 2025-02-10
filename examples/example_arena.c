#include <magic_memory.h>

#include <stdint.h>
#include <stdio.h>

//#define MAGIC_ARENA_EXAMPLE
#ifdef MAGIC_ARENA_EXAMPLE

#define BUFFER_SIZE 64

typedef struct {
	uint32_t x;
	uint32_t y;
	uint32_t z;
	uint32_t w;
} Test;

int main(void)
{
	Magic_Arena arena;
	int status = magic_arena_allocate(&arena, BUFFER_SIZE);
	if (status < 0)
	{
		printf("Failed to allocate arena.\n");
		return status;
	}

	Test test[4] = { 0 };
	Test *test_ptr = test;
	status = magic_arena_write(&arena, (void**)&test_ptr, sizeof(Test) * 4);
	if (status < 0)
	{
		printf("Failed to write arena.\n");
		return status;
	}

	test->x = 1;
	test->y = 2;
	test->z = 3;
	test->w = 4;

	printf("Test: %d %d %d %d\n", test->x, test->y, test->z, test->w);

	return 0;
}

#endif // MAGIC_ARENA_EXAMPLE
