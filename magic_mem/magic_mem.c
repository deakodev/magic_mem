#include "magic_mem.h"

#include <malloc.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define MG_ENCODE_HANDLE(index, generation) (((generation) << _MG_SLOT_BIT_SHIFT) | ((index) & _MG_SLOT_BIT_MASK))

#define MG_DECODE_INDEX(handle) ((handle) & _MG_SLOT_BIT_MASK)

#define MG_DECODE_GENERATION(handle) ((handle) >> _MG_SLOT_BIT_SHIFT)

enum {
    _MG_SLOT_BIT_SHIFT = 16,
    _MG_SLOT_BIT_MASK  = (1 << _MG_SLOT_BIT_SHIFT) - 1,
};

typedef enum _MgSlotStatus {
    _MG_SLOT_STATUS_FREE,
    _MG_SLOT_STATUS_VALID_ALLOC,
    _MG_SLOT_STATUS_VALID_WRITE,
    _MG_SLOT_STATUS_INVALID,
} _MgSlotStatus;

typedef struct _MgSlot {
    union {
        uint32_t handle;          // if allocated, stores generation + index
        uint32_t next_free_index; // if free, this points to next free slot in group
    };
    uint32_t generation;
    _MgSlotStatus status;
} _MgSlot;

typedef struct _MgGroup {
    uint8_t* data;
    _MgSlot* slots;
    uint32_t slot_count;
    uint32_t free_list_head;
    uint32_t handle_stride;
    uint32_t handle_type;
    size_t size;
} _MgGroup;

typedef struct _MgArena {
    _MgGroup* groups;
    uint32_t group_count;
    size_t alloc_size;
    const char* name;
} _MgArena;

static MgStatus _mg_group_init(_MgGroup* group, uintptr_t slots_start, MgHandleDescriptor* descriptor);
static _MgGroup* _mg_group_query(_MgArena* arena, uint32_t handle_type);
static uint32_t _mg_group_slot_alloc(_MgGroup* group);

MgArena* mg_arena_init(MgArenaDescriptor* descriptor)
{
    _MG_CHECK(descriptor, MG_ERROR_ARENA_DESC_INVALID);
    _MG_CHECK(descriptor->handle_descriptors && descriptor->handle_descriptors_count > 0, MG_ERROR_ARENA_DESC_INVALID);

    size_t alloc_size = sizeof(_MgArena) + (sizeof(_MgGroup) * descriptor->handle_descriptors_count); // arena + arena->groups

    for (uint32_t i = 0; i < descriptor->handle_descriptors_count; i++)
    {
        size_t handle_count  = descriptor->handle_descriptors[i].count + 1; // room for invalid slot 0
        size_t handle_stride = descriptor->handle_descriptors[i].stride;

        alloc_size += handle_count * sizeof(_MgSlot); // group->slots
        alloc_size += handle_count * handle_stride;   // group->data
    }

    _MgArena* arena_internal = (_MgArena*)malloc(alloc_size);
    _MG_CHECK(arena_internal, MG_ERROR_ARENA_ALLOC_FAILED);
    memset((void*)arena_internal, 0, alloc_size);

    arena_internal->name        = descriptor->arena_name;
    arena_internal->group_count = descriptor->handle_descriptors_count;
    arena_internal->alloc_size  = alloc_size;
    arena_internal->groups      = (_MgGroup*)(arena_internal + 1);

    uintptr_t slots_start = (uintptr_t)(arena_internal->groups + arena_internal->group_count);

    for (uint32_t i = 0; i < arena_internal->group_count; i++)
    {
        _MgGroup* group = &arena_internal->groups[i];
        _mg_group_init(group, slots_start, &descriptor->handle_descriptors[i]);
        slots_start += (uintptr_t)group->slot_count * sizeof(_MgSlot);      // move addr pass slots
        slots_start += (uintptr_t)group->slot_count * group->handle_stride; // move addr pass data
    }

    return (MgArena*)arena_internal;
}

