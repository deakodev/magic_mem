#pragma once

#include "MultiThreadArena.h"

namespace Magic {

	class Arena final // used to prevent inheritance
	{
	public:
		Arena() : m_MultiThreadArena() { std::cout << "Arena::Arena()" << std::endl; }

	private:
		MultiThreadArena m_MultiThreadArena;

	};

} // namespace Magic

