#ifndef MG_DEBUG_HEADER
#define MG_DEBUG_HEADER

#if __cplusplus
extern "C" {
#endif

#define MG_SUCCESS MG_ERROR_NONE

#define STRING(s) #s
#define STRINGIFY(s) STRING(s)
#define ERROR_LINE "in " __FILE__ " line " STRINGIFY(__LINE__)

#define ERROR_PRINT(error)                   \
    {                                        \
        mg_error_print((error), ERROR_LINE); \
    }

#ifdef DEBUG
#include <assert.h>
#define MG_ASSERT(x) assert(x)
#else
#define MG_ASSERT(x) ((void)0)
#endif

#ifdef DEBUG

#define MG_CHECK_ERROR_HANDLE(error) \
    {                                \
        ERROR_PRINT(error);          \
        MG_ASSERT(false);            \
    }

#define MG_STATUS_ERROR_HANDLE(error) \
    {                                 \
        ERROR_PRINT(error);           \
        MG_ASSERT(false);             \
    }

#else

#define MG_CHECK_ERROR_HANDLE(error) ((void)0);

#define MG_STATUS_ERROR_HANDLE(error) \
    {                                 \
        ERROR_PRINT(error);           \
        return error;                 \
    }

#endif

#define _MG_CHECK(condition, error)      \
    do                                   \
    {                                    \
        if (!(condition))                \
        {                                \
            MG_CHECK_ERROR_HANDLE(error) \
        }                                \
    } while (0)

#define _MG_STATUS(condition, status)       \
    do                                      \
    {                                       \
        if (!(condition))                   \
        {                                   \
            MG_STATUS_ERROR_HANDLE(status); \
        }                                   \
    } while (0)

#define MG_STATUS(status) _MG_STATUS(status, status)

typedef enum MgStatus {
    MG_ERROR_NONE                    = 1001,
    MG_ERROR_ARENA_ALLOC_FAILED      = -1001,
    MG_ERROR_ARENA_INVALID           = -1002,
    MG_ERROR_ARENA_DESC_INVALID      = -1003,
    MG_ERROR_GROUP_CREATION_FAILED   = -1004,
    MG_ERROR_GROUP_QUERY_FAILED      = -1005,
    MG_ERROR_GROUP_EXHAUSTED         = -1006,
    MG_ERROR_GROUP_SLOT_ALLOC_FAILED = -1007,
    MG_ERROR_HANDLE_CREATION_FAILED  = -1008,
    MG_ERROR_HANDLE_TYPE_INVALID     = -1009,
    MG_ERROR_HANDLE_READ_FAILED      = -1010,
    MG_ERROR_HANDLE_WRITE_FAILED     = -1011,
    MG_ERROR_HANDLE_ERASE_FAILED     = -1012,
    MG_ERROR_HANDLE_INVALID          = -1013,
    MG_ERROR_DATA_INVALID            = -1014,
} MgStatus;

extern void mg_error_print(MgStatus error, const char* location);
extern const char* mg_error_name_string(MgStatus error);
extern const char* mg_error_message_string(MgStatus error);

#if __cplusplus
} // end extern "C"
#endif

#endif // MG_DEBUG_HEADER