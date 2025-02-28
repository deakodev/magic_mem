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

TEST_SUITE("mg_arena_init")
{
    TEST_CASE("Passing valid descriptors")
    {
        MgArena* arena = mg_arena_init(&arena_descriptor);
        CHECK(arena != NULL);
    }

    /*  TEST_CASE("Passing invalid descriptors")
      {
          MgArena* arena = mg_arena_init(NULL);
          CHECK(arena == NULL);
      }*/
}

TEST_SUITE("mg_handle_create")
{
    TEST_CASE("Passing valid handles")
    {
        MgArena* arena = mg_arena_init(&arena_descriptor);
        REQUIRE(arena);

        MgHandle string_handle = mg_handle_create(arena, USER_HANDLE_TYPE_STRING);
        CHECK(string_handle.slot_handle != MG_HANDLE_INVALID);

        MgHandle array_handle = mg_handle_create(arena, USER_HANDLE_TYPE_ARRAY);
        CHECK(array_handle.slot_handle != MG_HANDLE_INVALID);
    }

    TEST_CASE("Passing invalid handles")
    {
        MgArena* arena = mg_arena_init(&arena_descriptor);
        REQUIRE(arena);

        MgHandle invalid_handle = mg_handle_create(arena, USER_HANDLE_TYPE_INVALID);
        CHECK(invalid_handle.slot_handle == MG_HANDLE_INVALID);

        uint32_t bad_descriptor  = 999;
        MgHandle invalid_handle2 = mg_handle_create(arena, bad_descriptor);
        CHECK(invalid_handle2.slot_handle == MG_HANDLE_INVALID);
    }

    TEST_CASE("Exceeding handle max")
    {
        MgArena* arena = mg_arena_init(&arena_descriptor);
        REQUIRE(arena);

        MgHandle handles[HANDLE_LIMIT];
        for (int i = 0; i < HANDLE_LIMIT; ++i)
        {
            handles[i] = mg_handle_create(arena, USER_HANDLE_TYPE_STRING);
            CHECK(handles[i].slot_handle != MG_HANDLE_INVALID);
        }

        // Try creating one handle over the limit
        MgHandle handle = mg_handle_create(arena, USER_HANDLE_TYPE_STRING);
        CHECK(handle.slot_handle == MG_HANDLE_INVALID);
    }
}

TEST_SUITE("mg_handle_write")
{
    MgStatus status;
    const char* data = "handle_write_test";

    TEST_CASE("Writing valid data to a handle")
    {
        MgArena* arena = mg_arena_init(&arena_descriptor);
        REQUIRE(arena);

        MgHandle string_handle = mg_handle_create(arena, USER_HANDLE_TYPE_STRING);
        CHECK(string_handle.slot_handle != MG_HANDLE_INVALID);

        status = mg_handle_write(arena, string_handle, &data, sizeof(UserString));
        CHECK(status == MG_SUCCESS);
    }

    TEST_CASE("Writing with invalid arena")
    {
        MgArena* arena = mg_arena_init(&arena_descriptor);
        REQUIRE(arena);

        MgHandle string_handle = mg_handle_create(arena, USER_HANDLE_TYPE_STRING);
        CHECK(string_handle.slot_handle != MG_HANDLE_INVALID);

        status = mg_handle_write(NULL, string_handle, &data, sizeof(UserString));
        CHECK(status == MG_ERROR_ARENA_INVALID);
    }

    TEST_CASE("Writing with invalid handle")
    {
        MgArena* arena = mg_arena_init(&arena_descriptor);
        REQUIRE(arena);

        MgHandle invalid_handle = { 0, 0 };

        status = mg_handle_write(arena, invalid_handle, &data, sizeof(UserString));
        CHECK(status == MG_ERROR_HANDLE_INVALID);
    }

    TEST_CASE("Writing null data")
    {
        MgArena* arena = mg_arena_init(&arena_descriptor);
        REQUIRE(arena);

        MgHandle string_handle = mg_handle_create(arena, USER_HANDLE_TYPE_STRING);
        CHECK(string_handle.slot_handle != MG_HANDLE_INVALID);

        status = mg_handle_write(arena, string_handle, NULL, sizeof(UserString));
        CHECK(status == MG_ERROR_DATA_INVALID);
    }

    TEST_CASE("Writing zero-size data")
    {
        MgArena* arena = mg_arena_init(&arena_descriptor);
        REQUIRE(arena);

        MgHandle string_handle = mg_handle_create(arena, USER_HANDLE_TYPE_STRING);
        CHECK(string_handle.slot_handle != MG_HANDLE_INVALID);

        status = mg_handle_write(arena, string_handle, &data, 0);
        CHECK(status == MG_ERROR_DATA_INVALID);
    }

    TEST_CASE("Writing with incorrect data size")
    {
        MgArena* arena = mg_arena_init(&arena_descriptor);
        REQUIRE(arena);

        MgHandle string_handle = mg_handle_create(arena, USER_HANDLE_TYPE_STRING);
        CHECK(string_handle.slot_handle != MG_HANDLE_INVALID);

        status = mg_handle_write(arena, string_handle, &data, sizeof(data) * 10); // Incorrect size
        CHECK(status == MG_ERROR_DATA_INVALID);
    }
}

