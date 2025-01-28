#include "magic_pch.h"
#include "Arena.h"

#include <mutex>

namespace Magic {

	/*void Arena::DoWork()
	{
		{
			std::scoped_lock lock(m_Mutex);

			auto threadId = std::this_thread::get_id();
			auto threadIdHash = std::hash<std::thread::id>{}(threadId);
			MC_INFO("... {}\n", threadIdHash);
		}

		std::this_thread::sleep_for(std::chrono::seconds(5));

	}*/

} // namespace Magic
