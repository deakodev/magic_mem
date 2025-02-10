#include <magic_memory.h>

//#define MAGIC_CLAY_EXAMPLE
#ifdef MAGIC_CLAY_EXAMPLE

#define CLAY_IMPLEMENTATION
#include "clay.h"

void HandleClayErrors(Clay_ErrorData errorData)
{
	printf("%s", errorData.errorText.chars);
}

int main(void)
{


}

#endif // MAGIC_CLAY_EXAMPLE