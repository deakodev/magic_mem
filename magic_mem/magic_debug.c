#include "magic_debug.h"

#include <stdio.h>

#define ERROR_CASE_MESSAGE(error, message) \
    case error: return message;
#define ERROR_CASE_NAME(error, message) \
    case error: return #error;

#define ERROR_CASE_MAP(CASE)                                         \
    CASE(MG_ERROR_NONE, "no error found")                            \
    CASE(MG_ERROR_ARENA_ALLOC_FAILED, "failed to allocate arena")    \
    CASE(MG_ERROR_GROUP_CREATION_FAILED, "failed to create group")   \
    CASE(MG_ERROR_HANDLE_CREATION_FAILED, "failed to create handle") \
    CASE(MG_ERROR_HANDLE_READ_FAILED, "failed to read handle")       \
    CASE(MG_ERROR_HANDLE_WRITE_FAILED, "failed to write handle")     \
    CASE(MG_ERROR_ARENA_INVALID, "arena is invalid")                 \
    CASE(MG_ERROR_DATA_INVALID, "data is invalid")                   \
    CASE(MG_ERROR_ARENA_DESC_INVALID, "arena descriptor is invalid") \
    CASE(MG_ERROR_HANDLE_INVALID, "handle is invalid")

void mg_error_print(MgStatus error, const char* location)
{
    printf("\033[1;31m%s: %s %s\033[0m\n", mg_error_name_string(error), mg_error_message_string(error), location);
}

const char* mg_error_name_string(MgStatus error)
{
    switch (error)
    {
        ERROR_CASE_MAP(ERROR_CASE_NAME)
    }
    return "MG_ERROR_UNKNOWN";
}

const char* mg_error_message_string(MgStatus error)
{
    switch (error)
    {
        ERROR_CASE_MAP(ERROR_CASE_MESSAGE)
    }
    return "unable to identify error type";
}