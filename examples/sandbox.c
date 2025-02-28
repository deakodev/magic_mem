#include "magic_mem.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define HANDLE_LIMIT 32

typedef struct UserString {
    char data[32];
    size_t length;
} UserString;

typedef uint64_t Array[10];
typedef Array UserArray;

typedef struct UserStringHandle {
    MgHandle mg_handle;
} UserStringHandle;

typedef struct UserArrayHandle {
    MgHandle mg_handle;
} UserArrayHandle;

typedef enum UserHandleType {
    USER_HANDLE_TYPE_INVALID = 0, // always invalid
    USER_HANDLE_TYPE_STRING  = 1,
    USER_HANDLE_TYPE_ARRAY   = 2,
} UserHandleType;

static MgHandleDescriptor handle_descriptors[] = {
    { .type = USER_HANDLE_TYPE_STRING, .stride = sizeof(UserString), .count = HANDLE_LIMIT },
    { .type = USER_HANDLE_TYPE_ARRAY, .stride = sizeof(UserArray), .count = HANDLE_LIMIT },
};

static MgArenaDescriptor arena_descriptor = {
    .arena_name               = "USER_ARENA",
    .handle_descriptors       = handle_descriptors,
    .handle_descriptors_count = sizeof(handle_descriptors) / sizeof(MgHandleDescriptor),
};

static void safer_string_print(MgArena* arena, UserStringHandle handle);
static void safer_array_print(MgArena* arena, UserArrayHandle handle);

int main(void)
{
    MgStatus status;

    MgArena* arena = mg_arena_init(&arena_descriptor);

    mg_print_arena_layout(arena);

    /////////////////////////////////////////////////
    // Vanilla pattern //////////////////////////////
    /////////////////////////////////////////////////

    UserString string_to_allocate = { "Hello, World!", 12 };
    UserArray array_to_allocate   = { 420, 2, 27, 20, 74, 40, 65, 290, 39, 203 };

    MgHandle string_handle = mg_handle_create(arena, USER_HANDLE_TYPE_STRING);
    MgHandle array_handle  = mg_handle_create(arena, USER_HANDLE_TYPE_ARRAY);

    status = mg_handle_write(arena, string_handle, &string_to_allocate, sizeof(UserString));
    MG_STATUS(status);

    status = mg_handle_write(arena, array_handle, &array_to_allocate, sizeof(UserArray));
    MG_STATUS(status);

    // Important tip -> memory safety == only read handle contents when absolutely necessary and within a finite scope (don't pass read values between functions)
    UserString retrived_string = *(UserString*)mg_handle_read(arena, string_handle);
    UserArray retrived_array;
    memcpy(retrived_array, mg_handle_read(arena, array_handle), sizeof(UserArray));

    printf("Retrived string: %s\n", retrived_string.data);
    printf("Retrived array: [ ");
    for (int i = 0; i < 9; i++)
    {
        printf("%llu ", retrived_array[i]);
    }
    printf("]\n\n");

    mg_print_arena_layout(arena);

    /////////////////////////////////////////////////
    // Safer pattern ////////////////////////////////
    /////////////////////////////////////////////////

    UserString string_to_allocate2 = { "Hello, Safer World!", 18 };
    UserArray array_to_allocate2   = { 543, 6, 278, 40, 94, 42, 17, 222, 58, 305 };

    // Using user-defined wrapper handles for stronger type safety
    // (so array_handle can't be passed to function expecting a string_handle)
    UserStringHandle user_string_handle = { mg_handle_create(arena, USER_HANDLE_TYPE_STRING) };
    UserArrayHandle user_array_handle   = { mg_handle_create(arena, USER_HANDLE_TYPE_ARRAY) };

    status = mg_handle_write(arena, user_string_handle.mg_handle, &string_to_allocate2, sizeof(UserString));
    MG_STATUS(status);

    status = mg_handle_write(arena, user_array_handle.mg_handle, &array_to_allocate2, sizeof(UserArray));
    MG_STATUS(status);

    // Now we can safely pass the correct handles to functions that expect them
    safer_string_print(arena, user_string_handle);
    safer_array_print(arena, user_array_handle);

    mg_print_arena_layout(arena);

    /////////////////////////////////////////////////
    // How many handles can we make? ////////////////
    /////////////////////////////////////////////////

    UserStringHandle handles[31];

    for (int i = 0; i < 30; i++)
    {
        handles[i] = (UserStringHandle){ mg_handle_create(arena, USER_HANDLE_TYPE_STRING) };
        if (handles[i].mg_handle.slot_handle == MG_HANDLE_INVALID)
        {
            printf("Failed to create handle\n");
            return MG_ERROR_HANDLE_CREATION_FAILED;
        }
        status = mg_handle_write(arena, handles[i].mg_handle, &string_to_allocate, sizeof(UserString));
        if (status != MG_SUCCESS)
        {
            printf("Failed to write data\n");
            return status;
        }

        mg_print_arena_layout(arena);
    }

    mg_handle_erase(arena, user_string_handle.mg_handle);

    UserString string_to_allocate3       = { "Hello, Final Handle!", 20 };
    UserStringHandle user_string_handle2 = { mg_handle_create(arena, USER_HANDLE_TYPE_STRING) };
    status = mg_handle_write(arena, user_string_handle.mg_handle, &string_to_allocate3, sizeof(UserString));
    MG_STATUS(status);

    mg_print_arena_layout(arena);

    if (mg_handle_valid(arena, user_string_handle.mg_handle))
    {
        printf("Handle1:\n");
        safer_string_print(arena, user_string_handle);
    }

    if (mg_handle_valid(arena, user_string_handle2.mg_handle))
    {
        printf("Handle2:\n");
        safer_string_print(arena, user_string_handle2);
    }

} // end of main

static void safer_string_print(MgArena* arena, UserStringHandle user_handle)
{
    UserString* retrived_string = (UserString*)mg_handle_read(arena, user_handle.mg_handle);
    printf("Retrived safer string: %s\n", retrived_string->data);
}

static void safer_array_print(MgArena* arena, UserArrayHandle user_handle)
{
    UserArray retrived_array;
    memcpy(retrived_array, mg_handle_read(arena, user_handle.mg_handle), sizeof(UserArray));
    printf("Retrived safer array: [ ");
    for (int i = 0; i < 9; i++)
    {
        printf("%llu ", retrived_array[i]);
    }
    printf("]\n");
}