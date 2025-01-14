#pragma once

namespace Magic {

	// forward declare
	class ArenaBlock;
	class MultiThreadArena;

	/// building block for multi-threaded arena
	/// not responsible for actual memory allocation
	/// instead owns memory blocks, manages cleanup list, 
	/// and delegates memory allocation
	class SingleThreadArena
	{
	public:

	private:
		/// used in multi-thread arena construction to set initial single thread arena (child)
		SingleThreadArena(MultiThreadArena& parent);

		friend class MultiThreadArena;

	private:
		std::atomic<ArenaBlock*> m_Blocks;
		MultiThreadArena& m_Parent;

	};

} // namespace Magic

