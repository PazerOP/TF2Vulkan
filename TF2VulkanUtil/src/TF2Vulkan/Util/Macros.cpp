#include "TF2Vulkan/Util/Macros.h"
#include "TF2Vulkan/Util/SafeConvert.h"

#include <cctype>

#pragma push_macro("RTL_NUMBER_OF_V2")
#pragma push_macro("_PREFAST_")
#undef RTL_NUMBER_OF_V2
#undef _PREFAST_
#include <Windows.h>
#pragma pop_macro("RTL_NUMBER_OF_V2")
#pragma pop_macro("_PREFAST_")

namespace
{
	class FnSigComponents
	{
	public:
		FnSigComponents(const std::string_view& fnSig)
		{
			size_t offset = 0;

			// Return type
			{
				size_t returnTypeBegin = offset;
				size_t returnTypeEnd = 0;
				while (offset < fnSig.size())
				{
					if (isspace(fnSig[offset]))
						break;

					returnTypeEnd++;
					offset++;
				}

				if (returnTypeEnd > returnTypeBegin)
					m_ReturnType = fnSig.substr(returnTypeBegin, returnTypeEnd - returnTypeBegin);
			}

			// TODO
		}

		std::string_view m_ReturnType;
		std::string_view m_Arguments;
	};
}

void Util::EnsureConditionFailed(const char* condition, const char* fnSig, const char* file, int line)
{
	Warning("[TF2Vulkan] ENSURE(%s) failed @ %s, %s:%i\n", condition, fnSig, file, line);
	assert(false);
	Error("[TF2Vulkan] ENSURE(%s) failed @ %s, %s:%i\n", condition, fnSig, file, line);
}

void Util::FunctionNotImplemented(const char* fnSig, const char* file, int line)
{
	Warning("[TF2Vulkan] %s in %s:%i not implemented\n", fnSig, file, line);
}

static thread_local int s_LogFunctionCallIndentation = 0;
static constexpr const char LOG_FN_INDENT_CHARS[] =
	"                                                            ";

template<bool enabled>
Util::LogFunctionCallScope<enabled>::LogFunctionCallScope(const std::string_view& fnSig,
	const std::string_view& file, int line, const std::string_view& msg)
{
	LogFunctionCall(fnSig, file, line, msg);
	s_LogFunctionCallIndentation++;
}
template<bool enabled>
Util::LogFunctionCallScope<enabled>::~LogFunctionCallScope()
{
	s_LogFunctionCallIndentation--;
}

template struct Util::LogFunctionCallScope<true>;

void Util::LogFunctionCall(const std::string_view& fnSig, const std::string_view& file, int line,
	const std::string_view& msg)
{
	ASSERT_MAIN_THREAD();
	const FnSigComponents comps(fnSig);

	char buf[512];
	sprintf_s(buf, "[TF2Vulkan] -> %.*s%.*s%s%.*s\n",
		s_LogFunctionCallIndentation, LOG_FN_INDENT_CHARS,
		PRINTF_SV(fnSig), msg.empty() ? "" : ": ", PRINTF_SV(msg));

	OutputDebugStringA(buf);
}
