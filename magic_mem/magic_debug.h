#ifndef MG_DEBUG_HEADER
#define MG_DEBUG_HEADER

#if __cplusplus
extern "C" {
#endif

#define MG_SUCCESS MG_ERROR_NONE

#ifdef DEBUG
#include <assert.h>
#define MG_ASSERT(x) assert(x)
#else
#define MG_ASSERT(x) ((void)0)
#endif

#define STRING(s) #s
#define STRINGIFY(s) STRING(s)
#define ERROR_LINE "in " __FILE__ " line " STRINGIFY(__LINE__)

#define ERROR_PRINT(error)                   \
    {                                        \
        mg_error_print((error), ERROR_LINE); \
    }

#define ERROR_HANDLE(error) \
    {                       \
        ERROR_PRINT(error); \
        MG_ASSERT(false);   \
    }

#define STATUS(status)                 \
    do                                 \
    {                                  \
        if ((status) != MG_ERROR_NONE) \
        {                              \
            return MG_STATUS_ERROR;    \
        }                              \
    } while (0)

#define MG_CHECK(condition, error) \
    do                             \
    {                              \
        if (!(condition))          \
        {                          \
            ERROR_HANDLE(error);   \
        }                          \
    } while (0)

typedef enum MgStatus {
    MG_ERROR_NONE = 0,
    MG_ERROR_ARENA_ALLOC_FAILED,
    MG_ERROR_GROUP_CREATION_FAILED,
    MG_ERROR_HANDLE_CREATION_FAILED,
    MG_ERROR_HANDLE_READ_FAILED,
    MG_ERROR_HANDLE_WRITE_FAILED,
    MG_ERROR_ARENA_INVALID,
    MG_ERROR_ARENA_DESC_INVALID,
    MG_ERROR_HANDLE_INVALID,
    MG_ERROR_DATA_INVALID,
} MgStatus;

extern void mg_error_print(MgStatus error, const char* location);
extern const char* mg_error_name_string(MgStatus error);
extern const char* mg_error_message_string(MgStatus error);

#if __cplusplus
} // end extern "C"
#endif

#endif // MG_DEBUG_HEADER