#include <magic_mem.h>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

#define HANDLE_LIMIT 32

typedef struct UserString {
    char data[32];
} UserString;

typedef uint64_t UserArray[100];

typedef enum UserHandleType {
    USER_HANDLE_TYPE_INVALID = 0, // always invalid
    USER_HANDLE_TYPE_STRING  = 1,
    USER_HANDLE_TYPE_ARRAY   = 2,
} UserHandleType;

static MgHandleDescriptor handle_descriptors[] = {
    { .type = USER_HANDLE_TYPE_STRING, .count = HANDLE_LIMIT, .stride = sizeof(UserString) },
    { .type = USER_HANDLE_TYPE_ARRAY, .count = HANDLE_LIMIT, .stride = sizeof(UserArray) },
};

static MgArenaDescriptor arena_descriptor = {
    .arena_name               = "USER_ARENA",
    .handle_descriptors       = handle_descriptors,
    .handle_descriptors_count = sizeof(handle_descriptors) / sizeof(MgHandleDescriptor),
};

TEST_SUITE("mg_arena_create")
{
    TEST_CASE("Passing valid handles")
    {
        MgArena arena;
        MgStatus status = mg_arena_create(&arena, &arena_descriptor);
        REQUIRE(status == MG_SUCCESS);

        MgHandle string_handle = mg_handle_create(&arena, USER_HANDLE_TYPE_STRING);
        CHECK(string_handle.type != USER_HANDLE_TYPE_INVALID);

        MgHandle array_handle = mg_handle_create(&arena, USER_HANDLE_TYPE_ARRAY);
        CHECK(array_handle.type != USER_HANDLE_TYPE_INVALID);
    }

    TEST_CASE("Passing invalid handles")
    {
        MgArena arena;
        MgStatus status = mg_arena_create(&arena, &arena_descriptor);
        REQUIRE(status == MG_SUCCESS);

        MgHandle invalid_handle = mg_handle_create(&arena, USER_HANDLE_TYPE_INVALID);
        CHECK(invalid_handle.type == USER_HANDLE_TYPE_INVALID);

        uint32_t bad_descriptor  = 999;
        MgHandle invalid_handle2 = mg_handle_create(&arena, bad_descriptor);
        CHECK(invalid_handle2.type == USER_HANDLE_TYPE_INVALID);
    }

    TEST_CASE("Exceeding handle count limit")
    {
        MgArena arena;
        MgStatus status = mg_arena_create(&arena, &arena_descriptor);
        REQUIRE(status == MG_SUCCESS);

        MgHandle handles[HANDLE_LIMIT];
        for (int i = 0; i < HANDLE_LIMIT; ++i)
        {
            handles[i] = mg_handle_create(&arena, USER_HANDLE_TYPE_STRING);
            CHECK(handles[i].type != USER_HANDLE_TYPE_INVALID);
        }

        // Try creating one handle over the limit
        MgHandle handle = mg_handle_create(&arena, USER_HANDLE_TYPE_STRING);
        CHECK(handle.type == USER_HANDLE_TYPE_INVALID);
    }
}
