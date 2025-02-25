#include "magic_error.h"

#include <stdbool.h>
#include <stdio.h>

#define ERROR_CASE_MESSAGE(error, message) \
    case error: return message;
#define ERROR_CASE_NAME(error, message) \
    case error: return #error;

#define ERROR_CASE_MAP(CASE)                            \
    CASE(MAGIC_ERROR_NONE, "no error found")            \
    CASE(MAGIC_ERROR_NULL_PTR, "null pointer found")    \
    CASE(MAGIC_ERROR_ALLOC_FAILED, "allocation failed") \
    CASE(MAGIC_ERROR_READ_FAILED, "falied to read")     \
    CASE(MAGIC_ERROR_WRITE_FAILED, "falied to write")   \
    CASE(MAGIC_ERROR_ARENA_INVALID, "arena is invalid") \
    CASE(MAGIC_ERROR_RING_INVALID, "ring is invalid")

void mg_error_print(MgError error, const char* location)
{
    printf("\033[1;31m%s: %s %s\033[0m\n", mg_error_name_string(error), mg_error_message_string(error), location);
}

const char* mg_error_name_string(MgError error)
{
    switch (error)
    {
        ERROR_CASE_MAP(ERROR_CASE_NAME)
    }
    return "MAGIC_ERROR_UNKNOWN";
}

const char* mg_error_message_string(MgError error)
{
    switch (error)
    {
        ERROR_CASE_MAP(ERROR_CASE_MESSAGE)
    }
    return "unable to identify error type";
}