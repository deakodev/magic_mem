#include "magic_mem.h"

#include <malloc.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef uint8_t _MgSlot;

typedef struct _MgGroup {
    _MgSlot* slots;
    _MgSlot* free_slots;
    uint32_t write_marker;
    uint32_t end_marker;
    MgHandleType handle_type;
    uint32_t handle_stride;
    size_t max_handles;
} _MgGroup;

typedef struct _MgArena {
    _MgGroup* groups;
    uint32_t group_count;
    size_t alloc_size;
    const char* name;
} _MgArena;

static MgStatus _mg_group_create(_MgGroup* group, MgHandleDescriptor* descriptor);

MgStatus mg_arena_create(MgArena* arena, MgArenaDescriptor* descriptor)
{
    MG_CHECK(arena, MG_ERROR_ARENA_INVALID);
    MG_CHECK(descriptor && descriptor->handle_descriptors && descriptor->handle_descriptors_count > 0, MG_ERROR_ARENA_DESC_INVALID);

    size_t alloc_size = sizeof(_MgArena);
    for (uint32_t i = 0; i < descriptor->handle_descriptors_count; i++)
    {
        alloc_size += sizeof(_MgGroup);
        alloc_size += descriptor->handle_descriptors[i].stride * descriptor->handle_descriptors[i].count;
    }

    _MgArena* arena_internal = (_MgArena*)malloc(alloc_size);

    if (!arena_internal)
    {
        return MG_ERROR_ARENA_ALLOC_FAILED;
    }

    arena_internal->name        = descriptor->arena_name;
    arena_internal->group_count = descriptor->handle_descriptors_count;
    arena_internal->alloc_size  = alloc_size;

    uintptr_t arena_start  = (uintptr_t)arena_internal;
    size_t group_offset    = sizeof(_MgArena);
    arena_internal->groups = (_MgGroup*)(arena_start + group_offset);

    for (uint32_t i = 0; i < descriptor->handle_descriptors_count; i++)
    {
        _MgGroup* group = (_MgGroup*)(arena_start + group_offset);
        _mg_group_create(group, &descriptor->handle_descriptors[i]);
        group_offset += group->end_marker;
    }

    *arena = (MgArena)arena_internal;

    return MG_SUCCESS;
}

MgHandle mg_handle_create(MgArena* arena, uint32_t handle_type)
{
    MG_CHECK(arena && *arena, MG_ERROR_ARENA_INVALID);
    _MgArena* arena_internal = (_MgArena*)*arena;
    MG_CHECK(handle_type <= arena_internal->group_count, MG_ERROR_HANDLE_INVALID);

    uint32_t group_offset = 0;
    for (uint32_t i = 0; i < arena_internal->group_count; i++)
    {
        _MgGroup* group = (_MgGroup*)((uintptr_t)arena_internal->groups + group_offset);
        if (group->handle_type == handle_type)
        {
            if (group->free_slots)
            {
                return (MgHandle){ (uint32_t)(uintptr_t)group->free_slots, handle_type }; // TODO: implement free list
            }

            size_t allocated_handles = group->write_marker / group->handle_stride; // TODO: think about
            if (allocated_handles < group->max_handles)
            {
                MgHandle handle = { (uint32_t)group->write_marker, handle_type };
                group->write_marker += group->handle_stride;
                return handle;
            }
        }

        group_offset += group->end_marker;
    }

    ERROR_PRINT(MG_ERROR_HANDLE_CREATION_FAILED);
    return (MgHandle){ 0, 0 }; // invalid handle
}

MgStatus mg_handle_write(MgArena* arena, MgHandle handle, const void* data, size_t data_size)
{
    MG_CHECK(arena && *arena, MG_ERROR_ARENA_INVALID);
    MG_CHECK(data && data_size > 0, MG_ERROR_DATA_INVALID);
    _MgArena* arena_internal = (_MgArena*)*arena;

    uint32_t group_offset = 0;
    for (uint32_t i = 0; i < arena_internal->group_count; i++)
    {
        _MgGroup* group = (_MgGroup*)((uintptr_t)arena_internal->groups + group_offset);
        if (handle.type == group->handle_type && data_size == group->handle_stride)
        {
            _MgSlot* handle_slot = (_MgSlot*)group->slots + handle.offset;
            memcpy((void*)handle_slot, data, data_size);
            return MG_SUCCESS;
        }

        group_offset += group->end_marker;
    }

    return MG_ERROR_HANDLE_WRITE_FAILED;
}

const void* mg_handle_read(MgArena* arena, MgHandle handle)
{
    MG_CHECK(arena && *arena, MG_ERROR_ARENA_INVALID);
    _MgArena* arena_internal = (_MgArena*)*arena;

    size_t group_offset = 0;
    for (uint32_t i = 0; i < arena_internal->group_count; i++)
    {
        _MgGroup* group = (_MgGroup*)((uintptr_t)arena_internal->groups + group_offset);
        if (handle.type == group->handle_type && handle.offset % group->handle_stride == 0)
        {
            return (void*)((uintptr_t)group->slots + handle.offset);
        }

        group_offset += group->end_marker;
    }

    ERROR_PRINT(MG_ERROR_HANDLE_READ_FAILED);
    return NULL;
}

static MgStatus _mg_group_create(_MgGroup* group, MgHandleDescriptor* descriptor)
{
    MG_CHECK(group && descriptor, MG_ERROR_GROUP_CREATION_FAILED);

    uintptr_t group_start    = (uintptr_t)group;
    size_t group_header_size = sizeof(_MgGroup);
    size_t group_write_size  = descriptor->stride * descriptor->count;
    uint32_t group_end       = (uint32_t)(group_header_size + group_write_size);

    group->slots         = (_MgSlot*)(group_start + group_header_size);
    group->free_slots    = NULL;
    group->write_marker  = 0;
    group->end_marker    = group_end;
    group->max_handles   = descriptor->count;
    group->handle_type   = descriptor->type;
    group->handle_stride = descriptor->stride;

    return MG_SUCCESS;
}

void mg_print_arena_layout(MgArena arena)
{
    if (!arena)
    {
        printf("\nArena is NULL!\n");
        return;
    }

    _MgArena* internal = (_MgArena*)arena;
    printf("=== Arena Layout ===\n");
    printf("Arena Name: %s\n", internal->name);
    printf("Block Count: %u\n", internal->group_count);
    printf("Arena Payload Size: %zu bytes\n", internal->alloc_size);
    printf("Arena Header Address: %p\n", (void*)internal);

    uintptr_t arena_start = (uintptr_t)internal;
    size_t offset         = sizeof(_MgArena);

    for (uint32_t i = 1; i < internal->group_count + 1; i++)
    {
        _MgGroup* group = (_MgGroup*)(arena_start + offset);

        printf("\n[[ Group %u ]]:", i);
        printf(" [0x%p] -> [0x%p]", (void*)group, (void*)((uintptr_t)group + group->end_marker));
        printf(" (%u bytes)\n", group->end_marker);

        printf("Handle Desc: [type: %u, stride: %u]\n", group->handle_type, group->handle_stride);
        printf("Mutable State: [write marker: %u]\n", group->write_marker);

        offset += group->end_marker;
    }

    printf("=== End of Arena Layout ===\n\n");
}