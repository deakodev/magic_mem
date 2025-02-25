#include "magic_mem.h"

#include <malloc.h>
#include <stdint.h>
#include <string.h>

#ifdef DEBUG
#include <assert.h>
#define MG_ASSERT(x) assert(x)
#else
#define MG_ASSERT(x) ((void)0)
#endif

typedef uint8_t _MgSlot;

typedef struct _MgSegment {
    _MgSlot* slots;
    _MgSlot* free_slots;
    uint32_t write_marker;
    uint32_t end_marker;
    size_t max_handles;
} _MgSegment;

typedef struct _MgGroup {
    _MgSegment* segments;
    uint32_t segment_count;
    uint32_t handle_type;
    uint32_t handle_stride;
} _MgGroup;

typedef struct _MgArena {
    _MgGroup* groups;
    uint32_t group_count;
    size_t alloc_size;
    const char* name;
} _MgArena;

static MgStatus _mg_group_create(_MgGroup* group, MgHandleDescriptor* descriptor);
static uint32_t _mg_group_offset(_MgGroup* group);

MgStatus mg_arena_create(MgArena* arena, MgArenaDescriptor* descriptor)
{
    MG_ASSERT(arena && descriptor);
    MG_ASSERT(descriptor->handle_descriptors && descriptor->handle_descriptors_count > 0);

    size_t alloc_size = sizeof(_MgArena);
    for (uint32_t i = 0; i < descriptor->handle_descriptors_count; i++)
    {
        alloc_size += sizeof(_MgGroup) + sizeof(_MgSegment);
        alloc_size += descriptor->handle_descriptors[i].stride * descriptor->handle_descriptors[i].count;
    }

    _MgArena* arena_internal = (_MgArena*)malloc(alloc_size);
    MG_ASSERT(arena_internal);

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
        group_offset += _mg_group_offset(group);
    }

    *arena = (MgArena)arena_internal;

    return MG_SUCCESS;
}


MgHandle mg_handle_create(MgArena* arena, uint32_t handle_type)
{
    MG_ASSERT(arena && *arena);
    _MgArena* arena_internal = (_MgArena*)*arena;
    MG_ASSERT(handle_type <= arena_internal->group_count);

    uint32_t group_offset = 0;
    for (uint32_t i = 0; i < arena_internal->group_count; i++)
    {
        _MgGroup* group = (_MgGroup*)((uintptr_t)arena_internal->groups + group_offset);
        if (group->handle_type == handle_type)
        {
            for (uint32_t j = 0; j < group->segment_count; j++)
            {
                _MgSegment* segment = &group->segments[j];
                if (segment->free_slots)
                {
                    return (MgHandle){ (uint32_t)(uintptr_t)segment->free_slots, handle_type }; // TODO: implement free list
                }

                size_t allocated_handles = segment->write_marker / group->handle_stride;
                if (allocated_handles < segment->max_handles)
                {
                    MgHandle handle = { (uint32_t)segment->write_marker, handle_type };
                    segment->write_marker += group->handle_stride;
                    return handle;
                }
            }
        }

        group_offset += _mg_group_offset(group);
    }

    return (MgHandle){ 0, 0 }; // invalid handle
}

MgStatus mg_handle_write(MgArena* arena, MgHandle handle, const void* data, size_t data_size)
{
    MG_ASSERT(arena && *arena);
    MG_ASSERT(data && data_size > 0);
    _MgArena* arena_internal = (_MgArena*)*arena;

    uint32_t group_offset = 0;
    for (uint32_t i = 0; i < arena_internal->group_count; i++)
    {
        _MgGroup* group = (_MgGroup*)((uintptr_t)arena_internal->groups + group_offset);
        if (handle.type == group->handle_type && data_size == group->handle_stride)
        {
            for (uint32_t j = 0; j < group->segment_count; j++)
            {
                _MgSegment* segment  = &group->segments[j];
                _MgSlot* handle_slot = (_MgSlot*)segment->slots + handle.offset;
                memcpy((void*)handle_slot, data, data_size);

                return MG_SUCCESS;
            }
        }

        group_offset += _mg_group_offset(group);
    }

    return MG_FAILURE;
}

const void* mg_handle_read(MgArena* arena, MgHandle handle)
{
    MG_ASSERT(arena && *arena);
    _MgArena* arena_internal = (_MgArena*)*arena;

    size_t group_offset = 0;
    for (uint32_t i = 0; i < arena_internal->group_count; i++)
    {
        _MgGroup* group = (_MgGroup*)((uintptr_t)arena_internal->groups + group_offset);
        if (handle.type == group->handle_type)
        {
            for (uint32_t j = 0; j < group->segment_count; j++)
            {
                _MgSegment* segment = &group->segments[j];
                if (handle.offset % group->handle_stride == 0)
                {
                    return (void*)((uintptr_t)segment->slots + handle.offset);
                }
            }
        }

        group_offset += _mg_group_offset(group);
    }

    return NULL;
}

static MgStatus _mg_group_create(_MgGroup* group, MgHandleDescriptor* descriptor)
{
    MG_ASSERT(group && descriptor);

    size_t group_header_size   = sizeof(_MgGroup);
    size_t segment_header_size = sizeof(_MgSegment);
    size_t segment_write_size  = descriptor->stride * descriptor->count;

    uintptr_t segment_start = (uintptr_t)group + group_header_size;
    uint32_t segment_end    = (uint32_t)(segment_header_size + segment_write_size);

    group->segments = (_MgSegment*)segment_start;

    group->segments->slots        = (_MgSlot*)(segment_start + segment_header_size);
    group->segments->free_slots   = NULL;
    group->segments->write_marker = 0;
    group->segments->end_marker   = segment_end;
    group->segments->max_handles  = descriptor->count;


    group->segment_count = 1;
    group->handle_type   = descriptor->type;
    group->handle_stride = descriptor->stride;

    return MG_SUCCESS;
}

static uint32_t _mg_group_offset(_MgGroup* group)
{
    uint32_t offset = sizeof(_MgGroup);
    for (uint32_t i = 0; i < group->segment_count; i++)
    {
        offset += group->segments[i].end_marker;
    }

    MG_ASSERT(offset <= UINT32_MAX);
    return offset;
}

void mg_print_arena_layout(MgArena arena)
{
    if (!arena)
    {
        printf("Arena is NULL!\n");
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

    for (uint32_t i = 0; i < internal->group_count; i++)
    {
        _MgGroup* group = (_MgGroup*)(arena_start + offset);

        printf("\n--- Group %u ---\n", i);
        printf("Group Address: %p\n", (void*)group);
        printf("Group Size: %u bytes\n", _mg_group_offset(group));

        printf("  Handle Type: %du\n", group->handle_type);
        printf("  Handle Stride: %u\n", group->handle_stride);
        printf("  Segment Count: %u\n", group->segment_count);

        if (group->segment_count > 0 && group->segments)
        {
            _MgSegment* seg = group->segments;
            printf("  Segment 0 Address: %p\n", (void*)seg);
            printf("    Write Marker: %u\n", seg->write_marker);
            printf("    End Marker: %u\n", seg->end_marker);
            printf("    Slots Pointer: %p\n", (void*)seg->slots);
            printf("    Free Slots: %p\n", seg->free_slots);
        }
        else
        {
            printf("  No segments present.\n");
        }

        offset += _mg_group_offset(group);
    }

    printf("=== End of Arena Layout ===\n\n");
}