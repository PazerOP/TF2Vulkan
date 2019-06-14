#pragma once

#include "Threads.h"

#include <tier0/dbg.h>

#include <string_view>

#include <tier0/basetypes.h>

#define ENSURE(condition) { if (!(condition)) ::Util::EnsureConditionFailed(V_STRINGIFY(#condition), __FUNCSIG__, __FILE__, __LINE__); }

#define ASSERT_MAIN_THREAD() ENSURE(::Util::IsMainThread())

namespace Util
{
	template<bool enabled>
	struct LogFunctionCallScope
	{
		LogFunctionCallScope(const std::string_view& fnSig, const std::string_view& file, int line,
			const std::string_view& msg, bool checkThread);
		~LogFunctionCallScope();
	};

	void LogFunctionCall(const std::string_view& fnSig, const std::string_view& file, int line, const std::string_view& msg);
	[[noreturn]] void EnsureConditionFailed(const char* condition, const char* fnSig, const char* file, int line);
	void FunctionNotImplemented(const char* fnSig, const char* file, int line, const std::string_view& msg = {});

	template<>
	struct LogFunctionCallScope<false>
	{
		LogFunctionCallScope(const char*, const char*, int, const std::string_view&, bool checkThread)
		{
			if (checkThread)
				ASSERT_MAIN_THREAD();
		}
	};

	struct StaticTypeInfo final
	{
		size_t m_Size;
		size_t m_Offset;
		size_t m_Alignment;
	};
}

#define ALL_STATIC_TYPE_INFO_MEMBER(type, member) \
	::Util::StaticDataInfo{ sizeof(type ## :: ## member), offsetof(type, member), alignof(type ## :: ## member) }
#define ALL_STATIC_TYPE_INFO(type) \
	::Util::StaticTypeInfo{ sizeof(type), 0, alignof(type) }

//#define TF2VULKAN_ENABLE_FUNCTION_LOGGING 1
#define TF2VULKAN_LOCAL_ENABLE_FUNCTION_LOGGING true

#if defined(TF2VULKAN_ENABLE_FUNCTION_LOGGING)
#define LOG_FUNC_MSG_IMPL(msg, checkThread) ::Util::LogFunctionCallScope<TF2VULKAN_LOCAL_ENABLE_FUNCTION_LOGGING> EXPAND_CONCAT(_TF2Vulkan_LogFunctionCallScope_, __COUNTER__) (__FUNCSIG__, __FILE__, __LINE__, msg, checkThread)
#else
#define LOG_FUNC_MSG_IMPL(msg, checkThread) { if constexpr (checkThread) { ASSERT_MAIN_THREAD(); } }
#endif

#define LOG_FUNC_MSG_ANYTHREAD(msg) LOG_FUNC_MSG_IMPL(msg, false)
#define LOG_FUNC_MSG(msg) LOG_FUNC_MSG_IMPL(msg, true)

#define LOG_FUNC() LOG_FUNC_MSG(std::string_view{})
#define LOG_FUNC_ANYTHREAD() LOG_FUNC_MSG_ANYTHREAD(std::string_view{})

#define TF2VULKAN_PREFIX "[TF2Vulkan] " __FUNCSIG__ ": "

#define NOT_IMPLEMENTED_FUNC_NOBREAK(...) ::Util::FunctionNotImplemented(__FUNCSIG__, __FILE__, __LINE__, __VA_ARGS__)

#define PRINTF_SV(stringView) Util::SafeConvert<int>((stringView).size()), (stringView).data()
static constexpr const char* PRINTF_BOOL(bool val) { return val ? "true" : "false"; }

#ifdef MATT_HAYNIE
#define NOT_IMPLEMENTED_FUNC() \
	{ NOT_IMPLEMENTED_FUNC_NOBREAK(); __debugbreak(); throw 0; }
#else
#define NOT_IMPLEMENTED_FUNC() NOT_IMPLEMENTED_FUNC_NOBREAK()
#endif

#define ORDERING_OP_LT_IMPL_GENERIC(varNameLHS, varNameRHS) \
	if (varNameLHS < varNameRHS) \
		return true; \
	else if (varNameRHS < varNameLHS) \
		return false;

#define ORDERING_OP_LT_IMPL(varName) ORDERING_OP_LT_IMPL_GENERIC(varName, other. ## varName)
#define ORDERING_OP_LT_IMPL_PUB(varName) ORDERING_OP_LT_IMPL_GENERIC(lhs. ## varName, rhs. ## varName)

#define DISABLE_OBJ_MOVE(type) \
	type(type&&) = delete; \
	type& operator=(type&&) = delete;

#define DISABLE_OBJ_COPY(type) \
	type(const type&) = delete; \
	type& operator=(const type&) = delete;

#define DISABLE_OBJ_COPY_MOVE(type) \
	DISABLE_OBJ_MOVE(type); \
	DISABLE_OBJ_COPY(type);

#define CHECK_OFFSET(type, member, expectedOffset) \
	static_assert(!(offsetof(type, member) < expectedOffset), "Actual offset less than expected"); \
	static_assert(!(offsetof(type, member) > expectedOffset), "Actual offset greater than expected");

#define CHECK_SIZE(type, expectedSize) \
	static_assert(!(sizeof(type) < expectedSize), "Actual size less than expected"); \
	static_assert(!(sizeof(type) > expectedSize), "Actual size greater than expected");