TEST_SUITE("mg_handle_read")
{
    MgStatus status;
    const char* data = "handle_read_test";

    TEST_CASE("Reading valid data from a handle")
    {
        MgArena* arena = mg_arena_init(&arena_descriptor);
        REQUIRE(arena);

        MgHandle string_handle = mg_handle_create(arena, USER_HANDLE_TYPE_STRING);
        CHECK(string_handle.slot_handle != MG_HANDLE_INVALID);

        status = mg_handle_write(arena, string_handle, &data, sizeof(UserString));
        REQUIRE(status == MG_SUCCESS);

        const void* read_data = mg_handle_read(arena, string_handle);
        REQUIRE(read_data != NULL);
        CHECK(strcmp(*(const char**)read_data, data) == 0);
    }

    TEST_CASE("Reading with invalid arena")
    {
        MgArena* arena = mg_arena_init(&arena_descriptor);
        REQUIRE(arena);

        MgHandle string_handle = mg_handle_create(arena, USER_HANDLE_TYPE_STRING);
        CHECK(string_handle.slot_handle != MG_HANDLE_INVALID);

        const void* read_data = mg_handle_read(NULL, string_handle);
        CHECK(read_data == NULL);
    }

    TEST_CASE("Reading with invalid handle")
    {
        MgArena* arena = mg_arena_init(&arena_descriptor);
        REQUIRE(arena);

        MgHandle invalid_handle = { 0, 0 };

        const void* read_data = mg_handle_read(arena, invalid_handle);
        CHECK(read_data == NULL);
    }

    // TEST_CASE("Reading from an uninitialized handle")
    //{
    //

    //    MgArena* arena = mg_arena_init(&arena_descriptor);
    //    REQUIRE(status == MG_SUCCESS);

    //    MgHandle uninitialized_handle = mg_handle_create(arena, USER_HANDLE_TYPE_STRING);
    //    CHECK(uninitialized_handle.type != USER_HANDLE_TYPE_INVALID);

    //    // Read before writing any data
    //    const void* read_data = mg_handle_read(arena, uninitialized_handle);
    //    CHECK(read_data != NULL); // Should return valid memory, but may contain uninitialized data
    //}

    /*TEST_CASE("Reading from a freed arena")
    {


        MgArena* arena = mg_arena_init(&arena_descriptor);
        REQUIRE(status == MG_SUCCESS);

        MgHandle string_handle = mg_handle_create(arena, USER_HANDLE_TYPE_STRING);
        CHECK(string_handle.type != USER_HANDLE_TYPE_INVALID);

        mg_arena_destroy(arena);

        const void* read_data = mg_handle_read(arena, string_handle);
        CHECK(read_data == NULL);
    }*/
}
