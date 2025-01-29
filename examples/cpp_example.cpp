#define MAGIC_ARENA_IMPL
#include <magic_arena.h>

struct Test
{
	int a = 8;
	int b = 2001;
	int* c = nullptr;
};

void PrintArenaStatus(Magic_Arena* arena)
{
	MC_INFO("Arena update:");
	MC_INFO("  Allocated: {0}", arena->Next);
	MC_INFO("  Capacity: {0}", arena->Capacity);
	MC_INFO("----------------------------------------");
}

int main(int argc, char* argv[])
{
	Magic::DebugProvider::Init();

	Magic_Arena arena = Magic_CreateArena(1024);

	PrintArenaStatus(&arena);

	int num = 7;

	Test test;
	test.c = &num;

	int num2 = 9;

	Test test2;
	test2.c = &num2;

	Test* test_ptr = Magic_Push(&arena, Test);

	if (test_ptr != nullptr)
	{
		*test_ptr = test;
		PrintArenaStatus(&arena);
	}
	else
	{
		std::cout << "Allocation failed\n";
	}

	Test* test_ptr2 = Magic_Push(&arena, Test);

	if (test_ptr2 != nullptr)
	{
		*test_ptr2 = test2;
		PrintArenaStatus(&arena);
	}
	else
	{
		std::cout << "Allocation failed\n";
	}

	Magic_Pop(&arena, Test);
	PrintArenaStatus(&arena);

	uint64_t* test_ptr3 = Magic_PushArray(&arena, uint64_t, 3);

	if (test_ptr3 != nullptr)
	{
		test_ptr3[0] = 13;
		test_ptr3[1] = 29;
		test_ptr3[2] = 34;

		PrintArenaStatus(&arena);

		MC_TRACE("test_ptr3[0]: {0}\n", test_ptr3[0]);
		MC_TRACE("test_ptr3[1]: {0}\n", test_ptr3[1]);
		MC_TRACE("test_ptr3[2]: {0}\n", test_ptr3[2]);
	}
	else
	{
		std::cout << "Allocation failed\n";
	}



} // end of main