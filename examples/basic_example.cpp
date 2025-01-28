#include <magic_arena.h>

#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <vector>

class Thread
{
public:
	Thread()
	{
		m_Worker = std::thread(&Thread::JobQueueLoop, this);
	}

	~Thread()
	{
		if (m_Worker.joinable())
		{
			Wait();

			m_QueueMutex.lock();
			m_Destroy = true;
			m_WorkCondition.notify_one();
			m_QueueMutex.unlock();

			m_Worker.join();
		}
	}

	Thread(const Thread&) = delete;
	Thread& operator=(const Thread&) = delete;

	Thread(Thread&& other) noexcept
		: m_Worker(std::move(other.m_Worker)), m_QueueMutex(), m_WorkCondition(),
		m_JobQueue(std::move(other.m_JobQueue)), m_Destroy(other.m_Destroy)
	{
		other.m_Destroy = true;
	}

	Thread& operator=(Thread&& other) noexcept
	{
		if (this != &other)
		{
			if (m_Worker.joinable())
			{
				Wait();

				m_QueueMutex.lock();
				m_Destroy = true;
				m_WorkCondition.notify_one();
				m_QueueMutex.unlock();

				m_Worker.join();
			}

			m_Worker = std::move(other.m_Worker);
			m_JobQueue = std::move(other.m_JobQueue);
			m_Destroy = other.m_Destroy;
			other.m_Destroy = true;
		}

		return *this;
	}

	void AddJob(std::function<void()> job)
	{
		std::lock_guard<std::mutex> lock(m_QueueMutex);
		m_JobQueue.push(std::move(job));
		m_WorkCondition.notify_one();
	}

	void Wait()
	{
		std::unique_lock<std::mutex> lock(m_QueueMutex);
		m_WorkCondition.wait(lock, [this]() { return m_JobQueue.empty(); });
	}

private:
	void JobQueueLoop()
	{
		std::function<void()> nextJob;

		while (true)
		{
			{
				std::unique_lock<std::mutex> lock(m_QueueMutex);
				m_WorkCondition.wait(lock, [this]() { return !m_JobQueue.empty() || m_Destroy; });
				if (m_Destroy) break;
				nextJob = m_JobQueue.front();
			}

			nextJob();

			{
				std::lock_guard<std::mutex> lock(m_QueueMutex);
				m_JobQueue.pop();
				m_WorkCondition.notify_one();
			}
		}
	}

private:
	std::thread m_Worker;
	std::mutex m_QueueMutex;
	std::condition_variable m_WorkCondition;
	std::queue<std::function<void()>> m_JobQueue;
	bool m_Destroy = false;
};

class ThreadPool
{
public:
	std::vector<Thread>& GetThreads()
	{
		return m_Threads;
	}

	void SetThreadCount(uint32_t count)
	{
		m_Threads.clear();
		MC_INFO("Thread Count: {}", count);
		m_Threads.resize(count);
	}

	void WaitForAll()
	{
		for (auto& thread : m_Threads)
		{
			thread.Wait();
		}
	}

private:
	std::vector<Thread> m_Threads;
};




int main(int argc, char* argv[])
{
	Magic::DebugProvider::Init();

	ThreadPool threadPool;

	const int fallbackThreadCount = 2;
	const int hardwareThreadCount = std::thread::hardware_concurrency();
	const int threadCount = hardwareThreadCount > 0 ? hardwareThreadCount - 1 : fallbackThreadCount;

	threadPool.SetThreadCount(threadCount);

	Magic::Arena arena;

	arena.Allocate(1024);

	/*for (auto& thread : threadPool.GetThreads())
	{
		thread.AddJob([&arena] {arena.DoWork();});
	}

	threadPool.WaitForAll();

	for (auto& thread : threadPool.GetThreads())
	{
		thread.AddJob([&arena] {arena.DoWork();});
	}

	threadPool.WaitForAll();*/



} // end of main