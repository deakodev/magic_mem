#include <magic_memory.h>

#include <stdint.h>
#include <stdio.h>

//#define MAGIC_RING_EXAMPLE
#ifdef MAGIC_RING_EXAMPLE

#ifdef _WIN32
#include <windows.h>
#define sleep_ms(ms) Sleep(ms)
#else
#include <unistd.h>
#define sleep_ms(ms) usleep(ms * 1000)
#endif

#define BUFFER_SIZE 64
#define MAX_ITERATIONS 100

typedef struct {
	uint32_t a;
	uint32_t b;
	uint32_t c;
	uint32_t d;
	uint32_t e;
	uint32_t f;
	uint32_t g;
	uint32_t h;
	uint32_t i;
	uint32_t j;
	uint32_t k;
	uint32_t l;
	uint32_t m;
	uint32_t n;
	uint32_t o;
	uint32_t p;
	uint32_t q;
	uint32_t r;
	uint32_t s;
	uint32_t t;
	uint32_t u;
	uint32_t v;
	uint32_t w;
	uint32_t x;
	uint32_t y;
} Test;

int main(void)
{
	Magic_Ring ring;
	magic_ring_allocate(&ring, BUFFER_SIZE);

	Test write_data = { 0 };
	Test read_data = { 0 };
	uint32_t iteration = 0;

	while (MAX_ITERATIONS == 0 || iteration < MAX_ITERATIONS)
	{
		printf("Iteration %d\n", iteration);

		// **WRITE**
		printf("Writing: %d\n", write_data.c);
		if (magic_ring_write(&ring, &write_data, sizeof(write_data)) != MAGIC_STATUS_OK)
		{
			printf("Write failed! Buffer may be full.\n");
		}
		else
		{
			write_data.a += 1;
			write_data.b += 2;
			write_data.c += 3;
			write_data.d += 4;
		}

		// **READ**
		if (iteration % 2 == 0) {
			printf("Skipping read.\n");
			printf("-----------\n");
			printf("Read marker: %d\n", ring.read_marker);
			printf("Write marker: %d\n", ring.write_marker);
			printf("-----------\n");
			iteration++;
			sleep_ms(1000); // Delay for readability (50ms)
			continue;
		}

		if (magic_ring_read(&ring, &read_data, sizeof(read_data)) == MAGIC_STATUS_OK)
		{
			printf("Read: %d\n", read_data.c);
		}
		else
		{
			printf("Read failed! Buffer may be empty.\n");
		}

		printf("-----------\n");
		printf("Read marker: %d\n", ring.read_marker);
		printf("Write marker: %d\n", ring.write_marker);
		printf("-----------\n");

		iteration++;
		sleep_ms(1000);
	}

	return 0;
} // end main

#endif // MAGIC_RING_EXAMPLE