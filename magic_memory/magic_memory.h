#ifndef MAGIC_MEMORY_HEADER
#define MAGIC_MEMORY_HEADER

#include "magic_error.h"

#include <stdint.h>
#include <stdbool.h>
#include <vadefs.h>

#if !( \
    (defined(__STDC__) && __STDC__ == 1 && defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) || \
    (defined(__cplusplus)) || \
    (defined(_MSC_VER) && _MSC_VER >= 1600) \
)
#error "magic_memory requires either C99, C++11, or MSVC!"
#endif

#if __cplusplus
extern "C" {
#endif

#define PAGE_SIZE 4096

	/* memory for allocated magic memory objects */
	typedef struct {
		uint8_t* data; /* refernce to data in memory */
		uint32_t size; /* size of the data */
		uint32_t allocated; /* amount of memory allocated, zero if not allocated */
		bool dynamic; /* whether memory can be resized */
	} Magic_Memory;

	/* serial arena memory allocator */
	typedef struct {
		Magic_Memory memory; /* memory for the arena */
		uint32_t write_marker; /* position in memory to write to */
		uint32_t end_marker; /* end of the memory */
		bool dynamic; /* whether the memory can be resized */
		bool allocated; /* whether arena has been allocated */
	} Magic_Arena;

	/* circular ring memory allocator */
	typedef struct {
		Magic_Memory memory; /* memory for the arena */
		uint32_t read_marker; /* position in memory to read from */
		uint32_t write_marker; /* position in memory to write to */
		bool dynamic; /* whether the memory can be resized */
		bool allocated; /* whether ring has been allocated */
	} Magic_Ring;

	/* memory functions */
	extern int magic_allocate(Magic_Memory* memory, const uint32_t size);
	extern int magic_malloc(void** data, uint32_t* size, uint32_t* allocated);
	extern bool magic_memory_is_dynamic(const Magic_Memory* memory);

	/* arena functions */
	extern int magic_arena_allocate(Magic_Arena* arena, const uint32_t size);
	extern int magic_arena_validate(const Magic_Arena* arena);
	extern int magic_arena_write(Magic_Arena* arena, void** data, uint32_t size);
	extern int magic_arena_clear(Magic_Arena* arena);

	/* ring functions */
	extern int magic_ring_allocate(Magic_Ring* ring, const uint32_t size);
	extern int magic_ring_validate(const Magic_Ring* arena);
	extern int magic_ring_write(Magic_Ring* arena, const void* data, uint32_t size);
	extern int magic_ring_write_next(Magic_Ring* arena, const uint32_t size);
	extern int magic_ring_read(Magic_Ring* arena, void* data, uint32_t size);
	extern int magic_ring_read_next(Magic_Ring* arena, const uint32_t size);

#if __cplusplus
} // end extern "C"
#endif

#endif // MAGIC_MEMORY_HEADER
