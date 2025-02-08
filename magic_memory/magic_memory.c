#include "magic_memory.h"

#include "magic_error.h"

#include <malloc.h>
#include <stdbool.h>
#include <stdint.h>

int magic_allocate(Magic_Memory* memory, const uint32_t size)
{
	CHECK_PTR(memory);
	*memory = (Magic_Memory){ 0 };

	Magic_Memory temp_memory = { .data = NULL, .size = size, .allocated = 0, .dynamic = 1 };
	magic_malloc((void**)&temp_memory.data, &temp_memory.size, &temp_memory.allocated);

	CHECK(temp_memory.allocated <= temp_memory.size, MAGIC_ERROR_ALLOC_FAILED);
	CHECK(temp_memory.data != NULL, MAGIC_ERROR_ALLOC_FAILED);

	*memory = temp_memory;

	return MAGIC_STATUS_OK;
}

int magic_malloc(void** data, uint32_t* size, uint32_t* allocated)
{
	const uint32_t size_aligned = PAGE_SIZE * (((*size - 1) / PAGE_SIZE) + 1);
	*data = malloc(size_aligned);
	*allocated = *size = size_aligned;

	CHECK_PTR(*data);

	return MAGIC_STATUS_OK;
}

bool magic_memory_is_dynamic(const Magic_Memory* memory)
{
	return memory && (memory->dynamic || (memory->data == NULL && memory->size == 0 && memory->allocated == 0));
}