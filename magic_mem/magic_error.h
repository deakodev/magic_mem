#ifndef MAGIC_STATUS_HEADER
#define MAGIC_STATUS_HEADER

#include <stdio.h>

#if __cplusplus
extern "C" {
#endif

#define MAGIC_STATUS_OK MAGIC_ERROR_NONE
#define MAGIC_STATUS_ERROR -1

#define STRING(s) #s
#define STRINGIFY(s) STRING(s)

#define ERROR_LINE "in " __FILE__ " line " STRINGIFY(__LINE__)
#define ERROR_PRINT(error)  { mg_error_print((error), ERROR_LINE); }
#define ERROR_HANDLE(error) { ERROR_PRINT(error); return MAGIC_STATUS_ERROR; }

#define STATUS(status) do { if((status) != MAGIC_ERROR_NONE) { return MAGIC_STATUS_ERROR; } } while (0)
#define CHECK(condition, error) do { if(!(condition)) { ERROR_HANDLE(error); } } while (0)
#define CHECK_PTR(ptr) CHECK((ptr) != NULL, MAGIC_ERROR_NULL_PTR)

	typedef enum {
		MAGIC_ERROR_NONE = 0,
		MAGIC_ERROR_NULL_PTR = 1,
		MAGIC_ERROR_ALLOC_FAILED = 2,
		MAGIC_ERROR_READ_FAILED = 3,
		MAGIC_ERROR_WRITE_FAILED = 4,
		MAGIC_ERROR_ARENA_INVALID = 5,
		MAGIC_ERROR_RING_INVALID = 6
	} MgError;

	extern void mg_error_print(MgError error, const char* location);
	extern const char* mg_error_name_string(MgError error);
	extern const char* mg_error_message_string(MgError error);

#if __cplusplus
} // end extern "C"
#endif

#endif // MAGIC_STATUS_HEADER