MgHandle mg_handle_create(MgArena* arena, uint32_t handle_type)
{
    _MG_CHECK(arena, MG_ERROR_HANDLE_CREATION_FAILED);
    _MG_CHECK(handle_type > 0, MG_ERROR_HANDLE_INVALID);
    _MgArena* arena_internal = (_MgArena*)arena;

    _MgGroup* group = _mg_group_query(arena_internal, handle_type);
    if (group)
    {
        uint32_t slot_handle = _mg_group_slot_alloc(group);
        return (MgHandle){ slot_handle, handle_type };
    }

    _MG_CHECK(false, MG_ERROR_HANDLE_CREATION_FAILED);
    return (MgHandle){ 0, 0 }; // invalid
}

MgStatus mg_handle_write(MgArena* arena, MgHandle handle, const void* data, size_t data_size)
{
    _MG_STATUS(arena, MG_ERROR_ARENA_INVALID);
    _MG_STATUS(handle.slot_handle != MG_HANDLE_INVALID, MG_ERROR_HANDLE_INVALID);
    _MG_STATUS(data && data_size > 0, MG_ERROR_DATA_INVALID);
    _MgArena* arena_internal = (_MgArena*)arena;

    _MgGroup* group = _mg_group_query(arena_internal, handle.type);
    _MG_STATUS(group, MG_ERROR_GROUP_QUERY_FAILED);
    _MG_STATUS(data_size <= group->handle_stride, MG_ERROR_DATA_INVALID);

    uint32_t slot_index = MG_DECODE_INDEX(handle.slot_handle);
    _MgSlot* slot       = &group->slots[slot_index];
    _MG_STATUS(slot && slot->status == _MG_SLOT_STATUS_VALID_ALLOC, MG_ERROR_HANDLE_WRITE_FAILED);

    uint8_t* slot_data = group->data + (slot_index * group->handle_stride);
    memcpy((void*)slot_data, data, data_size);

    slot->status = _MG_SLOT_STATUS_VALID_WRITE;

    return MG_SUCCESS;
}

const void* mg_handle_read(MgArena* arena, MgHandle handle)
{
    _MG_CHECK(arena, MG_ERROR_ARENA_INVALID);
    _MG_CHECK(handle.slot_handle != MG_HANDLE_INVALID, MG_ERROR_HANDLE_INVALID);
    _MgArena* arena_internal = (_MgArena*)arena;

    _MgGroup* group = _mg_group_query(arena_internal, handle.type);
    _MG_CHECK(group, MG_ERROR_GROUP_QUERY_FAILED);

    uint32_t slot_index = MG_DECODE_INDEX(handle.slot_handle);
    _MgSlot* slot       = &group->slots[slot_index];
    _MG_CHECK(slot && (slot->status == _MG_SLOT_STATUS_VALID_WRITE), MG_ERROR_HANDLE_READ_FAILED);

    return (void*)(group->data + (slot_index * group->handle_stride));
}

void mg_handle_erase(MgArena* arena, MgHandle handle)
{
    _MG_CHECK(arena, MG_ERROR_ARENA_INVALID);
    _MgArena* arena_internal = (_MgArena*)arena;

    _MgGroup* group = _mg_group_query(arena_internal, handle.type);
    _MG_CHECK(group, MG_ERROR_GROUP_QUERY_FAILED);

    uint32_t slot_index = MG_DECODE_INDEX(handle.slot_handle);
    _MgSlot* slot       = &group->slots[slot_index];
    _MG_CHECK(slot && (slot->status == _MG_SLOT_STATUS_VALID_WRITE), MG_ERROR_HANDLE_ERASE_FAILED);

    slot->handle = 0;
    slot->status = _MG_SLOT_STATUS_FREE;

    memset((void*)(group->data + (slot_index * group->handle_stride)), 0, group->handle_stride);

    slot->next_free_index = group->free_list_head;
    group->free_list_head = slot_index;
}

bool mg_handle_valid(MgArena* arena, MgHandle handle)
{
    _MG_CHECK(arena, MG_ERROR_ARENA_INVALID);
    _MgArena* arena_internal = (_MgArena*)arena;

    bool valid = true;

    _MgGroup* group = _mg_group_query(arena_internal, handle.type);
    valid &= (group != NULL);

    if (valid)
    {
        uint32_t slot_index = MG_DECODE_INDEX(handle.slot_handle);
        valid &= (slot_index < group->slot_count);

        if (valid)
        {
            _MgSlot* slot = &group->slots[slot_index];
            valid &= (slot->status == _MG_SLOT_STATUS_VALID_WRITE);
            valid &= (slot->generation == MG_DECODE_GENERATION(handle.slot_handle));
        }
    }

    return valid;
}

