#ifndef MAGIC_MEM_HEADER
#define MAGIC_MEM_HEADER

#include "magic_debug.h"

#include <stdbool.h>
#include <stdint.h>

#if !((defined(__STDC__) && __STDC__ == 1 && defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) || \
(defined(__cplusplus)) || (defined(_MSC_VER) && _MSC_VER >= 1600))
#error "mg_memory requires either C99, C++11, or MSVC!"
#endif

#if __cplusplus
extern "C" {
#endif

typedef uint32_t MgHandleType; // zero is reserved for invalid handle

typedef struct MgHandle {
    uint32_t slot_handle;
    MgHandleType type;
} MgHandle;

typedef struct MgHandleDescriptor {
    MgHandleType type;
    size_t count;
    uint32_t stride;
} MgHandleDescriptor;

typedef struct MgArenaDescriptor {
    const char* arena_name;
    MgHandleDescriptor* handle_descriptors;
    uint32_t handle_descriptors_count;
} MgArenaDescriptor;

#define MG_DEFINE_OPAQUE_HANDLE(object) typedef struct object##_T* object;
MG_DEFINE_OPAQUE_HANDLE(MgSegment);
MG_DEFINE_OPAQUE_HANDLE(MgBlock);
MG_DEFINE_OPAQUE_HANDLE(MgArena);

extern MgArena* mg_arena_init(MgArenaDescriptor* descriptor);
extern void mg_arena_destroy(MgArena** arena);

extern MgHandle mg_handle_create(MgArena* arena, uint32_t handle_type);
extern MgStatus mg_handle_write(MgArena* arena, MgHandle handle, const void* data, size_t data_size);
extern const void* mg_handle_read(MgArena* arena, MgHandle handle);
extern void mg_handle_erase(MgArena* arena, MgHandle handle);
extern bool mg_handle_valid(MgArena* arena, MgHandle handle);

extern void mg_arena_print(MgArena* arena);

#if __cplusplus
} // end extern "C"
#endif

#endif // MAGIC_MEM_HEADER
