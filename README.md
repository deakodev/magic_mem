# Magic Memory

**Magic Memory** is a lightweight, efficient memory management library for C/C++ projects. It provides dynamic memory allocation routines via a serial arena allocator.

## Features

- **Dynamic Memory Allocation:**  
  Allocate memory blocks using a custom memory object that tracks size, allocated capacity, and whether the block is dynamic.

- **Serial Arena Allocator:**  
  Write data sequentially into a pre-allocated memory arena. Quickly reset the arena to reuse the memory without individual deallocations.

- **Cross-Platform Compatibility:**  
  Depends solely on the C99 standard library, making it suitable for various platforms.

## Requirements

- **Standards:** C99, C++11, or MSVC 2010+
- **Dependencies:** Standard C headers
- **Build Tools:** [Premake5](https://premake.github.io/) for project generation.

## Running Examples

You can generate project files using Premake5. Run the setup_windows.bat file in the scripts dir to generate Visual Studio 2022 project files. Next, choose the example you'd like to run by defining the associated macro eg. #define MAGIC_ARENA_EXAMPLE in example_arena.h. NOTE: ensure startup project is set to examples in the workspace premake file and only one #define MAGIC_XXX_EXAMPLE is defined at one time (or exclude other examples from example project)

## Usage

### Magic_Memory

The core memory object is defined as:

```c
typedef struct {
    uint8_t* data;      // Pointer to allocated data
    uint32_t size;      // Size of the data in bytes
    uint32_t allocated; // Actual amount of memory allocated (may be greater than size)
    bool dynamic;       // Indicates if the memory can be resized
} Magic_Memory;
```

To allocate memory:

```c
#include "magic_memory.h"
#include "magic_error.h"

Magic_Memory memory;
int status = magic_allocate(&memory, 1024); // Request 1024 bytes
if (status != MAGIC_STATUS_OK) {
    // Handle error accordingly
}
```

### Arena Allocator

The arena allocator allows sequential writes to a memory block. Its structure is defined as follows:

```c
typedef struct {
    Magic_Memory memory;   // Underlying memory block
    uint32_t write_marker; // Current write position
    uint32_t end_marker;   // End of the arena
    bool dynamic;          // Can the arena be resized?
    bool allocated;        // Has the arena been allocated?
} Magic_Arena;
```

**Allocating and Writing to an Arena:**

```c
Magic_Arena arena;
status = magic_arena_allocate(&arena, 4096); // Allocate 4096 bytes for the arena
if (status != MAGIC_STATUS_OK) {
    // Handle error
}

// Writing data into the arena:
void* dest = /* pointer to your data */;
uint32_t data_size = /* size of your data */;
status = magic_arena_write(&arena, &dest, data_size);
if (status != MAGIC_STATUS_OK) {
    // Handle write error
}

// Clear the arena when you need to reuse it:
status = magic_arena_clear(&arena);
if (status != MAGIC_STATUS_OK) {
    // Handle clear error
}
```

## Contributing

Contributions are welcome! If you have improvements or bug fixes, please fork the repository and submit a pull request.