void mg_print_arena_layout(MgArena* arena)
{
    _MG_CHECK(arena, MG_ERROR_ARENA_INVALID);
    _MgArena* arena_internal = (_MgArena*)arena;

    printf("=== Arena Layout ===\n");
    printf("Arena Name: %s\n", arena_internal->name);
    printf("Block Count: %u\n", arena_internal->group_count);
    printf("Arena Payload Size: %zu bytes\n", arena_internal->alloc_size);
    printf("Arena Header Address: %p\n", (void*)arena_internal);

    for (uint32_t i = 0; i < arena_internal->group_count; i++)
    {
        _MgGroup* group = &arena_internal->groups[i];

        printf("\n[[ Group %u ]]:", i);
        printf("Header Address: [0x%p]\n", (void*)group);
        printf("Slot Array Address: [0x%p]\n", (void*)group->slots);
        printf("Data Block Address: [0x%p]\n", (void*)group->data);
        printf("Total Group Size: (%zu bytes)\n", group->size);

        printf("Handle Desc: [type: %u, stride: %u]\n", group->handle_type, group->handle_stride);
        printf("Mutable State: [next slot index: %u]\n", group->free_list_head);
    }

    printf("===============\n\n");
}

static MgStatus _mg_group_init(_MgGroup* group, uintptr_t slots_start, MgHandleDescriptor* descriptor)
{
    _MG_STATUS(group && descriptor, MG_ERROR_GROUP_CREATION_FAILED);
    _MG_STATUS(descriptor->type > 0 && descriptor->count > 0 && descriptor->stride > 0, MG_ERROR_GROUP_CREATION_FAILED);

    size_t slot_count = descriptor->count + 1; // include invalid slot 0

    group->slots = (_MgSlot*)slots_start;
    group->data  = (uint8_t*)(slots_start + slot_count * sizeof(_MgSlot));

    group->slot_count    = (uint32_t)slot_count;
    group->handle_type   = descriptor->type;
    group->handle_stride = descriptor->stride;
    group->size          = slot_count * (sizeof(_MgSlot) + descriptor->stride);

    group->slots[0].handle     = 0; // invalid slot 0
    group->slots[0].generation = 0;
    group->slots[0].status     = _MG_SLOT_STATUS_INVALID;

    group->free_list_head = 1; // free list for slots 1..N

    for (uint32_t i = 1; i < slot_count; i++)
    {
        _MgSlot* slot    = &group->slots[i];
        slot->status     = _MG_SLOT_STATUS_FREE;
        slot->generation = 0;

        if (i < slot_count - 1)
        {
            slot->next_free_index = i + 1;
        }
        else
        {
            slot->next_free_index = 0; // last slot
        }
    }

    return MG_SUCCESS;
}

static _MgGroup* _mg_group_query(_MgArena* arena_internal, uint32_t handle_type)
{
    _MG_CHECK(arena_internal, MG_ERROR_ARENA_INVALID);
    _MG_CHECK(handle_type > 0 && handle_type <= arena_internal->group_count, MG_ERROR_HANDLE_TYPE_INVALID);

    for (uint32_t i = 0; i < arena_internal->group_count; i++)
    {
        if (arena_internal->groups[i].handle_type == handle_type)
        {
            return &arena_internal->groups[i];
        }
    }

    _MG_CHECK(false, MG_ERROR_GROUP_QUERY_FAILED);
    return NULL;
}

static uint32_t _mg_group_slot_alloc(_MgGroup* group)
{
    _MG_CHECK(group->free_list_head != 0, MG_ERROR_GROUP_EXHAUSTED);

    uint32_t slot_index = group->free_list_head;
    _MgSlot* slot       = &group->slots[slot_index];
    _MG_CHECK(slot->status == _MG_SLOT_STATUS_FREE, MG_ERROR_GROUP_SLOT_ALLOC_FAILED);

    group->free_list_head = slot->handle; // advance to next free slot

    uint32_t slot_generation = ++(slot->generation);
    slot->handle             = MG_ENCODE_HANDLE(slot_index, slot_generation);
    slot->status             = _MG_SLOT_STATUS_VALID_ALLOC;

    return slot->handle;
}