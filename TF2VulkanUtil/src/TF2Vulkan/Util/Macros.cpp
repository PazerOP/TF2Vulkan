#include "TF2Vulkan/Util/Macros.h"
#include "TF2Vulkan/Util/SafeConvert.h"

#include <cctype>

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

void Util::LogFunctionCall(const std::string_view& fnSig, const std::string_view& file, int line,
	const std::string_view& msg)
{
	const FnSigComponents comps(fnSig);
	Msg("[TF2Vulkan] %.*s%s%.*s\n", PRINTF_SV(fnSig), msg.empty() ? "" : ": ", PRINTF_SV(msg));
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
