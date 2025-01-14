#pragma once

#include <spdlog/logger.h>

namespace Magic {

	class DebugProvider
	{
	public:
		static void Init();

		static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }

	private:
		inline static std::shared_ptr<spdlog::logger> s_CoreLogger;
		inline static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};

}

// Core Log Macros
#define MC_CORE_CRITICAL(...) ::Magic::DebugProvider::GetCoreLogger()->critical(__VA_ARGS__)
#define MC_CORE_ERROR(...)    ::Magic::DebugProvider::GetCoreLogger()->error(__VA_ARGS__)
#define MC_CORE_WARN(...)     ::Magic::DebugProvider::GetCoreLogger()->warn(__VA_ARGS__)
#define MC_CORE_INFO(...)     ::Magic::DebugProvider::GetCoreLogger()->info(__VA_ARGS__)
#define MC_CORE_TRACE(...)    ::Magic::DebugProvider::GetCoreLogger()->trace(__VA_ARGS__)

// Client Log Macros
#define MC_CRITICAL(...)      ::Magic::DebugProvider::GetClientLogger()->critical(__VA_ARGS__)
#define MC_ERROR(...)         ::Magic::DebugProvider::GetClientLogger()->error(__VA_ARGS__)
#define MC_WARN(...)          ::Magic::DebugProvider::GetClientLogger()->warn(__VA_ARGS__)
#define MC_INFO(...)          ::Magic::DebugProvider::GetClientLogger()->info(__VA_ARGS__)
#define MC_TRACE(...)         ::Magic::DebugProvider::GetClientLogger()->trace(__VA_ARGS__)

// Assert Macros
#ifdef DEBUG
#if defined(MC_PLATFORM_WINDOWS)
#define MC_DEBUGBREAK() __debugbreak()
#elif defined(MC_PLATFORM_MACOS)
#include <signal.h>
#define MC_DEBUGBREAK() raise(SIGTRAP)
#else
#error "Platform doesn't support debugbreak yet!"
#endif
#define MC_ENABLE_ASSERTS
#else
#define MC_DEBUGBREAK()
#endif

#ifdef MC_ENABLE_ASSERTS
#define MC_ASSERT(x, ...) { if(!(x)) { MC_ERROR("Assertion Failed: {0}", __VA_ARGS__); MC_DEBUGBREAK(); } }
#define MC_CORE_ASSERT(x, ...) { if(!(x)) { MC_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); MC_DEBUGBREAK(); } }
#else
#define MC_ASSERT(...)
#define MC_CORE_ASSERT(...)
#endif