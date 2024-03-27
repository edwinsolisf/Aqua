#pragma once

#include "Core/Platform.h"

#ifdef AQUA_DEBUG
    #if defined(AQUA_PLATFORM_WINDOWS)
        #define AQUA_DEBUG_BREAK() __debugbreak()
    #elif defined(AQUA_PLATFORM_LINUX)
        #include <signal.h>
        #define AQUA_DEBUG_BREAK() raise(SIGTRAP)
    #else
        #error "Platform does not support debug break"
    #endif
#else
    #define AQUA_DEBUG_BREAK()
#endif

#if defined(AQUA_ENABLE_ASSERTS) && defined(AQUA_DEBUG)
    #define AQUA_ASSERT(check, msg) { if(!(check)) { std::cout << msg << std::endl; AQUA_DEBUG_BREAK(); } }
#else
    #define AQUA_ASSERT(check, msg)
#endif


#if defined(AQUA_ENABLE_PROFILING) && defined(AQUA_DEBUG)
    #include "Profile.h"

    #if defined(__GNUC__) || (defined(__MWERKS__) && (__MWERKS__ >= 0x3000)) || (defined(__ICC) && (__ICC >= 600)) || defined(__ghs__)
		#define AQUA_FUNC_SIG __PRETTY_FUNCTION__
	#elif defined(__DMC__) && (__DMC__ >= 0x810)
		#define AQUA_FUNC_SIG __PRETTY_FUNCTION__
	#elif (defined(__FUNCSIG__) || (_MSC_VER))
		#define AQUA_FUNC_SIG __FUNCSIG__
	#elif (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 600)) || (defined(__IBMCPP__) && (__IBMCPP__ >= 500))
		#define AQUA_FUNC_SIG __FUNCTION__
	#elif defined(__BORLANDC__) && (__BORLANDC__ >= 0x550)
		#define AQUA_FUNC_SIG __FUNC__
	#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901)
		#define AQUA_FUNC_SIG __func__
	#elif defined(__cplusplus) && (__cplusplus >= 201103)
		#define AQUA_FUNC_SIG __func__
	#else
		#define AQUA_FUNC_SIG "AQUA_FUNC_SIG unknown!"
	#endif

    #define AQUA_PROFILE_BEGIN(file) ::Aqua::Profiler::BeginProfile(file)
    #define AQUA_PROFILE_END() ::Aqua::Profiler::EndProfile()
    #define AQUA_PROFILE_FUNCTION() ::Aqua::Timer timer##__LINE__(AQUA_FUNC_SIG);
    
    #define AQUA_INFO(str) ::Aqua::Profiler::Get().info(str)
    #define AQUA_WARN(str) ::Aqua::Profiler::Get().warn(str)
    #ifdef AQUA_ERROR_BREAK
        #define AQUA_ERROR(str) { ::Aqua::Profiler::Get().error(str); AQUA_DEBUG_BREAK(); }
    #else
        #define AQUA_ERROR(str) ::Aqua::Profiler::Get().error(str)
    #endif
    #define AQUA_CRITICAL(str) { ::Aqua::Profiler::Get().critical(str); AQUA_DEBUG_BREAK(); }
#else
    #define AQUA_PROFILE_BEGIN(file)
    #define AQUA_PROFILE_END()
    #define AQUA_PROFILE_FUNCTION()

    #define AQUA_INFO(str)
    #define AQUA_WARN(str)
    #define AQUA_ERROR(str)
    #define AQUA_CRITICAL(str)
#endif

